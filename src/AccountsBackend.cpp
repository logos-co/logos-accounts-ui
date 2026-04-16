#include "AccountsBackend.h"
#include <QDebug>
#include <QDir>

AccountsBackend::AccountsBackend(LogosAPI* logosAPI, QObject* parent)
    : AccountsBackendSimpleSource(parent), m_logosAPI(logosAPI) {
    if (!m_logosAPI) {
        m_logosAPI = new LogosAPI("core", this);
    }
    m_logos = new LogosModules(m_logosAPI);
    updateStatus("Ready");
}

AccountsBackend::~AccountsBackend() {
}

void AccountsBackend::updateStatus(const QString& message) {
    setStatus(message);
    qDebug() << "Status:" << message;
}

// --- Keys operations ---

QString AccountsBackend::createRandomMnemonic(int length) {
    updateStatus("Creating random mnemonic...");
    QString mnemonic = m_logos->accounts_module.createRandomMnemonic(length);
    updateStatus(mnemonic.isEmpty() ? "Failed to create mnemonic" : "Mnemonic created successfully");
    return mnemonic;
}

QString AccountsBackend::createRandomMnemonicDefault() {
    updateStatus("Creating random mnemonic...");
    QString mnemonic = m_logos->accounts_module.createRandomMnemonicWithDefaultLength();
    updateStatus(mnemonic.isEmpty() ? "Failed to create mnemonic" : "Mnemonic created successfully");
    return mnemonic;
}

int AccountsBackend::lengthToEntropyStrength(int length) {
    updateStatus("Calculating entropy strength...");
    int strength = m_logos->accounts_module.lengthToEntropyStrength(length);
    updateStatus("Entropy strength calculated");
    return strength;
}

QString AccountsBackend::createExtKeyFromMnemonic(QString phrase, QString passphrase) {
    updateStatus("Creating extended key from mnemonic...");
    QString extKey = m_logos->accounts_module.createExtKeyFromMnemonic(phrase, passphrase);
    updateStatus(extKey.isEmpty() ? "Failed to create extended key" : "Extended key created successfully");
    return extKey;
}

QString AccountsBackend::deriveExtKey(QString extKey, QString path) {
    updateStatus("Deriving extended key...");
    QString derivedKey = m_logos->accounts_module.deriveExtKey(extKey, path);
    updateStatus(derivedKey.isEmpty() ? "Failed to derive extended key" : "Extended key derived successfully");
    return derivedKey;
}

QString AccountsBackend::extKeyToECDSA(QString extKey) {
    updateStatus("Converting extended key to ECDSA...");
    QString ecdsaKey = m_logos->accounts_module.extKeyToECDSA(extKey);
    updateStatus(ecdsaKey.isEmpty() ? "Failed to convert extended key" : "Conversion successful");
    return ecdsaKey;
}

QString AccountsBackend::ecdsaToPublicKey(QString privateKeyHex) {
    updateStatus("Converting ECDSA to public key...");
    QString publicKey = m_logos->accounts_module.ecdsaToPublicKey(privateKeyHex);
    updateStatus(publicKey.isEmpty() ? "Failed to convert ECDSA key" : "Conversion successful");
    return publicKey;
}

QString AccountsBackend::publicKeyToAddress(QString publicKeyHex) {
    updateStatus("Converting public key to address...");
    QString address = m_logos->accounts_module.publicKeyToAddress(publicKeyHex);
    updateStatus(address.isEmpty() ? "Failed to convert public key" : "Conversion successful");
    return address;
}

// --- Keystore operations ---

bool AccountsBackend::initKeystore(QString dir, int scryptN, int scryptP) {
    updateStatus("Initializing keystore...");
    bool success = m_logos->accounts_module.initKeystore(dir, scryptN, scryptP);
    updateStatus(success ? "Keystore initialized successfully" : "Failed to initialize keystore");
    if (success) refreshKeystoreAccounts();
    return success;
}

bool AccountsBackend::closeKeystore() {
    updateStatus("Closing keystore...");
    bool success = m_logos->accounts_module.closeKeystore("");
    updateStatus(success ? "Keystore closed successfully" : "Failed to close keystore");
    return success;
}

void AccountsBackend::refreshKeystoreAccounts() {
    updateStatus("Listing accounts...");
    setKeystoreAccounts(m_logos->accounts_module.keystoreAccounts());
    updateStatus(QString("Found %1 accounts").arg(keystoreAccounts().size()));
}

