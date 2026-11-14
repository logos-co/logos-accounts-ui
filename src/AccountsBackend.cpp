#include "AccountsBackend.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QTime>
#include <QtGlobal>

#include "account_core.h"
#include "logos_sdk.h"

#include <algorithm>
#include <chrono>
#include <future>
#include <memory>

namespace {

// Quiet below warnings: every failure it logs is already on screen.
Q_LOGGING_CATEGORY(lcBackend, "accounts_ui", QtWarningMsg)

/// Parse a reply and release it. Every library call returns JSON with an `ok`
/// field, so a null or unparseable reply is itself reported as a failed one
/// rather than left to the caller to notice.
QJsonObject takeReply(char *reply)
{
    const QByteArray text(reply);
    logos_account_core_string_free(reply);

    QJsonParseError parse{};
    const QJsonDocument document = QJsonDocument::fromJson(text, &parse);
    if (!document.isObject())
        return QJsonObject{{QStringLiteral("ok"), false},
                           {QStringLiteral("error"), QStringLiteral("unreadable reply: ") + parse.errorString()}};
    return document.object();
}

QByteArray utf8(const QString &text)
{
    return text.toUtf8();
}

/// An empty string from the view means "this account has no password", which
/// the library spells as an absent argument. It cannot mean an empty password:
/// the vault refuses that spelling outright.
const char *passwordArg(const QByteArray &password)
{
    return password.isEmpty() ? nullptr : password.constData();
}

QString compact(const QJsonObject &object)
{
    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

/// What a read says about the `count` staged edits it dropped. The log can
/// hold what one adds without holding the edit itself: a key made live under
/// another context, say.
QString droppedText(int count)
{
    return count == 1
        ? QStringLiteral("An edit waiting here adds nothing the store's log does not already "
                         "hold, so it was dropped.")
        : QStringLiteral("%1 edits waiting here add nothing the store's log does not already "
                         "hold, so they were dropped.")
              .arg(count);
}

/// A store's address without the slash it may end in.
QString locationPrefix(QString url)
{
    while (url.endsWith(QLatin1Char('/')))
        url.chop(1);
    return url;
}

/// Whether `balance`, a u128 in decimal, is at least `reserve`.
bool covers(const QString &balance, quint64 reserve)
{
    bool ok = false;
    const quint64 value = balance.toULongLong(&ok);
    if (ok)
        return value >= reserve;
    // Digits that overflow 64 bits are more than any reserve.
    return !balance.isEmpty()
        && std::all_of(balance.cbegin(), balance.cend(), [](QChar c) { return c.isDigit(); });
}

/// Trims a relay's peer id and address and checks them, taking the peer id
/// from an address that ends in /p2p/<peer id>. Both empty is no relay.
bool relayInput(QString *peer, QString *address, QString *problem)
{
    static const QRegularExpression suffix(QStringLiteral("/p2p/([1-9A-HJ-NP-Za-km-z]+)$"));
    static const QRegularExpression peerId(QStringLiteral("^[1-9A-HJ-NP-Za-km-z]{46,64}$"));
    static const QRegularExpression tcp(
        QStringLiteral("^/(ip4|ip6|dns|dns4|dns6)/[^/]+/tcp/[0-9]{1,5}$"));
    *peer = peer->trimmed();
    *address = address->trimmed();
    const QRegularExpressionMatch named = suffix.match(*address);
    if (named.hasMatch()) {
        if (!peer->isEmpty() && *peer != named.captured(1)) {
            *problem = QStringLiteral("The address ends in another peer id than the one given.");
            return false;
        }
        *peer = named.captured(1);
        address->chop(named.capturedLength());
    }
    if (peer->isEmpty() && address->isEmpty())
        return true;
    if (!peerId.match(*peer).hasMatch()) {
        *problem = QStringLiteral("A peer id is the relay's libp2p identity, 46 or more base58 "
                                  "characters such as 16Uiu2HAm\u2026");
        return false;
    }
    if (!tcp.match(*address).hasMatch()) {
        *problem = QStringLiteral("An address is a libp2p TCP address, such as "
                                  "/ip4/203.0.113.7/tcp/9010.");
        return false;
    }
    return true;
}

} // namespace

AccountsBackend::AccountsBackend()
{
    m_pool.setMaxThreadCount(1);

    const QString vault = vaultDirectory();
    const QString chatStore = qEnvironmentVariable("LOGOS_ACCOUNTS_STORE_URL");
    if (!chatStore.isEmpty()) {
        m_core = logos_account_core_new_with_chat_store(utf8(vault).constData(),
                                                        utf8(chatStore).constData());
        setStoreUrl(locationPrefix(chatStore) + QStringLiteral("/v1/account/"));
    } else {
        const QString sequencer = configuredSequencerUrl();
        const QString program = configuredRegistryProgram();
        m_core = logos_account_core_new(utf8(vault).constData(), utf8(sequencer).constData(),
                                        utf8(program).constData());
        setStoreUrl(locationPrefix(sequencer) + QStringLiteral(" \u00b7 "));
        const QByteArray token = qgetenv("LOGOS_ACCOUNTS_RELAY_TOKEN");
        const char *tokenArg = token.isEmpty() ? nullptr : token.constData();
        logos_account_core_set_relay_sender(m_core, &AccountsBackend::relayTrampoline, this,
                                            tokenArg);
        setRelaySupported(true);
        QSettings settings(settingsPath(), QSettings::IniFormat);
        QString peer = qEnvironmentVariable("LOGOS_ACCOUNTS_RELAY_PEER");
        QString address = qEnvironmentVariable("LOGOS_ACCOUNTS_RELAY_ADDR");
        if (settings.contains(QStringLiteral("relay/peer"))) {
            peer = settings.value(QStringLiteral("relay/peer")).toString();
            address = settings.value(QStringLiteral("relay/address")).toString();
        }
        QString problem;
        if (relayInput(&peer, &address, &problem))
            applyRelay(peer, address);
        else
            qCWarning(lcBackend).noquote() << "the account relay is off:" << problem;
        const QByteArray payerKey = qgetenv("LOGOS_ACCOUNTS_LEZ_PAYER_KEY");
        if (payerKey.isEmpty())
            logos_account_core_set_submitter(m_core, &AccountsBackend::submitTrampoline, this);
        else if (!logos_account_core_set_payer_key(m_core, payerKey.constData()))
            qCWarning(lcBackend) << "LOGOS_ACCOUNTS_LEZ_PAYER_KEY is not a LEE key";
    }

    if (!m_core) {
        setStatusText(QStringLiteral("Could not read the vault"));
        setVaultProblem(QStringLiteral("the account core could not start on ") + vault);
        return;
    }
    setStatusText(QStringLiteral("Ready"));
    reloadAccounts();
}

AccountsBackend::~AccountsBackend()
{
    // The pool holds a raw handle, so it has to drain before the handle goes.
    m_closing = true;
    m_pool.waitForDone();
    logos_account_core_free(m_core);
}

QString AccountsBackend::vaultDirectory()
{
    const QByteArray override = qgetenv("LOGOS_ACCOUNTS_VAULT_DIR");
    if (!override.isEmpty())
        return QString::fromLocal8Bit(override);

    // A view plugin is assigned no directory, so the vault goes where a core
    // module's would: module_data under the profile root. Basecamp exports that
    // root as LOGOS_USER_DIR only for --user-dir (logos-basecamp#315); otherwise
    // this is ui-host's own AppDataLocation, which every profile shares.
    QString root = qEnvironmentVariable("LOGOS_USER_DIR");
    if (root.isEmpty())
        root = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(root).filePath(QStringLiteral("module_data/accounts_ui/vault"));
}

QString AccountsBackend::configuredSequencerUrl()
{
    const QByteArray override = qgetenv("LOGOS_ACCOUNTS_SEQUENCER_URL");
    if (!override.isEmpty())
        return QString::fromLocal8Bit(override);
    return QString::fromUtf8(logos_account_core_default_sequencer_url());
}

QString AccountsBackend::configuredRegistryProgram()
{
    const QByteArray override = qgetenv("LOGOS_ACCOUNTS_REGISTRY_PROGRAM");
    if (!override.isEmpty())
        return QString::fromLocal8Bit(override);
    return QString::fromUtf8(logos_account_core_default_registry_program());
}

const char *AccountsBackend::submitTrampoline(void *context, const char *request)
{
    auto *self = static_cast<AccountsBackend *>(context);
    return self->answerOnOwnThread(&AccountsBackend::submitToWallet, request, self->m_submitReply);
}

const char *AccountsBackend::relayTrampoline(void *context, const char *request)
{
    auto *self = static_cast<AccountsBackend *>(context);
    return self->answerOnOwnThread(&AccountsBackend::sendToRelay, request, self->m_relayReply);
}

const char *AccountsBackend::answerOnOwnThread(QByteArray (AccountsBackend::*work)(const QByteArray &),
                                               const char *request, QByteArray &replyBuffer)
{
    const QByteArray body(request);
    auto answer = std::make_shared<std::promise<QByteArray>>();
    std::future<QByteArray> reply = answer->get_future();
    QMetaObject::invokeMethod(
        this, [this, work, body, answer] { answer->set_value((this->*work)(body)); },
        Qt::QueuedConnection);
    while (reply.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready) {
        if (m_closing) {
            replyBuffer = compact({{QStringLiteral("ok"), false},
                                   {QStringLiteral("error"), QStringLiteral("the app is closing")}})
                              .toUtf8();
            return replyBuffer.constData();
        }
    }
    replyBuffer = reply.get();
    return replyBuffer.constData();
}

QByteArray AccountsBackend::sendToRelay(const QByteArray &request)
{
    const auto fail = [](const QString &error) {
        return compact({{QStringLiteral("ok"), false}, {QStringLiteral("error"), error}}).toUtf8();
    };
    if (!m_contextReady)
        return fail(QStringLiteral("libp2p is not reachable yet"));
    QString problem;
    if (!startLibp2p(&problem))
        return fail(problem);

    const QJsonObject args{
        {QStringLiteral("peerId"), m_relayPeer},
        {QStringLiteral("multiaddrs"), QJsonArray{m_relayAddr}},
        {QStringLiteral("proto"), QStringLiteral("/logos/accounts/relay/1.0.0")},
        {QStringLiteral("requestB64"), QString::fromLatin1(request.toBase64())},
        {QStringLiteral("timeoutMs"), 15000},
    };
    const LogosResult sent = modules().libp2p_module.protocolRequest(compact(args));
    if (!sent.success) {
        const QString error = sent.error.toString();
        return fail(error.isEmpty() ? QStringLiteral("the request did not go through") : error);
    }
    const QByteArray reply = QByteArray::fromBase64(
        sent.value.toMap().value(QStringLiteral("responseB64")).toString().toLatin1());
    return compact({{QStringLiteral("ok"), true}, {QStringLiteral("reply"), QString::fromUtf8(reply)}})
        .toUtf8();
}

bool AccountsBackend::startLibp2p(QString *problem)
{
    if (m_libp2pStarted) {
        *problem = m_libp2pProblem;
        return m_libp2pProblem.isEmpty();
    }
    const QJsonObject config{
        {QStringLiteral("addrs"), QJsonArray{QStringLiteral("/ip4/127.0.0.1/tcp/0")}},
        {QStringLiteral("transport"), QStringLiteral("tcp")},
        {QStringLiteral("maxConnsPerPeer"), 1},
        {QStringLiteral("mountGossipsub"), false},
        {QStringLiteral("mountKad"), false},
        {QStringLiteral("mountServiceDiscovery"), false},
    };
    const LogosResult created = modules().libp2p_module.createNode(compact(config));
    // Another module made the node first, and runs it for us too.
    if (!created.success && created.error.toString().contains(QStringLiteral("already"))) {
        m_libp2pStarted = true;
        return true;
    }
    if (!created.success) {
        *problem = QStringLiteral("libp2p did not start: ") + created.error.toString();
        return false;
    }
    // A second start while one still runs crashes libp2p_module.
    m_libp2pStarted = true;
    const LogosResult started = modules().libp2p_module.start();
    if (!started.success) {
        m_libp2pProblem = QStringLiteral("libp2p did not start: ") + started.error.toString();
        *problem = m_libp2pProblem;
        return false;
    }
    return true;
}

QByteArray AccountsBackend::submitToWallet(const QByteArray &request)
{
    const auto fail = [](const QString &error) {
        return compact({{QStringLiteral("ok"), false}, {QStringLiteral("error"), error}}).toUtf8();
    };
    if (!m_contextReady)
        return fail(QStringLiteral("the LEZ wallet is not reachable yet"));

    const QJsonObject write = QJsonDocument::fromJson(request).object();
    QString problem;
    const QString payer = payerFor(&problem);
    if (payer.isEmpty())
        return fail(problem);

    const QString reply = modules().lez_core.send_generic_public_transaction(
        QStringList{write.value(QStringLiteral("account")).toString()},
        QList<bool>{false},
        QByteArray::fromHex(write.value(QStringLiteral("instruction")).toString().toLatin1()),
        write.value(QStringLiteral("program")).toString(),
        payer,
        QStringList{QString()});
    const QJsonObject sent = QJsonDocument::fromJson(reply.toUtf8()).object();
    if (!sent.value(QStringLiteral("success")).toBool()) {
        const QString error = sent.value(QStringLiteral("error")).toString();
        return fail(error.isEmpty() ? QStringLiteral("the wallet refused the transaction") : error);
    }
    return compact({{QStringLiteral("ok"), true},
                    {QStringLiteral("txHash"), sent.value(QStringLiteral("tx_hash")).toString()}})
        .toUtf8();
}

QString AccountsBackend::payerFor(QString *problem)
{
    const QByteArray override = qgetenv("LOGOS_ACCOUNTS_LEZ_PAYER");
    if (!override.isEmpty())
        return QString::fromLocal8Bit(override);

    // The wallet reserves its own gas_limit, at most the 10M cap, x base fee (at least 8).
    const quint64 reserve = 10'000'000ULL * 8;
    const QVariantList accounts = modules().lez_core.list_accounts();
    if (accounts.isEmpty()) {
        *problem = QStringLiteral("open the LEZ Wallet app: it holds no account to pay with");
        return {};
    }
    for (const QVariant &entry : accounts) {
        const QVariantMap account = entry.toMap();
        if (!account.value(QStringLiteral("is_public"), true).toBool())
            continue;
        const QString id = account.value(QStringLiteral("account_id")).toString();
        if (covers(modules().lez_core.get_balance(id, true), reserve))
            return id;
    }
    *problem = QStringLiteral("no public account in the LEZ wallet holds the %1 units a "
                              "write reserves")
                   .arg(reserve);
    return {};
}

void AccountsBackend::onContextReady()
{
    // The first point at which calling out is safe: the LEZ wallet is reached
    // only from here on, and the first read of the registry happens here
    // rather than in the constructor.
    m_contextReady = true;
    if (m_core && !selectedAddress().isEmpty())
        refresh();
}

void AccountsBackend::call(const QString &what,
                           std::function<char *(LogosAccountCore *)> work,
                           std::function<void(const QJsonObject &)> landed,
                           std::function<void(const QString &)> refused)
{
    if (!m_core) {
        report(what, QStringLiteral("the account core is not open"));
        return;
    }

    ++m_inFlight;
    setBusy(true);
    LogosAccountCore *core = m_core;
    m_pool.start([this, what, core, work = std::move(work), landed = std::move(landed),
                  refused = std::move(refused)] {
        char *raw = work(core);
        // Back to the thread that owns the properties. A property set from the
        // pool would be a data race with every replica read.
        if (!raw) {
            QMetaObject::invokeMethod(
                this,
                [this] {
                    if (--m_inFlight == 0)
                        setBusy(false);
                },
                Qt::QueuedConnection);
            return;
        }
        const QJsonObject reply = takeReply(raw);
        QMetaObject::invokeMethod(
            this,
            [this, what, reply, landed, refused] {
                if (!reply.value(QStringLiteral("ok")).toBool()) {
                    const QString message = reply.value(QStringLiteral("error")).toString();
                    report(what, message);
                    if (refused)
                        refused(message);
                } else {
                    if (landed)
                        landed(reply);
                    emit succeeded(what);
                }
                // After the handlers, which chain the reads that follow, so
                // busy holds across the chain rather than dropping between its
                // links.
                if (--m_inFlight == 0)
                    setBusy(false);
            },
            Qt::QueuedConnection);
    });
}

void AccountsBackend::reloadAccounts()
{
    call(QStringLiteral("accounts"),
         [](LogosAccountCore *core) { return logos_account_core_accounts(core); },
         [this](const QJsonObject &reply) {
             setVaultProblem(QString());
             const QJsonArray accounts = reply.value(QStringLiteral("accounts")).toArray();
             setAccountsJson(QString::fromUtf8(QJsonDocument(accounts).toJson(QJsonDocument::Compact)));

             if (accounts.isEmpty()) {
                 setSelection(QString());
                 setStateJson(QStringLiteral("{}"));
                 setStatusText(QStringLiteral("No accounts yet"));
                 return;
             }
             // Hold the selection across a reload; fall to the first account
             // when what was selected is gone, or nothing was.
             const QString held = selectedAddress();
             bool stillHeld = false;
             for (const QJsonValue &account : accounts)
                 stillHeld = stillHeld
                     || account.toObject().value(QStringLiteral("address")).toString() == held;
             if (!stillHeld)
                 setSelection(accounts.first().toObject().value(QStringLiteral("address")).toString());
             reloadState();
             // An account newly on screen has not been read yet this session.
             if (!stillHeld && m_contextReady)
                 refresh();
         },
         // An empty listing would say the vault holds nothing.
         [this](const QString &message) {
             setStatusText(QStringLiteral("Could not read the vault"));
             setVaultProblem(message);
         });
}

void AccountsBackend::reloadState()
{
    const QByteArray address = utf8(selectedAddress());
    if (address.isEmpty()) {
        setStateJson(QStringLiteral("{}"));
        return;
    }
    call(QStringLiteral("state"),
         [address](LogosAccountCore *core) {
             return logos_account_core_state(core, address.constData());
         },
         [this, address](const QJsonObject &reply) {
             // A reply for an account no longer on screen would put its log
             // under the other one's name.
             if (address != utf8(selectedAddress()))
                 return;
             setVaultProblem(QString());
             setStateJson(compact(reply.value(QStringLiteral("state")).toObject()));
         },
         // Without it the account would show as loading for good.
         [this, address](const QString &message) {
             if (address != utf8(selectedAddress()))
                 return;
             setStatusText(QStringLiteral("Could not read the vault"));
             setVaultProblem(message);
         });
}

void AccountsBackend::refresh()
{
    const QByteArray address = utf8(selectedAddress());
    if (address.isEmpty()) {
        reloadAccounts();
        return;
    }
    setStatusText(QStringLiteral("Reading the store"));
    const quint64 selection = m_selection.load();
    call(QStringLiteral("refresh"),
         [this, address, selection](LogosAccountCore *core) -> char * {
             // A read queued for an account since left would only hold up the
             // one now on screen, which has its own read queued.
             if (m_selection.load() != selection)
                 return nullptr;
             return logos_account_core_refresh(core, address.constData());
         },
         [this, address, selection](const QJsonObject &reply) {
             // Edits the store already holds leave the pending list, and would
             // go without a word.
             const int dropped = reply.value(QStringLiteral("dropped")).toInt();
             if (dropped > 0 && address == utf8(selectedAddress()))
                 setNotice(QStringLiteral("info"), QStringLiteral("Already in the store"),
                           droppedText(dropped));
             else if (dropped > 0)
                 m_dropped[QString::fromUtf8(address)] += dropped;
             // A read the selection has since moved past speaks for no account
             // on screen, and another read is on its way for the one that is.
             if (m_selection.load() == selection) {
                 setStatusText(QStringLiteral("Ready"));
                 setStoreProblem(QString());
                 setFirstNewIndex(-1);
             }
             reloadAccounts();
         },
         // The vault answers whether or not the store does, and what it holds
         // is what the screen is drawn from.
         [this, selection](const QString &message) {
             if (m_selection.load() == selection) {
                 // Timed, so a retry that fails the same way still shows it ran.
                 setStatusText(QStringLiteral("Could not read the store at %1")
                                   .arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss"))));
                 setStoreProblem(message);
             }
             reloadAccounts();
         });
}

void AccountsBackend::selectAccount(QString address)
{
    if (address == selectedAddress())
        return;
    setSelection(address);
    reloadState();
    refresh();
}

void AccountsBackend::setSelection(const QString &address)
{
    ++m_selection;
    setSelectedAddress(address);
    setFirstNewIndex(-1);
    dismissNotice();
    if (const int dropped = m_dropped.take(address); dropped > 0)
        setNotice(QStringLiteral("info"), QStringLiteral("Already in the store"),
                  droppedText(dropped));
}

void AccountsBackend::createAccount(QString password)
{
    const QByteArray secret = utf8(password);
    call(QStringLiteral("createAccount"),
         [secret](LogosAccountCore *core) {
             return logos_account_core_create_account(core, passwordArg(secret));
         },
         [this](const QJsonObject &reply) {
             setSelection(reply.value(QStringLiteral("address")).toString());
             reloadAccounts();
             reloadState();
             refresh();
         });
}

void AccountsBackend::importAccount(QString secretHex, QString password)
{
    const QByteArray secret = utf8(secretHex);
    const QByteArray pass = utf8(password);
    call(QStringLiteral("importAccount"),
         [secret, pass](LogosAccountCore *core) {
             return logos_account_core_import_account(core, secret.constData(), passwordArg(pass));
         },
         [this](const QJsonObject &reply) {
             setSelection(reply.value(QStringLiteral("address")).toString());
             if (reply.value(QStringLiteral("alreadyHeld")).toBool())
                 setNotice(QStringLiteral("info"), QStringLiteral("Already in this vault"),
                           QStringLiteral("This key is an account the vault already holds, so it "
                                          "is kept as it was, with its own password or none."));
             reloadAccounts();
             reloadState();
             refresh();
         });
}

void AccountsBackend::exportAccount(QString address, QString password)
{
    const QByteArray target = utf8(address);
    const QByteArray pass = utf8(password);
    call(QStringLiteral("exportAccount"),
         [target, pass](LogosAccountCore *core) {
             return logos_account_core_export_account(core, target.constData(), passwordArg(pass));
         },
         [this, address](const QJsonObject &reply) {
             // Straight to the view and nowhere else: not a property, so it is
             // not re-sent to every replica on every later change.
             emit accountExported(address, reply.value(QStringLiteral("key")).toString());
         });
}

void AccountsBackend::unlock(QString address, QString password)
{
    const QByteArray target = utf8(address);
    const QByteArray pass = utf8(password);
    call(QStringLiteral("unlock"),
         [target, pass](LogosAccountCore *core) {
             return logos_account_core_unlock(core, target.constData(), pass.constData());
         },
         [this](const QJsonObject &) { reloadAccounts(); });
}

void AccountsBackend::forgetAccount(QString address)
{
    const QByteArray target = utf8(address);
    call(QStringLiteral("forgetAccount"),
         [target](LogosAccountCore *core) {
             return logos_account_core_forget_account(core, target.constData());
         },
         [this, address](const QJsonObject &) {
             m_dropped.remove(address);
             if (address == selectedAddress())
                 setSelection(QString());
             reloadAccounts();
         });
}

void AccountsBackend::observeAccount(QString address)
{
    // The core takes an address in one spelling only, so what it was asked
    // about is what goes on screen: a selection in any other spelling would
    // never match the state that comes back.
    const QString addr = address.trimmed();
    const QByteArray target = utf8(addr);
    call(QStringLiteral("observeAccount"),
         [target](LogosAccountCore *core) {
             return logos_account_core_observe_account(core, target.constData());
         },
         [this, addr](const QJsonObject &reply) {
             setSelection(addr);
             if (reply.value(QStringLiteral("alreadyObserved")).toBool())
                 setNotice(QStringLiteral("info"), QStringLiteral("Already observed"),
                           QStringLiteral("This account is already one this app reads, so it is "
                                          "shown as it was."));
             reloadAccounts();
             reloadState();
             refresh();
         });
}

void AccountsBackend::stopObserving(QString address)
{
    const QByteArray target = utf8(address);
    call(QStringLiteral("stopObserving"),
         [target](LogosAccountCore *core) {
             return logos_account_core_stop_observing(core, target.constData());
         },
         [this, address](const QJsonObject &) {
             m_dropped.remove(address);
             if (address == selectedAddress())
                 setSelection(QString());
             reloadAccounts();
         });
}

void AccountsBackend::stageAddInstallation(QString keyHex)
{
    const QByteArray address = utf8(selectedAddress());
    const QByteArray key = utf8(keyHex.trimmed());
    call(QStringLiteral("stageAddInstallation"),
         [address, key](LogosAccountCore *core) {
             return logos_account_core_stage_add_installation(core, address.constData(),
                                                              key.constData());
         },
         [this](const QJsonObject &) { edited(); });
}

void AccountsBackend::stageSetDisplayName(QString name)
{
    const QByteArray address = utf8(selectedAddress());
    const QByteArray value = utf8(name);
    call(QStringLiteral("stageSetDisplayName"),
         [address, value](LogosAccountCore *core) {
             return logos_account_core_stage_set_display_name(core, address.constData(),
                                                              value.constData());
         },
         [this](const QJsonObject &) { edited(); });
}

void AccountsBackend::stageRevoke(int index)
{
    if (index < 0) {
        report(QStringLiteral("stageRevoke"), QStringLiteral("there is no entry before the first"));
        return;
    }
    const QByteArray address = utf8(selectedAddress());
    const quint32 target = static_cast<quint32>(index);
    call(QStringLiteral("stageRevoke"),
         [address, target](LogosAccountCore *core) {
             return logos_account_core_stage_revoke(core, address.constData(), target);
         },
         [this](const QJsonObject &) { edited(); });
}

void AccountsBackend::discardPending()
{
    const QByteArray address = utf8(selectedAddress());
    call(QStringLiteral("discardPending"),
         [address](LogosAccountCore *core) {
             return logos_account_core_discard_pending(core, address.constData());
         },
         [this](const QJsonObject &) { edited(); });
}

void AccountsBackend::useRelay(bool on)
{
    setSponsored(on);
}

void AccountsBackend::setRelay(QString peer, QString address)
{
    const QString what = QStringLiteral("setRelay");
    if (!relaySupported()) {
        report(what, QStringLiteral("This store publishes without a relay."));
        return;
    }
    QString problem;
    if (!relayInput(&peer, &address, &problem)) {
        report(what, problem);
        return;
    }
    QSettings settings(settingsPath(), QSettings::IniFormat);
    settings.setValue(QStringLiteral("relay/peer"), peer);
    settings.setValue(QStringLiteral("relay/address"), address);
    settings.sync();
    if (settings.status() != QSettings::NoError) {
        report(what, QStringLiteral("The relay could not be saved to ") + settingsPath());
        return;
    }
    applyRelay(peer, address);
    emit succeeded(what);
}

void AccountsBackend::applyRelay(const QString &peer, const QString &address)
{
    m_relayPeer = peer;
    m_relayAddr = address;
    setRelayPeer(peer);
    setRelayAddress(address);
    setRelayAvailable(!peer.isEmpty() && !address.isEmpty());
}

QString AccountsBackend::settingsPath()
{
    return QDir::cleanPath(vaultDirectory()) + QStringLiteral("-settings.ini");
}

void AccountsBackend::publish()
{
    const QString address = selectedAddress();
    const QByteArray target = utf8(address);
    const bool sponsored = relayAvailable() && this->sponsored();

    setStatusText(sponsored ? QStringLiteral("Publishing through the relay")
                            : QStringLiteral("Publishing"));
    call(QStringLiteral("publish"),
         [target, sponsored](LogosAccountCore *core) {
             logos_account_core_set_sponsored(core, sponsored);
             return logos_account_core_publish(core, target.constData());
         },
         [this, address](const QJsonObject &reply) {
             // Null when the store already held every staged edit.
             const int firstNew = reply.value(QStringLiteral("firstNewIndex")).toInt(-1);
             const int dropped = reply.value(QStringLiteral("dropped")).toInt();
             setStatusText(QStringLiteral("Ready"));
             setStoreProblem(QString());
             // The person may have moved to another account while the store
             // answered, and these describe the one that published.
             if (address != selectedAddress()) {
                 if (dropped > 0)
                     m_dropped[address] += dropped;
             } else if (firstNew < 0) {
                 setNotice(QStringLiteral("info"), QStringLiteral("Already in the store"),
                           droppedText(dropped));
             } else {
                 setFirstNewIndex(firstNew);
                 QString body = QStringLiteral("The store holds this log. Anyone reading this "
                                               "address now sees what was just written.");
                 if (dropped > 0)
                     body += QLatin1Char(' ') + droppedText(dropped);
                 setNotice(QStringLiteral("success"), QStringLiteral("Published"), body);
             }
             if (firstNew >= 0)
                 emit published(address, firstNew);
             reloadAccounts();
         },
         // The sheet the publish came from is still up, and says why.
         [this, address](const QString &) {
             if (address != selectedAddress()) {
                 reloadAccounts();
                 return;
             }
             // The refusal may be the store's, and can follow a read that moved
             // the log on: reading it again says which, and draws what is
             // staged and held.
             refresh();
         });
}

void AccountsBackend::edited()
{
    // A notice described the log before this edit.
    dismissNotice();
    reloadAccounts();
}

void AccountsBackend::dismissNotice()
{
    setNotice(QString(), QString(), QString());
}

void AccountsBackend::setNotice(const QString &kind, const QString &title, const QString &body)
{
    setNoticeKind(kind);
    setNoticeTitle(title);
    setNoticeBody(body);
}

void AccountsBackend::report(const QString &what, const QString &message)
{
    // A sheet or a screen shows its own call's failure. The calls made from
    // a button on the log or the account bar have nowhere else to say it.
    if (what == QLatin1String("stageRevoke")) {
        setNotice(QStringLiteral("error"), QStringLiteral("Could not stage the removal"), message);
    } else if (what == QLatin1String("discardPending")) {
        setNotice(QStringLiteral("error"), QStringLiteral("Could not discard the pending entries"),
                  message);
    } else if (what == QLatin1String("stopObserving")) {
        setNotice(QStringLiteral("error"), QStringLiteral("Could not stop observing"), message);
    }
    qCDebug(lcBackend).noquote() << what << "failed:" << message;
    emit failed(what, message);
}
