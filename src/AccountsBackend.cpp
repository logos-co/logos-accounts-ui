#include "AccountsBackend.h"
#include <QDebug>
#include <QDir>

AccountsBackend::AccountsBackend(LogosAPI* logosAPI, QObject* parent)
    : QObject(parent), m_logosAPI(logosAPI) {
    if (!m_logosAPI) {
        m_logosAPI = new LogosAPI("core", this);
    }
    m_logos = new LogosModules(m_logosAPI);
    setStatus("Ready");
}

AccountsBackend::~AccountsBackend() {
}

void AccountsBackend::setStatus(const QString& message) {
    m_status = message;
    emit statusChanged();
    qDebug() << "Status:" << message;
}

// --- Keys operations ---

QString AccountsBackend::createRandomMnemonic(int length) {
    setStatus("Creating random mnemonic...");
    QString mnemonic = m_logos->accounts_module.createRandomMnemonic(length);
    setStatus(mnemonic.isEmpty() ? "Failed to create mnemonic" : "Mnemonic created successfully");
    return mnemonic;
}

QString AccountsBackend::createRandomMnemonicDefault() {
    setStatus("Creating random mnemonic...");
    QString mnemonic = m_logos->accounts_module.createRandomMnemonicWithDefaultLength();
    setStatus(mnemonic.isEmpty() ? "Failed to create mnemonic" : "Mnemonic created successfully");
    return mnemonic;
}

int AccountsBackend::lengthToEntropyStrength(int length) {
    setStatus("Calculating entropy strength...");
    int strength = m_logos->accounts_module.lengthToEntropyStrength(length);
    setStatus("Entropy strength calculated");
    return strength;
}

QString AccountsBackend::createExtKeyFromMnemonic(const QString& phrase, const QString& passphrase) {
    setStatus("Creating extended key from mnemonic...");
    QString extKey = m_logos->accounts_module.createExtKeyFromMnemonic(phrase, passphrase);
    setStatus(extKey.isEmpty() ? "Failed to create extended key" : "Extended key created successfully");
    return extKey;
}

QString AccountsBackend::deriveExtKey(const QString& extKey, const QString& path) {
    setStatus("Deriving extended key...");
    QString derivedKey = m_logos->accounts_module.deriveExtKey(extKey, path);
    setStatus(derivedKey.isEmpty() ? "Failed to derive extended key" : "Extended key derived successfully");
    return derivedKey;
}

QString AccountsBackend::extKeyToECDSA(const QString& extKey) {
    setStatus("Converting extended key to ECDSA...");
    QString ecdsaKey = m_logos->accounts_module.extKeyToECDSA(extKey);
    setStatus(ecdsaKey.isEmpty() ? "Failed to convert extended key" : "Conversion successful");
    return ecdsaKey;
}

QString AccountsBackend::ecdsaToPublicKey(const QString& privateKeyHex) {
    setStatus("Converting ECDSA to public key...");
    QString publicKey = m_logos->accounts_module.ecdsaToPublicKey(privateKeyHex);
    setStatus(publicKey.isEmpty() ? "Failed to convert ECDSA key" : "Conversion successful");
    return publicKey;
}

QString AccountsBackend::publicKeyToAddress(const QString& publicKeyHex) {
    setStatus("Converting public key to address...");
    QString address = m_logos->accounts_module.publicKeyToAddress(publicKeyHex);
    setStatus(address.isEmpty() ? "Failed to convert public key" : "Conversion successful");
    return address;
}

// --- Keystore operations ---

bool AccountsBackend::initKeystore(const QString& dir, int scryptN, int scryptP) {
    setStatus("Initializing keystore...");
    bool success = m_logos->accounts_module.initKeystore(dir, scryptN, scryptP);
    setStatus(success ? "Keystore initialized successfully" : "Failed to initialize keystore");
    if (success) refreshKeystoreAccounts();
    return success;
}

bool AccountsBackend::closeKeystore() {
    setStatus("Closing keystore...");
    bool success = m_logos->accounts_module.closeKeystore("");
    setStatus(success ? "Keystore closed successfully" : "Failed to close keystore");
    return success;
}

void AccountsBackend::refreshKeystoreAccounts() {
    setStatus("Listing accounts...");
    m_keystoreAccounts = m_logos->accounts_module.keystoreAccounts();
    emit keystoreAccountsChanged();
    setStatus(QString("Found %1 accounts").arg(m_keystoreAccounts.size()));
}

