#pragma once

#include <QJsonObject>
#include <QHash>
#include <QString>
#include <QThreadPool>

#include <atomic>
#include <functional>

#include "rep_AccountsBackend_source.h"
#include "logos_ui_plugin_context.h"

struct LogosAccountCore;

// The Accounts backend.
//
// It decides nothing. Every account question -- what the vault holds, what the
// log says, whether an edit is legal, what a publish writes -- is answered by
// the linked-in Rust library, and this class marshals: JSON in from the
// library, properties out to the view, the view's strings back in.
//
// It is a `ui_qml` backend, so the runtime hands it no data directory the way
// it hands a core module `instance_persistence_path`. The vault path is
// therefore this app's own choice, and `vaultDirectory()` is where that choice
// lives.
//
// Threading: the two calls that reach the store take as long as the store
// takes, and a slot that blocks here blocks the view behind it. So every call
// runs on a one-thread pool and comes back through the event loop, and `busy`
// is what the view guards its buttons with.
class AccountsBackend : public AccountsBackendSimpleSource,
                        public LogosUiPluginContext
{
public:
    AccountsBackend();
    ~AccountsBackend() override;

    void refresh() override;
    void selectAccount(QString address) override;
    void createAccount(QString password) override;
    void importAccount(QString secretHex, QString password) override;
    void exportAccount(QString address, QString password) override;
    void unlock(QString address, QString password) override;
    void forgetAccount(QString address) override;
    void observeAccount(QString address) override;
    void stopObserving(QString address) override;
    void stageAddInstallation(QString keyHex) override;
    void stageSetDisplayName(QString name) override;
    void useRelay(bool on) override;
    void setRelay(QString peer, QString address) override;
    void stageRevoke(int index) override;
    void discardPending() override;
    void publish() override;
    void dismissNotice() override;

    /// Where this app keeps account keys. `LOGOS_ACCOUNTS_VAULT_DIR` if it is
    /// set, otherwise `module_data/accounts_ui/vault` under `LOGOS_USER_DIR`,
    /// or under ui-host's application data when the host exports no root.
    static QString vaultDirectory();
    /// `LOGOS_ACCOUNTS_SEQUENCER_URL` if it is set, otherwise the library's
    /// default LEZ sequencer. Not used when `LOGOS_ACCOUNTS_STORE_URL` names a
    /// chat-store to publish to instead. The literal "memory" runs against an in-process
    /// store.
    static QString configuredSequencerUrl();
    /// `LOGOS_ACCOUNTS_REGISTRY_PROGRAM` if it is set, otherwise the library's
    /// default: the base58 account the registry program is deployed at.
    static QString configuredRegistryProgram();

protected:
    void onContextReady() override;

private:
    /// One library call, off the view's thread. `what` names the call for the
    /// `failed` and `succeeded` signals; `work` runs on the pool, and returns
    /// null for a call no longer wanted, which then ends without a word;
    /// `landed` runs on this object's thread with the parsed reply when it
    /// said ok, and `refused` with the message when it did not.
    void call(const QString &what,
              std::function<char *(LogosAccountCore *)> work,
              std::function<void(const QJsonObject &)> landed = {},
              std::function<void(const QString &)> refused = {});

    /// Re-read the vault listing into `accountsJson`, and select the first
    /// account when nothing is selected yet.
    void reloadAccounts();
    /// Re-read `stateJson` for the selected account. Reads no store.
    void reloadState();
    /// Put `address` on screen. The one way the selection changes, so what
    /// described the last account (its notice, its just-published marks) goes
    /// with it.
    void setSelection(const QString &address);
    /// After a staged edit lands: the list and the state are drawn again.
    void edited();

    /// The library's submitter: hands `request` to this object's thread and
    /// waits for the wallet's answer, which stays valid until the next call.
    static const char *submitTrampoline(void *context, const char *request);
    /// Sends one prepared registry write through the LEZ wallet module, paid
    /// by `LOGOS_ACCOUNTS_LEZ_PAYER` or else the wallet's first public account
    /// that can cover it. Runs on this object's thread. Replies as the
    /// library's submitter does.
    QByteArray submitToWallet(const QByteArray &request);
    /// The wallet account that pays for a write, or empty with `problem` set.
    QString payerFor(QString *problem);
    /// The library's relay sender: hands `request` to this object's thread and
    /// waits for the relay's answer, which stays valid until the next call.
    static const char *relayTrampoline(void *context, const char *request);
    /// Runs `work` on this object's thread and waits for it, keeping its
    /// answer in `replyBuffer` until the next call. Gives up once closing.
    const char *answerOnOwnThread(QByteArray (AccountsBackend::*work)(const QByteArray &),
                                  const char *request, QByteArray &replyBuffer);
    /// Sends one submission to the relay at `relayPeer` and `relayAddress`
    /// over the libp2p module. Runs on this object's thread. Replies as the
    /// library's relay sender does.
    QByteArray sendToRelay(const QByteArray &request);
    /// `request` with `LOGOS_ACCOUNTS_RELAY_TOKEN` as its credentials when it
    /// goes to the relay `LOGOS_ACCOUNTS_RELAY_PEER` names, and as it is to any
    /// other: a token is for the relay that issued it.
    QByteArray withToken(const QByteArray &request) const;
    /// Where the relay chosen in the app is kept: beside the vault, not in it.
    static QString settingsPath();
    /// Points publishing at the relay `peer` at `address`, or at none.
    void applyRelay(const QString &peer, const QString &address);
    /// Makes sure the libp2p module runs a node, or says why not.
    bool startLibp2p(QString *problem);

    void setNotice(const QString &kind, const QString &title, const QString &body);
    void report(const QString &what, const QString &message);

    LogosAccountCore *m_core = nullptr;
    /// One thread, so two calls never run against the library at once and the
    /// order the person pressed things in is the order they happen in.
    QThreadPool m_pool;
    int m_inFlight = 0;
    /// Set once the host has handed over, before which the store is not read.
    bool m_contextReady = false;
    /// Moves on with every change of selection, so a store read queued for an
    /// account since left can tell, on the pool, that it is not wanted.
    std::atomic<quint64> m_selection{0};
    /// Staged edits a read dropped while their account was off screen, said
    /// once it is back.
    QHash<QString, int> m_dropped;
    /// Set as the backend goes, so a publish waiting on the wallet gives up
    /// instead of holding the pool the destructor drains.
    std::atomic<bool> m_closing{false};
    /// The last submitter reply, which the library reads before the next call.
    QByteArray m_submitReply;
    /// The last relay reply, which the library reads before the next call.
    QByteArray m_relayReply;
    /// The relay the environment names, and the token sent to it alone.
    QString m_tokenPeer;
    QString m_token;
    bool m_libp2pStarted = false;
    /// Why the one start libp2p gets failed, if it did.
    QString m_libp2pProblem;
};