QString AccountsBackend::keystoreNewAccount(QString passphrase) {
    updateStatus("Creating new account...");
    QString address = m_logos->accounts_module.keystoreNewAccount(passphrase);
    if (!address.isEmpty()) {
        updateStatus(QString("New account created: %1").arg(address));
        refreshKeystoreAccounts();
    } else {
        updateStatus("Failed to create new account");
    }
    return address;
}

QString AccountsBackend::keystoreImport(QString keyJSON, QString passphrase, QString newPassphrase) {
    updateStatus("Importing account...");
    QString address = m_logos->accounts_module.keystoreImport(keyJSON, passphrase, newPassphrase);
    if (!address.isEmpty()) {
        updateStatus(QString("Account imported: %1").arg(address));
        refreshKeystoreAccounts();
    } else {
        updateStatus("Failed to import account");
    }
    return address;
}

QString AccountsBackend::keystoreExport(QString address, QString passphrase, QString newPassphrase) {
    updateStatus("Exporting account...");
    QString keyJSON = m_logos->accounts_module.keystoreExport(address, passphrase, newPassphrase);
    updateStatus(keyJSON.isEmpty() ? "Failed to export account" : QString("Account exported: %1").arg(address));
    return keyJSON;
}

bool AccountsBackend::keystoreDelete(QString address, QString passphrase) {
    updateStatus("Deleting account...");
    bool success = m_logos->accounts_module.keystoreDelete(address, passphrase);
    if (success) {
        updateStatus(QString("Account deleted: %1").arg(address));
        refreshKeystoreAccounts();
    } else {
        updateStatus("Failed to delete account");
    }
    return success;
}

bool AccountsBackend::keystoreHasAddress(QString address) {
    updateStatus("Checking if address exists...");
    bool has = m_logos->accounts_module.keystoreHasAddress(address);
    updateStatus(has ? "Address exists" : "Address does not exist");
    return has;
}

bool AccountsBackend::keystoreUnlock(QString address, QString passphrase) {
    updateStatus("Unlocking account...");
    bool success = m_logos->accounts_module.keystoreUnlock(address, passphrase);
    updateStatus(success ? QString("Account unlocked: %1").arg(address) : "Failed to unlock account");
    return success;
}

bool AccountsBackend::keystoreLock(QString address) {
    updateStatus("Locking account...");
    bool success = m_logos->accounts_module.keystoreLock(address);
    updateStatus(success ? QString("Account locked: %1").arg(address) : "Failed to lock account");
    return success;
}

bool AccountsBackend::keystoreTimedUnlock(QString address, QString passphrase, int timeout) {
    updateStatus("Unlocking account with timeout...");
    bool success = m_logos->accounts_module.keystoreTimedUnlock(address, passphrase, timeout);
    updateStatus(success ? QString("Account unlocked with timeout: %1").arg(address) : "Failed to unlock account");
    return success;
}

bool AccountsBackend::keystoreUpdate(QString address, QString passphrase, QString newPassphrase) {
    updateStatus("Updating passphrase...");
    bool success = m_logos->accounts_module.keystoreUpdate(address, passphrase, newPassphrase);
    updateStatus(success ? QString("Passphrase updated: %1").arg(address) : "Failed to update passphrase");
    return success;
}

QString AccountsBackend::keystoreSignHash(QString address, QString hashHex) {
    updateStatus("Signing hash...");
    QString sig = m_logos->accounts_module.keystoreSignHash(address, hashHex);
    updateStatus(sig.isEmpty() ? "Failed to sign hash" : "Hash signed successfully");
    return sig;
}

QString AccountsBackend::keystoreSignHashWithPassphrase(QString address, QString passphrase, QString hashHex) {
    updateStatus("Signing hash with passphrase...");
    QString sig = m_logos->accounts_module.keystoreSignHashWithPassphrase(address, passphrase, hashHex);
    updateStatus(sig.isEmpty() ? "Failed to sign hash" : "Hash signed successfully");
    return sig;
}

QString AccountsBackend::keystoreImportECDSA(QString privateKeyHex, QString passphrase) {
    updateStatus("Importing ECDSA private key...");
    QString address = m_logos->accounts_module.keystoreImportECDSA(privateKeyHex, passphrase);
    if (!address.isEmpty()) {
        updateStatus(QString("ECDSA key imported: %1").arg(address));
        refreshKeystoreAccounts();
    } else {
        updateStatus("Failed to import ECDSA key");
    }
    return address;
}