QString AccountsBackend::keystoreNewAccount(const QString& passphrase) {
    setStatus("Creating new account...");
    QString address = m_logos->accounts_module.keystoreNewAccount(passphrase);
    if (!address.isEmpty()) {
        setStatus(QString("New account created: %1").arg(address));
        refreshKeystoreAccounts();
    } else {
        setStatus("Failed to create new account");
    }
    return address;
}

QString AccountsBackend::keystoreImport(const QString& keyJSON, const QString& passphrase, const QString& newPassphrase) {
    setStatus("Importing account...");
    QString address = m_logos->accounts_module.keystoreImport(keyJSON, passphrase, newPassphrase);
    if (!address.isEmpty()) {
        setStatus(QString("Account imported: %1").arg(address));
        refreshKeystoreAccounts();
    } else {
        setStatus("Failed to import account");
    }
    return address;
}

QString AccountsBackend::keystoreExport(const QString& address, const QString& passphrase, const QString& newPassphrase) {
    setStatus("Exporting account...");
    QString keyJSON = m_logos->accounts_module.keystoreExport(address, passphrase, newPassphrase);
    setStatus(keyJSON.isEmpty() ? "Failed to export account" : QString("Account exported: %1").arg(address));
    return keyJSON;
}

bool AccountsBackend::keystoreDelete(const QString& address, const QString& passphrase) {
    setStatus("Deleting account...");
    bool success = m_logos->accounts_module.keystoreDelete(address, passphrase);
    if (success) {
        setStatus(QString("Account deleted: %1").arg(address));
        refreshKeystoreAccounts();
    } else {
        setStatus("Failed to delete account");
    }
    return success;
}

bool AccountsBackend::keystoreHasAddress(const QString& address) {
    setStatus("Checking if address exists...");
    bool has = m_logos->accounts_module.keystoreHasAddress(address);
    setStatus(has ? "Address exists" : "Address does not exist");
    return has;
}

bool AccountsBackend::keystoreUnlock(const QString& address, const QString& passphrase) {
    setStatus("Unlocking account...");
    bool success = m_logos->accounts_module.keystoreUnlock(address, passphrase);
    setStatus(success ? QString("Account unlocked: %1").arg(address) : "Failed to unlock account");
    return success;
}

bool AccountsBackend::keystoreLock(const QString& address) {
    setStatus("Locking account...");
    bool success = m_logos->accounts_module.keystoreLock(address);
    setStatus(success ? QString("Account locked: %1").arg(address) : "Failed to lock account");
    return success;
}

bool AccountsBackend::keystoreTimedUnlock(const QString& address, const QString& passphrase, int timeout) {
    setStatus("Unlocking account with timeout...");
    bool success = m_logos->accounts_module.keystoreTimedUnlock(address, passphrase, timeout);
    setStatus(success ? QString("Account unlocked with timeout: %1").arg(address) : "Failed to unlock account");
    return success;
}

bool AccountsBackend::keystoreUpdate(const QString& address, const QString& passphrase, const QString& newPassphrase) {
    setStatus("Updating passphrase...");
    bool success = m_logos->accounts_module.keystoreUpdate(address, passphrase, newPassphrase);
    setStatus(success ? QString("Passphrase updated: %1").arg(address) : "Failed to update passphrase");
    return success;
}

QString AccountsBackend::keystoreSignHash(const QString& address, const QString& hashHex) {
    setStatus("Signing hash...");
    QString sig = m_logos->accounts_module.keystoreSignHash(address, hashHex);
    setStatus(sig.isEmpty() ? "Failed to sign hash" : "Hash signed successfully");
    return sig;
}

QString AccountsBackend::keystoreSignHashWithPassphrase(const QString& address, const QString& passphrase, const QString& hashHex) {
    setStatus("Signing hash with passphrase...");
    QString sig = m_logos->accounts_module.keystoreSignHashWithPassphrase(address, passphrase, hashHex);
    setStatus(sig.isEmpty() ? "Failed to sign hash" : "Hash signed successfully");
    return sig;
}

QString AccountsBackend::keystoreImportECDSA(const QString& privateKeyHex, const QString& passphrase) {
    setStatus("Importing ECDSA private key...");
    QString address = m_logos->accounts_module.keystoreImportECDSA(privateKeyHex, passphrase);
    if (!address.isEmpty()) {
        setStatus(QString("ECDSA key imported: %1").arg(address));
        refreshKeystoreAccounts();
    } else {
        setStatus("Failed to import ECDSA key");
    }
    return address;
}

