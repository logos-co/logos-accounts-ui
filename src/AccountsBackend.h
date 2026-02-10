#pragma once

#include <QObject>
#include <QStringList>
#include "logos_api.h"
#include "logos_api_client.h"
#include "logos_sdk.h"

class AccountsBackend : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QStringList keystoreAccounts READ keystoreAccountsList NOTIFY keystoreAccountsChanged)
    Q_PROPERTY(QStringList extKeystoreAccounts READ extKeystoreAccountsList NOTIFY extKeystoreAccountsChanged)

public:
    explicit AccountsBackend(LogosAPI* logosAPI, QObject* parent = nullptr);
    ~AccountsBackend();

    QString status() const { return m_status; }
    QStringList keystoreAccountsList() const { return m_keystoreAccounts; }
    QStringList extKeystoreAccountsList() const { return m_extKeystoreAccounts; }

    // --- Keys operations ---
    Q_INVOKABLE QString createRandomMnemonic(int length);
    Q_INVOKABLE QString createRandomMnemonicDefault();
    Q_INVOKABLE int lengthToEntropyStrength(int length);
    Q_INVOKABLE QString createExtKeyFromMnemonic(const QString& phrase, const QString& passphrase);
    Q_INVOKABLE QString deriveExtKey(const QString& extKey, const QString& path);
    Q_INVOKABLE QString extKeyToECDSA(const QString& extKey);
    Q_INVOKABLE QString ecdsaToPublicKey(const QString& privateKeyHex);
    Q_INVOKABLE QString publicKeyToAddress(const QString& publicKeyHex);

    // --- Keystore operations ---
    Q_INVOKABLE bool initKeystore(const QString& dir, int scryptN, int scryptP);
    Q_INVOKABLE bool closeKeystore();
    Q_INVOKABLE void refreshKeystoreAccounts();
    Q_INVOKABLE QString keystoreNewAccount(const QString& passphrase);
    Q_INVOKABLE QString keystoreImport(const QString& keyJSON, const QString& passphrase, const QString& newPassphrase);
    Q_INVOKABLE QString keystoreExport(const QString& address, const QString& passphrase, const QString& newPassphrase);
    Q_INVOKABLE bool keystoreDelete(const QString& address, const QString& passphrase);
    Q_INVOKABLE bool keystoreHasAddress(const QString& address);
    Q_INVOKABLE bool keystoreUnlock(const QString& address, const QString& passphrase);
    Q_INVOKABLE bool keystoreLock(const QString& address);
    Q_INVOKABLE bool keystoreTimedUnlock(const QString& address, const QString& passphrase, int timeout);
    Q_INVOKABLE bool keystoreUpdate(const QString& address, const QString& passphrase, const QString& newPassphrase);
    Q_INVOKABLE QString keystoreSignHash(const QString& address, const QString& hashHex);
    Q_INVOKABLE QString keystoreSignHashWithPassphrase(const QString& address, const QString& passphrase, const QString& hashHex);
    Q_INVOKABLE QString keystoreImportECDSA(const QString& privateKeyHex, const QString& passphrase);
    Q_INVOKABLE QString keystoreSignTx(const QString& address, const QString& txJSON, const QString& chainIDHex);
    Q_INVOKABLE QString keystoreSignTxWithPassphrase(const QString& address, const QString& passphrase, const QString& txJSON, const QString& chainIDHex);
    Q_INVOKABLE QString keystoreFind(const QString& address, const QString& url);

    // --- Extended Keystore operations ---
    Q_INVOKABLE bool initExtKeystore(const QString& dir, int scryptN, int scryptP);
    Q_INVOKABLE bool closeExtKeystore();
    Q_INVOKABLE void refreshExtKeystoreAccounts();
    Q_INVOKABLE QString extKeystoreNewAccount(const QString& passphrase);
    Q_INVOKABLE QString extKeystoreImport(const QString& keyJSON, const QString& passphrase, const QString& newPassphrase);
    Q_INVOKABLE QString extKeystoreExportExt(const QString& address, const QString& passphrase, const QString& newPassphrase);
    Q_INVOKABLE QString extKeystoreExportPriv(const QString& address, const QString& passphrase, const QString& newPassphrase);
    Q_INVOKABLE bool extKeystoreDelete(const QString& address, const QString& passphrase);
    Q_INVOKABLE bool extKeystoreHasAddress(const QString& address);
    Q_INVOKABLE bool extKeystoreUnlock(const QString& address, const QString& passphrase);
    Q_INVOKABLE bool extKeystoreLock(const QString& address);
    Q_INVOKABLE bool extKeystoreTimedUnlock(const QString& address, const QString& passphrase, int timeout);
    Q_INVOKABLE bool extKeystoreUpdate(const QString& address, const QString& passphrase, const QString& newPassphrase);
    Q_INVOKABLE QString extKeystoreSignHash(const QString& address, const QString& hashHex);
    Q_INVOKABLE QString extKeystoreSignHashWithPassphrase(const QString& address, const QString& passphrase, const QString& hashHex);
    Q_INVOKABLE QString extKeystoreImportExtendedKey(const QString& extKey, const QString& passphrase);
    Q_INVOKABLE QString extKeystoreSignTx(const QString& address, const QString& txJSON, const QString& chainIDHex);
    Q_INVOKABLE QString extKeystoreSignTxWithPassphrase(const QString& address, const QString& passphrase, const QString& txJSON, const QString& chainIDHex);
    Q_INVOKABLE QString extKeystoreDerive(const QString& address, const QString& path, int pin);
    Q_INVOKABLE QString extKeystoreDeriveWithPassphrase(const QString& address, const QString& path, int pin, const QString& passphrase, const QString& newPassphrase);
    Q_INVOKABLE QString extKeystoreFind(const QString& address, const QString& url);

signals:
    void statusChanged();
    void keystoreAccountsChanged();
    void extKeystoreAccountsChanged();

private:
    void setStatus(const QString& message);

    LogosAPI* m_logosAPI;
    LogosModules* m_logos;
    QString m_status;
    QStringList m_keystoreAccounts;
    QStringList m_extKeystoreAccounts;
};