QString AccountsBackend::keystoreSignTx(QString address, QString txJSON, QString chainIDHex) {
    updateStatus("Signing transaction...");
    QString signedTx = m_logos->accounts_module.keystoreSignTx(address, txJSON, chainIDHex);
    updateStatus(signedTx.isEmpty() ? "Failed to sign transaction" : "Transaction signed successfully");
    return signedTx;
}

QString AccountsBackend::keystoreSignTxWithPassphrase(QString address, QString passphrase, QString txJSON, QString chainIDHex) {
    updateStatus("Signing transaction with passphrase...");
    QString signedTx = m_logos->accounts_module.keystoreSignTxWithPassphrase(address, passphrase, txJSON, chainIDHex);
    updateStatus(signedTx.isEmpty() ? "Failed to sign transaction" : "Transaction signed successfully");
    return signedTx;
}

QString AccountsBackend::keystoreFind(QString address, QString url) {
    updateStatus("Finding account...");
    QString result = m_logos->accounts_module.keystoreFind(address, url);
    updateStatus(result.isEmpty() ? "Account not found" : "Account found");
    return result;
}

// --- Extended Keystore operations ---

bool AccountsBackend::initExtKeystore(QString dir, int scryptN, int scryptP) {
    updateStatus("Initializing extended keystore...");
    bool success = m_logos->accounts_module.initExtKeystore(dir, scryptN, scryptP);
    updateStatus(success ? "Extended keystore initialized successfully" : "Failed to initialize extended keystore");
    if (success) refreshExtKeystoreAccounts();
    return success;
}

bool AccountsBackend::closeExtKeystore() {
    updateStatus("Closing extended keystore...");
    bool success = m_logos->accounts_module.closeExtKeystore();
    updateStatus(success ? "Extended keystore closed successfully" : "Failed to close extended keystore");
    return success;
}

void AccountsBackend::refreshExtKeystoreAccounts() {
    updateStatus("Listing accounts...");
    setExtKeystoreAccounts(m_logos->accounts_module.extKeystoreAccounts());
    updateStatus(QString("Found %1 accounts").arg(extKeystoreAccounts().size()));
}

QString AccountsBackend::extKeystoreNewAccount(QString passphrase) {
    updateStatus("Creating new account...");
    QString address = m_logos->accounts_module.extKeystoreNewAccount(passphrase);
    if (!address.isEmpty()) {
        updateStatus(QString("New account created: %1").arg(address));
        refreshExtKeystoreAccounts();
    } else {
        updateStatus("Failed to create new account");
    }
    return address;
}

QString AccountsBackend::extKeystoreImport(QString keyJSON, QString passphrase, QString newPassphrase) {
    updateStatus("Importing account...");
    QString address = m_logos->accounts_module.extKeystoreImport(keyJSON, passphrase, newPassphrase);
    if (!address.isEmpty()) {
        updateStatus(QString("Account imported: %1").arg(address));
        refreshExtKeystoreAccounts();
    } else {
        updateStatus("Failed to import account");
    }
    return address;
}

QString AccountsBackend::extKeystoreExportExt(QString address, QString passphrase, QString newPassphrase) {
    updateStatus("Exporting extended key...");
    QString keyJSON = m_logos->accounts_module.extKeystoreExportExt(address, passphrase, newPassphrase);
    updateStatus(keyJSON.isEmpty() ? "Failed to export extended key" : QString("Extended key exported: %1").arg(address));
    return keyJSON;
}

QString AccountsBackend::extKeystoreExportPriv(QString address, QString passphrase, QString newPassphrase) {
    updateStatus("Exporting private key...");
    QString keyJSON = m_logos->accounts_module.extKeystoreExportPriv(address, passphrase, newPassphrase);
    updateStatus(keyJSON.isEmpty() ? "Failed to export private key" : QString("Private key exported: %1").arg(address));
    return keyJSON;
}

bool AccountsBackend::extKeystoreDelete(QString address, QString passphrase) {
    updateStatus("Deleting account...");
    bool success = m_logos->accounts_module.extKeystoreDelete(address, passphrase);
    if (success) {
        updateStatus(QString("Account deleted: %1").arg(address));
        refreshExtKeystoreAccounts();
    } else {
        updateStatus("Failed to delete account");
    }
    return success;
}

bool AccountsBackend::extKeystoreHasAddress(QString address) {
    updateStatus("Checking if address exists...");
    bool has = m_logos->accounts_module.extKeystoreHasAddress(address);
    updateStatus(has ? "Address exists" : "Address does not exist");
    return has;
}