QString AccountsBackend::keystoreSignTx(const QString& address, const QString& txJSON, const QString& chainIDHex) {
    setStatus("Signing transaction...");
    QString signedTx = m_logos->accounts_module.keystoreSignTx(address, txJSON, chainIDHex);
    setStatus(signedTx.isEmpty() ? "Failed to sign transaction" : "Transaction signed successfully");
    return signedTx;
}

QString AccountsBackend::keystoreSignTxWithPassphrase(const QString& address, const QString& passphrase, const QString& txJSON, const QString& chainIDHex) {
    setStatus("Signing transaction with passphrase...");
    QString signedTx = m_logos->accounts_module.keystoreSignTxWithPassphrase(address, passphrase, txJSON, chainIDHex);
    setStatus(signedTx.isEmpty() ? "Failed to sign transaction" : "Transaction signed successfully");
    return signedTx;
}

QString AccountsBackend::keystoreFind(const QString& address, const QString& url) {
    setStatus("Finding account...");
    QString result = m_logos->accounts_module.keystoreFind(address, url);
    setStatus(result.isEmpty() ? "Account not found" : "Account found");
    return result;
}

// --- Extended Keystore operations ---

bool AccountsBackend::initExtKeystore(const QString& dir, int scryptN, int scryptP) {
    setStatus("Initializing extended keystore...");
    bool success = m_logos->accounts_module.initExtKeystore(dir, scryptN, scryptP);
    setStatus(success ? "Extended keystore initialized successfully" : "Failed to initialize extended keystore");
    if (success) refreshExtKeystoreAccounts();
    return success;
}

bool AccountsBackend::closeExtKeystore() {
    setStatus("Closing extended keystore...");
    bool success = m_logos->accounts_module.closeExtKeystore();
    setStatus(success ? "Extended keystore closed successfully" : "Failed to close extended keystore");
    return success;
}

void AccountsBackend::refreshExtKeystoreAccounts() {
    setStatus("Listing accounts...");
    m_extKeystoreAccounts = m_logos->accounts_module.extKeystoreAccounts();
    emit extKeystoreAccountsChanged();
    setStatus(QString("Found %1 accounts").arg(m_extKeystoreAccounts.size()));
}

QString AccountsBackend::extKeystoreNewAccount(const QString& passphrase) {
    setStatus("Creating new account...");
    QString address = m_logos->accounts_module.extKeystoreNewAccount(passphrase);
    if (!address.isEmpty()) {
        setStatus(QString("New account created: %1").arg(address));
        refreshExtKeystoreAccounts();
    } else {
        setStatus("Failed to create new account");
    }
    return address;
}

QString AccountsBackend::extKeystoreImport(const QString& keyJSON, const QString& passphrase, const QString& newPassphrase) {
    setStatus("Importing account...");
    QString address = m_logos->accounts_module.extKeystoreImport(keyJSON, passphrase, newPassphrase);
    if (!address.isEmpty()) {
        setStatus(QString("Account imported: %1").arg(address));
        refreshExtKeystoreAccounts();
    } else {
        setStatus("Failed to import account");
    }
    return address;
}

QString AccountsBackend::extKeystoreExportExt(const QString& address, const QString& passphrase, const QString& newPassphrase) {
    setStatus("Exporting extended key...");
    QString keyJSON = m_logos->accounts_module.extKeystoreExportExt(address, passphrase, newPassphrase);
    setStatus(keyJSON.isEmpty() ? "Failed to export extended key" : QString("Extended key exported: %1").arg(address));
    return keyJSON;
}

QString AccountsBackend::extKeystoreExportPriv(const QString& address, const QString& passphrase, const QString& newPassphrase) {
    setStatus("Exporting private key...");
    QString keyJSON = m_logos->accounts_module.extKeystoreExportPriv(address, passphrase, newPassphrase);
    setStatus(keyJSON.isEmpty() ? "Failed to export private key" : QString("Private key exported: %1").arg(address));
    return keyJSON;
}

bool AccountsBackend::extKeystoreDelete(const QString& address, const QString& passphrase) {
    setStatus("Deleting account...");
    bool success = m_logos->accounts_module.extKeystoreDelete(address, passphrase);
    if (success) {
        setStatus(QString("Account deleted: %1").arg(address));
        refreshExtKeystoreAccounts();
    } else {
        setStatus("Failed to delete account");
    }
    return success;
}

bool AccountsBackend::extKeystoreHasAddress(const QString& address) {
    setStatus("Checking if address exists...");
    bool has = m_logos->accounts_module.extKeystoreHasAddress(address);
    setStatus(has ? "Address exists" : "Address does not exist");
    return has;
}

