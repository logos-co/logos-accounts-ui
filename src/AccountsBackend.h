#pragma once

#include "rep_AccountsBackend_source.h"
#include "logos_api.h"
#include "logos_sdk.h"

#include <QObject>
#include <QString>
#include <QStringList>

class AccountsBackend : public AccountsBackendSimpleSource
{
    Q_OBJECT

public:
    explicit AccountsBackend(LogosAPI* logosAPI, QObject* parent = nullptr);
    ~AccountsBackend() override;

public slots:
    // Keys operations
    QString createRandomMnemonic(int length) override;
    QString createRandomMnemonicDefault() override;
    int lengthToEntropyStrength(int length) override;
    QString createExtKeyFromMnemonic(QString phrase, QString passphrase) override;
    QString deriveExtKey(QString extKey, QString path) override;
    QString extKeyToECDSA(QString extKey) override;
    QString ecdsaToPublicKey(QString privateKeyHex) override;
    QString publicKeyToAddress(QString publicKeyHex) override;

    // Keystore operations
    bool initKeystore(QString dir, int scryptN, int scryptP) override;
    bool closeKeystore() override;
    void refreshKeystoreAccounts() override;
    QString keystoreNewAccount(QString passphrase) override;
    QString keystoreImport(QString keyJSON, QString passphrase, QString newPassphrase) override;
    QString keystoreExport(QString address, QString passphrase, QString newPassphrase) override;
    bool keystoreDelete(QString address, QString passphrase) override;
    bool keystoreHasAddress(QString address) override;
    bool keystoreUnlock(QString address, QString passphrase) override;
    bool keystoreLock(QString address) override;
    bool keystoreTimedUnlock(QString address, QString passphrase, int timeout) override;
    bool keystoreUpdate(QString address, QString passphrase, QString newPassphrase) override;
    QString keystoreSignHash(QString address, QString hashHex) override;
    QString keystoreSignHashWithPassphrase(QString address, QString passphrase, QString hashHex) override;
    QString keystoreImportECDSA(QString privateKeyHex, QString passphrase) override;
    QString keystoreSignTx(QString address, QString txJSON, QString chainIDHex) override;
    QString keystoreSignTxWithPassphrase(QString address, QString passphrase, QString txJSON, QString chainIDHex) override;
    QString keystoreFind(QString address, QString url) override;

    // Extended Keystore operations
    bool initExtKeystore(QString dir, int scryptN, int scryptP) override;
    bool closeExtKeystore() override;
    void refreshExtKeystoreAccounts() override;
    QString extKeystoreNewAccount(QString passphrase) override;
    QString extKeystoreImport(QString keyJSON, QString passphrase, QString newPassphrase) override;
    QString extKeystoreExportExt(QString address, QString passphrase, QString newPassphrase) override;
    QString extKeystoreExportPriv(QString address, QString passphrase, QString newPassphrase) override;
    bool extKeystoreDelete(QString address, QString passphrase) override;
    bool extKeystoreHasAddress(QString address) override;
    bool extKeystoreUnlock(QString address, QString passphrase) override;
    bool extKeystoreLock(QString address) override;
    bool extKeystoreTimedUnlock(QString address, QString passphrase, int timeout) override;
    bool extKeystoreUpdate(QString address, QString passphrase, QString newPassphrase) override;
    QString extKeystoreSignHash(QString address, QString hashHex) override;
    QString extKeystoreSignHashWithPassphrase(QString address, QString passphrase, QString hashHex) override;
    QString extKeystoreImportExtendedKey(QString extKey, QString passphrase) override;
    QString extKeystoreSignTx(QString address, QString txJSON, QString chainIDHex) override;
    QString extKeystoreSignTxWithPassphrase(QString address, QString passphrase, QString txJSON, QString chainIDHex) override;
    QString extKeystoreDerive(QString address, QString path, int pin) override;
    QString extKeystoreDeriveWithPassphrase(QString address, QString path, int pin, QString passphrase, QString newPassphrase) override;
    QString extKeystoreFind(QString address, QString url) override;

private:
    void updateStatus(const QString& message);

    LogosAPI* m_logosAPI;
    LogosModules* m_logos;
};