bool AccountsBackend::extKeystoreUnlock(QString address, QString passphrase) {
    updateStatus("Unlocking account...");
    bool success = m_logos->accounts_module.extKeystoreUnlock(address, passphrase);
    updateStatus(success ? QString("Account unlocked: %1").arg(address) : "Failed to unlock account");
    return success;
}

bool AccountsBackend::extKeystoreLock(QString address) {
    updateStatus("Locking account...");
    bool success = m_logos->accounts_module.extKeystoreLock(address);
    updateStatus(success ? QString("Account locked: %1").arg(address) : "Failed to lock account");
    return success;
}

bool AccountsBackend::extKeystoreTimedUnlock(QString address, QString passphrase, int timeout) {
    updateStatus("Unlocking account with timeout...");
    bool success = m_logos->accounts_module.extKeystoreTimedUnlock(address, passphrase, timeout);
    updateStatus(success ? QString("Account unlocked with timeout: %1").arg(address) : "Failed to unlock account");
    return success;
}

bool AccountsBackend::extKeystoreUpdate(QString address, QString passphrase, QString newPassphrase) {
    updateStatus("Updating passphrase...");
    bool success = m_logos->accounts_module.extKeystoreUpdate(address, passphrase, newPassphrase);
    updateStatus(success ? QString("Passphrase updated: %1").arg(address) : "Failed to update passphrase");
    return success;
}

QString AccountsBackend::extKeystoreSignHash(QString address, QString hashHex) {
    updateStatus("Signing hash...");
    QString sig = m_logos->accounts_module.extKeystoreSignHash(address, hashHex);
    updateStatus(sig.isEmpty() ? "Failed to sign hash" : "Hash signed successfully");
    return sig;
}

QString AccountsBackend::extKeystoreSignHashWithPassphrase(QString address, QString passphrase, QString hashHex) {
    updateStatus("Signing hash with passphrase...");
    QString sig = m_logos->accounts_module.extKeystoreSignHashWithPassphrase(address, passphrase, hashHex);
    updateStatus(sig.isEmpty() ? "Failed to sign hash" : "Hash signed successfully");
    return sig;
}

QString AccountsBackend::extKeystoreImportExtendedKey(QString extKey, QString passphrase) {
    updateStatus("Importing extended key...");
    QString address = m_logos->accounts_module.extKeystoreImportExtendedKey(extKey, passphrase);
    if (!address.isEmpty()) {
        updateStatus(QString("Extended key imported: %1").arg(address));
        refreshExtKeystoreAccounts();
    } else {
        updateStatus("Failed to import extended key");
    }
    return address;
}

QString AccountsBackend::extKeystoreSignTx(QString address, QString txJSON, QString chainIDHex) {
    updateStatus("Signing transaction...");
    QString signedTx = m_logos->accounts_module.extKeystoreSignTx(address, txJSON, chainIDHex);
    updateStatus(signedTx.isEmpty() ? "Failed to sign transaction" : "Transaction signed successfully");
    return signedTx;
}

QString AccountsBackend::extKeystoreSignTxWithPassphrase(QString address, QString passphrase, QString txJSON, QString chainIDHex) {
    updateStatus("Signing transaction with passphrase...");
    QString signedTx = m_logos->accounts_module.extKeystoreSignTxWithPassphrase(address, passphrase, txJSON, chainIDHex);
    updateStatus(signedTx.isEmpty() ? "Failed to sign transaction" : "Transaction signed successfully");
    return signedTx;
}

QString AccountsBackend::extKeystoreDerive(QString address, QString path, int pin) {
    updateStatus("Deriving address...");
    QString derivedAddress = m_logos->accounts_module.extKeystoreDerive(address, path, pin);
    updateStatus(derivedAddress.isEmpty() ? "Failed to derive address" : "Address derived successfully");
    return derivedAddress;
}

QString AccountsBackend::extKeystoreDeriveWithPassphrase(QString address, QString path, int pin, QString passphrase, QString newPassphrase) {
    updateStatus("Deriving address with passphrase...");
    QString derivedAddress = m_logos->accounts_module.extKeystoreDeriveWithPassphrase(address, path, pin, passphrase, newPassphrase);
    updateStatus(derivedAddress.isEmpty() ? "Failed to derive address" : "Address derived successfully");
    return derivedAddress;
}

QString AccountsBackend::extKeystoreFind(QString address, QString url) {
    updateStatus("Finding account...");
    QString result = m_logos->accounts_module.extKeystoreFind(address, url);
    updateStatus(result.isEmpty() ? "Account not found" : "Account found");
    return result;
}