bool AccountsBackend::extKeystoreUnlock(const QString& address, const QString& passphrase) {
    setStatus("Unlocking account...");
    bool success = m_logos->accounts_module.extKeystoreUnlock(address, passphrase);
    setStatus(success ? QString("Account unlocked: %1").arg(address) : "Failed to unlock account");
    return success;
}

bool AccountsBackend::extKeystoreLock(const QString& address) {
    setStatus("Locking account...");
    bool success = m_logos->accounts_module.extKeystoreLock(address);
    setStatus(success ? QString("Account locked: %1").arg(address) : "Failed to lock account");
    return success;
}

bool AccountsBackend::extKeystoreTimedUnlock(const QString& address, const QString& passphrase, int timeout) {
    setStatus("Unlocking account with timeout...");
    bool success = m_logos->accounts_module.extKeystoreTimedUnlock(address, passphrase, timeout);
    setStatus(success ? QString("Account unlocked with timeout: %1").arg(address) : "Failed to unlock account");
    return success;
}

bool AccountsBackend::extKeystoreUpdate(const QString& address, const QString& passphrase, const QString& newPassphrase) {
    setStatus("Updating passphrase...");
    bool success = m_logos->accounts_module.extKeystoreUpdate(address, passphrase, newPassphrase);
    setStatus(success ? QString("Passphrase updated: %1").arg(address) : "Failed to update passphrase");
    return success;
}

QString AccountsBackend::extKeystoreSignHash(const QString& address, const QString& hashHex) {
    setStatus("Signing hash...");
    QString sig = m_logos->accounts_module.extKeystoreSignHash(address, hashHex);
    setStatus(sig.isEmpty() ? "Failed to sign hash" : "Hash signed successfully");
    return sig;
}

QString AccountsBackend::extKeystoreSignHashWithPassphrase(const QString& address, const QString& passphrase, const QString& hashHex) {
    setStatus("Signing hash with passphrase...");
    QString sig = m_logos->accounts_module.extKeystoreSignHashWithPassphrase(address, passphrase, hashHex);
    setStatus(sig.isEmpty() ? "Failed to sign hash" : "Hash signed successfully");
    return sig;
}

QString AccountsBackend::extKeystoreImportExtendedKey(const QString& extKey, const QString& passphrase) {
    setStatus("Importing extended key...");
    QString address = m_logos->accounts_module.extKeystoreImportExtendedKey(extKey, passphrase);
    if (!address.isEmpty()) {
        setStatus(QString("Extended key imported: %1").arg(address));
        refreshExtKeystoreAccounts();
    } else {
        setStatus("Failed to import extended key");
    }
    return address;
}

QString AccountsBackend::extKeystoreSignTx(const QString& address, const QString& txJSON, const QString& chainIDHex) {
    setStatus("Signing transaction...");
    QString signedTx = m_logos->accounts_module.extKeystoreSignTx(address, txJSON, chainIDHex);
    setStatus(signedTx.isEmpty() ? "Failed to sign transaction" : "Transaction signed successfully");
    return signedTx;
}

QString AccountsBackend::extKeystoreSignTxWithPassphrase(const QString& address, const QString& passphrase, const QString& txJSON, const QString& chainIDHex) {
    setStatus("Signing transaction with passphrase...");
    QString signedTx = m_logos->accounts_module.extKeystoreSignTxWithPassphrase(address, passphrase, txJSON, chainIDHex);
    setStatus(signedTx.isEmpty() ? "Failed to sign transaction" : "Transaction signed successfully");
    return signedTx;
}

QString AccountsBackend::extKeystoreDerive(const QString& address, const QString& path, int pin) {
    setStatus("Deriving address...");
    QString derivedAddress = m_logos->accounts_module.extKeystoreDerive(address, path, pin);
    setStatus(derivedAddress.isEmpty() ? "Failed to derive address" : "Address derived successfully");
    return derivedAddress;
}

QString AccountsBackend::extKeystoreDeriveWithPassphrase(const QString& address, const QString& path, int pin, const QString& passphrase, const QString& newPassphrase) {
    setStatus("Deriving address with passphrase...");
    QString derivedAddress = m_logos->accounts_module.extKeystoreDeriveWithPassphrase(address, path, pin, passphrase, newPassphrase);
    setStatus(derivedAddress.isEmpty() ? "Failed to derive address" : "Address derived successfully");
    return derivedAddress;
}

QString AccountsBackend::extKeystoreFind(const QString& address, const QString& url) {
    setStatus("Finding account...");
    QString result = m_logos->accounts_module.extKeystoreFind(address, url);
    setStatus(result.isEmpty() ? "Account not found" : "Account found");
    return result;
}
