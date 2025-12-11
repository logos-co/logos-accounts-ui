#include "KeystoreWidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QDir>

KeystoreWidget::KeystoreWidget(LogosModules* logos, QWidget* parent)
    : QWidget(parent), m_logos(logos) {
    
    // Main layout
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Create scroll area
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(10);
    scrollLayout->setContentsMargins(10, 10, 10, 10);
    
    scrollArea->setWidget(scrollContent);
    
    // Create Keystore frame
    keystoreFrame = new QFrame(scrollContent);
    QVBoxLayout* keystoreLayout = new QVBoxLayout(keystoreFrame);
    keystoreFrame->setLayout(keystoreLayout);
    keystoreFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    keystoreFrame->setLineWidth(1);
    
    QLabel* keystoreTitle = new QLabel("<b>Keystore Operations</b>", scrollContent);
    keystoreLayout->addWidget(keystoreTitle);
    
    // Keystore Init
    QHBoxLayout* keystoreInitLayout = new QHBoxLayout();
    keystoreDirInput = new QLineEdit(scrollContent);
    keystoreDirInput->setPlaceholderText("Keystore Directory...");
    QString keystoreTempDir = QDir::tempPath() + QDir::separator() + "logos_keystore";
    keystoreDirInput->setText(keystoreTempDir);
    keystoreScryptNInput = new QLineEdit(scrollContent);
    keystoreScryptNInput->setPlaceholderText("Scrypt N (default: 262144)");
    keystoreScryptNInput->setText("262144");
    keystoreScryptPInput = new QLineEdit(scrollContent);
    keystoreScryptPInput->setPlaceholderText("Scrypt P (default: 1)");
    keystoreScryptPInput->setText("1");
    keystoreInitButton = new QPushButton("Init Keystore", scrollContent);
    keystoreInitLayout->addWidget(keystoreDirInput);
    keystoreInitLayout->addWidget(keystoreScryptNInput);
    keystoreInitLayout->addWidget(keystoreScryptPInput);
    keystoreInitLayout->addWidget(keystoreInitButton);
    keystoreLayout->addLayout(keystoreInitLayout);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Close
    keystoreCloseButton = new QPushButton("Close Keystore", scrollContent);
    keystoreLayout->addWidget(keystoreCloseButton);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore List Accounts
    QHBoxLayout* keystoreListLayout = new QHBoxLayout();
    keystoreListAccountsButton = new QPushButton("List Accounts", scrollContent);
    keystoreAccountsList = new QListWidget(scrollContent);
    keystoreAccountsList->setMaximumHeight(100);
    keystoreListLayout->addWidget(keystoreListAccountsButton);
    keystoreListLayout->addWidget(keystoreAccountsList, 1);
    keystoreLayout->addLayout(keystoreListLayout);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore New Account
    QHBoxLayout* keystoreNewAccountLayout = new QHBoxLayout();
    keystoreNewAccountPassphraseInput = new QLineEdit(scrollContent);
    keystoreNewAccountPassphraseInput->setPlaceholderText("Passphrase...");
    keystoreNewAccountPassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreNewAccountButton = new QPushButton("New Account", scrollContent);
    keystoreNewAccountLayout->addWidget(keystoreNewAccountPassphraseInput);
    keystoreNewAccountLayout->addWidget(keystoreNewAccountButton);
    keystoreLayout->addLayout(keystoreNewAccountLayout);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Import
    QLabel* keystoreImportLabel = new QLabel("Import Account:", scrollContent);
    keystoreLayout->addWidget(keystoreImportLabel);
    keystoreImportKeyJSONInput = new QTextEdit(scrollContent);
    keystoreImportKeyJSONInput->setPlaceholderText("Key JSON...");
    keystoreImportKeyJSONInput->setMaximumHeight(80);
    keystoreLayout->addWidget(keystoreImportKeyJSONInput);
    QHBoxLayout* keystoreImportLayout = new QHBoxLayout();
    keystoreImportPassphraseInput = new QLineEdit(scrollContent);
    keystoreImportPassphraseInput->setPlaceholderText("Current Passphrase...");
    keystoreImportPassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreImportNewPassphraseInput = new QLineEdit(scrollContent);
    keystoreImportNewPassphraseInput->setPlaceholderText("New Passphrase...");
    keystoreImportNewPassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreImportButton = new QPushButton("Import", scrollContent);
    keystoreImportLayout->addWidget(keystoreImportPassphraseInput);
    keystoreImportLayout->addWidget(keystoreImportNewPassphraseInput);
    keystoreImportLayout->addWidget(keystoreImportButton);
    keystoreLayout->addLayout(keystoreImportLayout);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Export
    QHBoxLayout* keystoreExportLayout = new QHBoxLayout();
    keystoreExportAddressInput = new QLineEdit(scrollContent);
    keystoreExportAddressInput->setPlaceholderText("Address...");
    keystoreExportPassphraseInput = new QLineEdit(scrollContent);
    keystoreExportPassphraseInput->setPlaceholderText("Passphrase...");
    keystoreExportPassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreExportNewPassphraseInput = new QLineEdit(scrollContent);
    keystoreExportNewPassphraseInput->setPlaceholderText("New Passphrase...");
    keystoreExportNewPassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreExportButton = new QPushButton("Export", scrollContent);
    keystoreExportLayout->addWidget(keystoreExportAddressInput);
    keystoreExportLayout->addWidget(keystoreExportPassphraseInput);
    keystoreExportLayout->addWidget(keystoreExportNewPassphraseInput);
    keystoreExportLayout->addWidget(keystoreExportButton);
    keystoreLayout->addLayout(keystoreExportLayout);
    keystoreExportResultLabel = new QLabel("Export Result:", scrollContent);
    keystoreLayout->addWidget(keystoreExportResultLabel);
    keystoreExportResult = new QTextEdit(scrollContent);
    keystoreExportResult->setPlaceholderText("Export result will appear here...");
    keystoreExportResult->setMaximumHeight(80);
    keystoreExportResult->setReadOnly(true);
    keystoreLayout->addWidget(keystoreExportResult);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Delete
    QHBoxLayout* keystoreDeleteLayout = new QHBoxLayout();
    keystoreDeleteAddressInput = new QLineEdit(scrollContent);
    keystoreDeleteAddressInput->setPlaceholderText("Address...");
    keystoreDeletePassphraseInput = new QLineEdit(scrollContent);
    keystoreDeletePassphraseInput->setPlaceholderText("Passphrase...");
    keystoreDeletePassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreDeleteButton = new QPushButton("Delete Account", scrollContent);
    keystoreDeleteLayout->addWidget(keystoreDeleteAddressInput);
    keystoreDeleteLayout->addWidget(keystoreDeletePassphraseInput);
    keystoreDeleteLayout->addWidget(keystoreDeleteButton);
    keystoreLayout->addLayout(keystoreDeleteLayout);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Has Address
    QHBoxLayout* keystoreHasAddressLayout = new QHBoxLayout();
    keystoreHasAddressInput = new QLineEdit(scrollContent);
    keystoreHasAddressInput->setPlaceholderText("Address...");
    keystoreHasAddressButton = new QPushButton("Has Address", scrollContent);
    keystoreHasAddressLayout->addWidget(keystoreHasAddressInput);
    keystoreHasAddressLayout->addWidget(keystoreHasAddressButton);
    keystoreLayout->addLayout(keystoreHasAddressLayout);
    keystoreHasAddressResultLabel = new QLabel("Has Address Result:", scrollContent);
    keystoreLayout->addWidget(keystoreHasAddressResultLabel);
    keystoreHasAddressResult = new QLineEdit(scrollContent);
    keystoreHasAddressResult->setReadOnly(true);
    keystoreHasAddressResult->setPlaceholderText("Result will appear here...");
    keystoreLayout->addWidget(keystoreHasAddressResult);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Unlock/Lock
    QHBoxLayout* keystoreUnlockLayout = new QHBoxLayout();
    keystoreUnlockAddressInput = new QLineEdit(scrollContent);
    keystoreUnlockAddressInput->setPlaceholderText("Address...");
    keystoreUnlockPassphraseInput = new QLineEdit(scrollContent);
    keystoreUnlockPassphraseInput->setPlaceholderText("Passphrase...");
    keystoreUnlockPassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreUnlockButton = new QPushButton("Unlock", scrollContent);
    keystoreUnlockLayout->addWidget(keystoreUnlockAddressInput);
    keystoreUnlockLayout->addWidget(keystoreUnlockPassphraseInput);
    keystoreUnlockLayout->addWidget(keystoreUnlockButton);
    keystoreLayout->addLayout(keystoreUnlockLayout);
    
    QHBoxLayout* keystoreLockLayout = new QHBoxLayout();
    keystoreLockAddressInput = new QLineEdit(scrollContent);
    keystoreLockAddressInput->setPlaceholderText("Address...");
    keystoreLockButton = new QPushButton("Lock", scrollContent);
    keystoreLockLayout->addWidget(keystoreLockAddressInput);
    keystoreLockLayout->addWidget(keystoreLockButton);
    keystoreLayout->addLayout(keystoreLockLayout);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Timed Unlock
    QHBoxLayout* keystoreTimedUnlockLayout = new QHBoxLayout();
    keystoreTimedUnlockAddressInput = new QLineEdit(scrollContent);
    keystoreTimedUnlockAddressInput->setPlaceholderText("Address...");
    keystoreTimedUnlockPassphraseInput = new QLineEdit(scrollContent);
    keystoreTimedUnlockPassphraseInput->setPlaceholderText("Passphrase...");
    keystoreTimedUnlockPassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreTimedUnlockTimeoutInput = new QLineEdit(scrollContent);
    keystoreTimedUnlockTimeoutInput->setPlaceholderText("Timeout (seconds)...");
    keystoreTimedUnlockButton = new QPushButton("Timed Unlock", scrollContent);
    keystoreTimedUnlockLayout->addWidget(keystoreTimedUnlockAddressInput);
    keystoreTimedUnlockLayout->addWidget(keystoreTimedUnlockPassphraseInput);
    keystoreTimedUnlockLayout->addWidget(keystoreTimedUnlockTimeoutInput);
    keystoreTimedUnlockLayout->addWidget(keystoreTimedUnlockButton);
    keystoreLayout->addLayout(keystoreTimedUnlockLayout);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Update
    QHBoxLayout* keystoreUpdateLayout = new QHBoxLayout();
    keystoreUpdateAddressInput = new QLineEdit(scrollContent);
    keystoreUpdateAddressInput->setPlaceholderText("Address...");
    keystoreUpdatePassphraseInput = new QLineEdit(scrollContent);
    keystoreUpdatePassphraseInput->setPlaceholderText("Current Passphrase...");
    keystoreUpdatePassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreUpdateNewPassphraseInput = new QLineEdit(scrollContent);
    keystoreUpdateNewPassphraseInput->setPlaceholderText("New Passphrase...");
    keystoreUpdateNewPassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreUpdateButton = new QPushButton("Update Passphrase", scrollContent);
    keystoreUpdateLayout->addWidget(keystoreUpdateAddressInput);
    keystoreUpdateLayout->addWidget(keystoreUpdatePassphraseInput);
    keystoreUpdateLayout->addWidget(keystoreUpdateNewPassphraseInput);
    keystoreUpdateLayout->addWidget(keystoreUpdateButton);
    keystoreLayout->addLayout(keystoreUpdateLayout);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Sign Hash
    QHBoxLayout* keystoreSignHashLayout = new QHBoxLayout();
    keystoreSignHashAddressInput = new QLineEdit(scrollContent);
    keystoreSignHashAddressInput->setPlaceholderText("Address...");
    keystoreSignHashHashInput = new QLineEdit(scrollContent);
    keystoreSignHashHashInput->setPlaceholderText("Hash (hex)...");
    keystoreSignHashButton = new QPushButton("Sign Hash", scrollContent);
    keystoreSignHashLayout->addWidget(keystoreSignHashAddressInput);
    keystoreSignHashLayout->addWidget(keystoreSignHashHashInput);
    keystoreSignHashLayout->addWidget(keystoreSignHashButton);
    keystoreLayout->addLayout(keystoreSignHashLayout);
    keystoreSignHashResultLabel = new QLabel("Signature Result:", scrollContent);
    keystoreLayout->addWidget(keystoreSignHashResultLabel);
    keystoreSignHashResult = new QLineEdit(scrollContent);
    keystoreSignHashResult->setReadOnly(true);
    keystoreSignHashResult->setPlaceholderText("Signature result will appear here...");
    keystoreLayout->addWidget(keystoreSignHashResult);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Sign Hash With Passphrase
    QHBoxLayout* keystoreSignHashWithPassphraseLayout = new QHBoxLayout();
    keystoreSignHashWithPassphraseAddressInput = new QLineEdit(scrollContent);
    keystoreSignHashWithPassphraseAddressInput->setPlaceholderText("Address...");
    keystoreSignHashWithPassphrasePassphraseInput = new QLineEdit(scrollContent);
    keystoreSignHashWithPassphrasePassphraseInput->setPlaceholderText("Passphrase...");
    keystoreSignHashWithPassphrasePassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreSignHashWithPassphraseHashInput = new QLineEdit(scrollContent);
    keystoreSignHashWithPassphraseHashInput->setPlaceholderText("Hash (hex)...");
    keystoreSignHashWithPassphraseButton = new QPushButton("Sign Hash With Passphrase", scrollContent);
    keystoreSignHashWithPassphraseLayout->addWidget(keystoreSignHashWithPassphraseAddressInput);
    keystoreSignHashWithPassphraseLayout->addWidget(keystoreSignHashWithPassphrasePassphraseInput);
    keystoreSignHashWithPassphraseLayout->addWidget(keystoreSignHashWithPassphraseHashInput);
    keystoreSignHashWithPassphraseLayout->addWidget(keystoreSignHashWithPassphraseButton);
    keystoreLayout->addLayout(keystoreSignHashWithPassphraseLayout);
    keystoreSignHashWithPassphraseResultLabel = new QLabel("Signature Result:", scrollContent);
    keystoreLayout->addWidget(keystoreSignHashWithPassphraseResultLabel);
    keystoreSignHashWithPassphraseResult = new QLineEdit(scrollContent);
    keystoreSignHashWithPassphraseResult->setReadOnly(true);
    keystoreSignHashWithPassphraseResult->setPlaceholderText("Signature result will appear here...");
    keystoreLayout->addWidget(keystoreSignHashWithPassphraseResult);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Import ECDSA
    QHBoxLayout* keystoreImportECDSALayout = new QHBoxLayout();
    keystoreImportECDSAPrivateKeyInput = new QLineEdit(scrollContent);
    keystoreImportECDSAPrivateKeyInput->setPlaceholderText("Private Key (hex)...");
    keystoreImportECDSAPassphraseInput = new QLineEdit(scrollContent);
    keystoreImportECDSAPassphraseInput->setPlaceholderText("Passphrase...");
    keystoreImportECDSAPassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreImportECDSAButton = new QPushButton("Import ECDSA", scrollContent);
    keystoreImportECDSALayout->addWidget(keystoreImportECDSAPrivateKeyInput);
    keystoreImportECDSALayout->addWidget(keystoreImportECDSAPassphraseInput);
    keystoreImportECDSALayout->addWidget(keystoreImportECDSAButton);
    keystoreLayout->addLayout(keystoreImportECDSALayout);
    keystoreImportECDSAResultLabel = new QLabel("Import Result:", scrollContent);
    keystoreLayout->addWidget(keystoreImportECDSAResultLabel);
    keystoreImportECDSAResult = new QLineEdit(scrollContent);
    keystoreImportECDSAResult->setReadOnly(true);
    keystoreImportECDSAResult->setPlaceholderText("Address will appear here...");
    keystoreLayout->addWidget(keystoreImportECDSAResult);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Sign Tx
    QLabel* keystoreSignTxLabel = new QLabel("Sign Transaction:", scrollContent);
    keystoreLayout->addWidget(keystoreSignTxLabel);
    keystoreSignTxAddressInput = new QLineEdit(scrollContent);
    keystoreSignTxAddressInput->setPlaceholderText("Address...");
    keystoreSignTxTxJSONInput = new QTextEdit(scrollContent);
    keystoreSignTxTxJSONInput->setPlaceholderText("Transaction JSON...");
    keystoreSignTxTxJSONInput->setMaximumHeight(80);
    keystoreLayout->addWidget(keystoreSignTxAddressInput);
    keystoreLayout->addWidget(keystoreSignTxTxJSONInput);
    QHBoxLayout* keystoreSignTxLayout = new QHBoxLayout();
    keystoreSignTxChainIDInput = new QLineEdit(scrollContent);
    keystoreSignTxChainIDInput->setPlaceholderText("Chain ID (hex)...");
    keystoreSignTxButton = new QPushButton("Sign Transaction", scrollContent);
    keystoreSignTxLayout->addWidget(keystoreSignTxChainIDInput);
    keystoreSignTxLayout->addWidget(keystoreSignTxButton);
    keystoreLayout->addLayout(keystoreSignTxLayout);
    keystoreSignTxResultLabel = new QLabel("Signed Transaction Result:", scrollContent);
    keystoreLayout->addWidget(keystoreSignTxResultLabel);
    keystoreSignTxResult = new QTextEdit(scrollContent);
    keystoreSignTxResult->setReadOnly(true);
    keystoreSignTxResult->setPlaceholderText("Signed transaction will appear here...");
    keystoreSignTxResult->setMaximumHeight(80);
    keystoreLayout->addWidget(keystoreSignTxResult);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Sign Tx With Passphrase
    QLabel* keystoreSignTxWithPassphraseLabel = new QLabel("Sign Transaction With Passphrase:", scrollContent);
    keystoreLayout->addWidget(keystoreSignTxWithPassphraseLabel);
    keystoreSignTxWithPassphraseAddressInput = new QLineEdit(scrollContent);
    keystoreSignTxWithPassphraseAddressInput->setPlaceholderText("Address...");
    keystoreSignTxWithPassphrasePassphraseInput = new QLineEdit(scrollContent);
    keystoreSignTxWithPassphrasePassphraseInput->setPlaceholderText("Passphrase...");
    keystoreSignTxWithPassphrasePassphraseInput->setEchoMode(QLineEdit::Password);
    keystoreSignTxWithPassphraseTxJSONInput = new QTextEdit(scrollContent);
    keystoreSignTxWithPassphraseTxJSONInput->setPlaceholderText("Transaction JSON...");
    keystoreSignTxWithPassphraseTxJSONInput->setMaximumHeight(80);
    keystoreLayout->addWidget(keystoreSignTxWithPassphraseAddressInput);
    keystoreLayout->addWidget(keystoreSignTxWithPassphrasePassphraseInput);
    keystoreLayout->addWidget(keystoreSignTxWithPassphraseTxJSONInput);
    QHBoxLayout* keystoreSignTxWithPassphraseLayout = new QHBoxLayout();
    keystoreSignTxWithPassphraseChainIDInput = new QLineEdit(scrollContent);
    keystoreSignTxWithPassphraseChainIDInput->setPlaceholderText("Chain ID (hex)...");
    keystoreSignTxWithPassphraseButton = new QPushButton("Sign Transaction With Passphrase", scrollContent);
    keystoreSignTxWithPassphraseLayout->addWidget(keystoreSignTxWithPassphraseChainIDInput);
    keystoreSignTxWithPassphraseLayout->addWidget(keystoreSignTxWithPassphraseButton);
    keystoreLayout->addLayout(keystoreSignTxWithPassphraseLayout);
    keystoreSignTxWithPassphraseResultLabel = new QLabel("Signed Transaction Result:", scrollContent);
    keystoreLayout->addWidget(keystoreSignTxWithPassphraseResultLabel);
    keystoreSignTxWithPassphraseResult = new QTextEdit(scrollContent);
    keystoreSignTxWithPassphraseResult->setReadOnly(true);
    keystoreSignTxWithPassphraseResult->setPlaceholderText("Signed transaction will appear here...");
    keystoreSignTxWithPassphraseResult->setMaximumHeight(80);
    keystoreLayout->addWidget(keystoreSignTxWithPassphraseResult);
    keystoreLayout->addWidget(createSeparator());
    
    // Keystore Find
    QHBoxLayout* keystoreFindLayout = new QHBoxLayout();
    keystoreFindAddressInput = new QLineEdit(scrollContent);
    keystoreFindAddressInput->setPlaceholderText("Address...");
    keystoreFindUrlInput = new QLineEdit(scrollContent);
    keystoreFindUrlInput->setPlaceholderText("URL...");
    keystoreFindButton = new QPushButton("Find", scrollContent);
    keystoreFindLayout->addWidget(keystoreFindAddressInput);
    keystoreFindLayout->addWidget(keystoreFindUrlInput);
    keystoreFindLayout->addWidget(keystoreFindButton);
    keystoreLayout->addLayout(keystoreFindLayout);
    keystoreFindResultLabel = new QLabel("Find Result:", scrollContent);
    keystoreLayout->addWidget(keystoreFindResultLabel);
    keystoreFindResult = new QLineEdit(scrollContent);
    keystoreFindResult->setReadOnly(true);
    keystoreFindResult->setPlaceholderText("Result will appear here...");
    keystoreLayout->addWidget(keystoreFindResult);
    
    // Add frame to scroll layout
    scrollLayout->addWidget(keystoreFrame);
    scrollLayout->addStretch();
    
    // Add scroll area to main layout
    mainLayout->addWidget(scrollArea);
    
    // Connect signals
    connect(keystoreInitButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreInitButtonClicked);
    connect(keystoreCloseButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreCloseButtonClicked);
    connect(keystoreListAccountsButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreListAccountsButtonClicked);
    connect(keystoreNewAccountButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreNewAccountButtonClicked);
    connect(keystoreImportButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreImportButtonClicked);
    connect(keystoreExportButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreExportButtonClicked);
    connect(keystoreDeleteButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreDeleteButtonClicked);
    connect(keystoreHasAddressButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreHasAddressButtonClicked);
    connect(keystoreUnlockButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreUnlockButtonClicked);
    connect(keystoreLockButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreLockButtonClicked);
    connect(keystoreTimedUnlockButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreTimedUnlockButtonClicked);
    connect(keystoreUpdateButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreUpdateButtonClicked);
    connect(keystoreSignHashButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreSignHashButtonClicked);
    connect(keystoreSignHashWithPassphraseButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreSignHashWithPassphraseButtonClicked);
    connect(keystoreImportECDSAButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreImportECDSAButtonClicked);
    connect(keystoreSignTxButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreSignTxButtonClicked);
    connect(keystoreSignTxWithPassphraseButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreSignTxWithPassphraseButtonClicked);
    connect(keystoreFindButton, &QPushButton::clicked, this, &KeystoreWidget::onKeystoreFindButtonClicked);
}

KeystoreWidget::~KeystoreWidget() {
}

QFrame* KeystoreWidget::createSeparator() {
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    return separator;
}

// Keystore operations
void KeystoreWidget::onKeystoreInitButtonClicked() {
    QString dir = keystoreDirInput->text();
    int scryptN = keystoreScryptNInput->text().toInt();
    int scryptP = keystoreScryptPInput->text().toInt();
    
    if (dir.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a keystore directory");
        return;
    }
    
    emit statusUpdate("Initializing keystore...");
    bool success = m_logos->accounts_module.initKeystore(dir, scryptN, scryptP);
    
    if (success) {
        emit statusUpdate("Keystore initialized successfully");
        QMessageBox::information(this, "Success", "Keystore initialized successfully");
        onKeystoreListAccountsButtonClicked(); // Refresh list
    } else {
        emit statusUpdate("Failed to initialize keystore");
        QMessageBox::warning(this, "Error", "Failed to initialize keystore");
    }
}

void KeystoreWidget::onKeystoreCloseButtonClicked() {
    emit statusUpdate("Closing keystore...");
    bool success = m_logos->accounts_module.closeKeystore("");
    
    if (success) {
        emit statusUpdate("Keystore closed successfully");
        QMessageBox::information(this, "Success", "Keystore closed successfully");
    } else {
        emit statusUpdate("Failed to close keystore");
        QMessageBox::warning(this, "Error", "Failed to close keystore");
    }
}

void KeystoreWidget::onKeystoreListAccountsButtonClicked() {
    emit statusUpdate("Listing accounts...");
    QStringList accounts = m_logos->accounts_module.keystoreAccounts();
    
    keystoreAccountsList->clear();
    for (const QString& account : accounts) {
        keystoreAccountsList->addItem(account);
    }
    
    emit statusUpdate(QString("Found %1 accounts").arg(accounts.size()));
}

void KeystoreWidget::onKeystoreNewAccountButtonClicked() {
    QString passphrase = keystoreNewAccountPassphraseInput->text();
    
    if (passphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a passphrase");
        return;
    }
    
    emit statusUpdate("Creating new account...");
    QString address = m_logos->accounts_module.keystoreNewAccount(passphrase);
    
    if (!address.isEmpty()) {
        emit statusUpdate(QString("New account created: %1").arg(address));
        QMessageBox::information(this, "Success", QString("New account created:\n%1").arg(address));
        keystoreNewAccountPassphraseInput->clear();
        onKeystoreListAccountsButtonClicked(); // Refresh list
    } else {
        emit statusUpdate("Failed to create new account");
        QMessageBox::warning(this, "Error", "Failed to create new account");
    }
}

void KeystoreWidget::onKeystoreImportButtonClicked() {
    QString keyJSON = keystoreImportKeyJSONInput->toPlainText();
    QString passphrase = keystoreImportPassphraseInput->text();
    QString newPassphrase = keystoreImportNewPassphraseInput->text();
    
    if (keyJSON.isEmpty() || passphrase.isEmpty() || newPassphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Importing account...");
    QString address = m_logos->accounts_module.keystoreImport(keyJSON, passphrase, newPassphrase);
    
    if (!address.isEmpty()) {
        emit statusUpdate(QString("Account imported: %1").arg(address));
        QMessageBox::information(this, "Success", QString("Account imported:\n%1").arg(address));
        keystoreImportKeyJSONInput->clear();
        keystoreImportPassphraseInput->clear();
        keystoreImportNewPassphraseInput->clear();
        onKeystoreListAccountsButtonClicked(); // Refresh list
    } else {
        emit statusUpdate("Failed to import account");
        QMessageBox::warning(this, "Error", "Failed to import account");
    }
}

void KeystoreWidget::onKeystoreExportButtonClicked() {
    QString address = keystoreExportAddressInput->text();
    QString passphrase = keystoreExportPassphraseInput->text();
    QString newPassphrase = keystoreExportNewPassphraseInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty() || newPassphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Exporting account...");
    QString keyJSON = m_logos->accounts_module.keystoreExport(address, passphrase, newPassphrase);
    
    if (!keyJSON.isEmpty()) {
        keystoreExportResult->setPlainText(keyJSON);
        emit statusUpdate(QString("Account exported: %1").arg(address));
    } else {
        keystoreExportResult->setPlainText("Export failed");
        emit statusUpdate("Failed to export account");
        QMessageBox::warning(this, "Error", "Failed to export account");
    }
}

void KeystoreWidget::onKeystoreDeleteButtonClicked() {
    QString address = keystoreDeleteAddressInput->text();
    QString passphrase = keystoreDeletePassphraseInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete", 
        QString("Are you sure you want to delete account %1?").arg(address),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        emit statusUpdate("Deleting account...");
        bool success = m_logos->accounts_module.keystoreDelete(address, passphrase);
        
        if (success) {
            emit statusUpdate(QString("Account deleted: %1").arg(address));
            QMessageBox::information(this, "Success", "Account deleted successfully");
            keystoreDeleteAddressInput->clear();
            keystoreDeletePassphraseInput->clear();
            onKeystoreListAccountsButtonClicked(); // Refresh list
        } else {
            emit statusUpdate("Failed to delete account");
            QMessageBox::warning(this, "Error", "Failed to delete account");
        }
    }
}

void KeystoreWidget::onKeystoreHasAddressButtonClicked() {
    QString address = keystoreHasAddressInput->text();
    
    if (address.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter an address");
        return;
    }
    
    emit statusUpdate("Checking if address exists...");
    bool hasAddress = m_logos->accounts_module.keystoreHasAddress(address);
    
    keystoreHasAddressResult->setText(hasAddress ? "true" : "false");
    emit statusUpdate(hasAddress ? "Address exists" : "Address does not exist");
}

void KeystoreWidget::onKeystoreUnlockButtonClicked() {
    QString address = keystoreUnlockAddressInput->text();
    QString passphrase = keystoreUnlockPassphraseInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Unlocking account...");
    bool success = m_logos->accounts_module.keystoreUnlock(address, passphrase);
    
    if (success) {
        emit statusUpdate(QString("Account unlocked: %1").arg(address));
        QMessageBox::information(this, "Success", "Account unlocked successfully");
        keystoreUnlockPassphraseInput->clear();
    } else {
        emit statusUpdate("Failed to unlock account");
        QMessageBox::warning(this, "Error", "Failed to unlock account");
    }
}

void KeystoreWidget::onKeystoreLockButtonClicked() {
    QString address = keystoreLockAddressInput->text();
    
    if (address.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter an address");
        return;
    }
    
    emit statusUpdate("Locking account...");
    bool success = m_logos->accounts_module.keystoreLock(address);
    
    if (success) {
        emit statusUpdate(QString("Account locked: %1").arg(address));
        QMessageBox::information(this, "Success", "Account locked successfully");
        keystoreLockAddressInput->clear();
    } else {
        emit statusUpdate("Failed to lock account");
        QMessageBox::warning(this, "Error", "Failed to lock account");
    }
}

void KeystoreWidget::onKeystoreTimedUnlockButtonClicked() {
    QString address = keystoreTimedUnlockAddressInput->text();
    QString passphrase = keystoreTimedUnlockPassphraseInput->text();
    QString timeoutStr = keystoreTimedUnlockTimeoutInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty() || timeoutStr.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    uint timeout = timeoutStr.toUInt();
    emit statusUpdate("Unlocking account with timeout...");
    bool success = m_logos->accounts_module.keystoreTimedUnlock(address, passphrase, timeout);
    
    if (success) {
        emit statusUpdate(QString("Account unlocked with timeout: %1").arg(address));
        QMessageBox::information(this, "Success", "Account unlocked successfully");
        keystoreTimedUnlockPassphraseInput->clear();
    } else {
        emit statusUpdate("Failed to unlock account");
        QMessageBox::warning(this, "Error", "Failed to unlock account");
    }
}

void KeystoreWidget::onKeystoreUpdateButtonClicked() {
    QString address = keystoreUpdateAddressInput->text();
    QString passphrase = keystoreUpdatePassphraseInput->text();
    QString newPassphrase = keystoreUpdateNewPassphraseInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty() || newPassphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Updating passphrase...");
    bool success = m_logos->accounts_module.keystoreUpdate(address, passphrase, newPassphrase);
    
    if (success) {
        emit statusUpdate(QString("Passphrase updated: %1").arg(address));
        QMessageBox::information(this, "Success", "Passphrase updated successfully");
        keystoreUpdatePassphraseInput->clear();
        keystoreUpdateNewPassphraseInput->clear();
    } else {
        emit statusUpdate("Failed to update passphrase");
        QMessageBox::warning(this, "Error", "Failed to update passphrase");
    }
}

void KeystoreWidget::onKeystoreSignHashButtonClicked() {
    QString address = keystoreSignHashAddressInput->text();
    QString hashHex = keystoreSignHashHashInput->text();
    
    if (address.isEmpty() || hashHex.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Signing hash...");
    QString signature = m_logos->accounts_module.keystoreSignHash(address, hashHex);
    
    if (!signature.isEmpty()) {
        keystoreSignHashResult->setText(signature);
        emit statusUpdate("Hash signed successfully");
    } else {
        keystoreSignHashResult->setText("Signing failed");
        emit statusUpdate("Failed to sign hash");
        QMessageBox::warning(this, "Error", "Failed to sign hash");
    }
}

void KeystoreWidget::onKeystoreSignHashWithPassphraseButtonClicked() {
    QString address = keystoreSignHashWithPassphraseAddressInput->text();
    QString passphrase = keystoreSignHashWithPassphrasePassphraseInput->text();
    QString hashHex = keystoreSignHashWithPassphraseHashInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty() || hashHex.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Signing hash with passphrase...");
    QString signature = m_logos->accounts_module.keystoreSignHashWithPassphrase(address, passphrase, hashHex);
    
    if (!signature.isEmpty()) {
        keystoreSignHashWithPassphraseResult->setText(signature);
        emit statusUpdate("Hash signed successfully");
    } else {
        keystoreSignHashWithPassphraseResult->setText("Signing failed");
        emit statusUpdate("Failed to sign hash");
        QMessageBox::warning(this, "Error", "Failed to sign hash");
    }
}

void KeystoreWidget::onKeystoreImportECDSAButtonClicked() {
    QString privateKeyHex = keystoreImportECDSAPrivateKeyInput->text();
    QString passphrase = keystoreImportECDSAPassphraseInput->text();
    
    if (privateKeyHex.isEmpty() || passphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Importing ECDSA private key...");
    QString address = m_logos->accounts_module.keystoreImportECDSA(privateKeyHex, passphrase);
    
    if (!address.isEmpty()) {
        keystoreImportECDSAResult->setText(address);
        emit statusUpdate(QString("ECDSA key imported: %1").arg(address));
        QMessageBox::information(this, "Success", QString("ECDSA key imported:\n%1").arg(address));
        keystoreImportECDSAPrivateKeyInput->clear();
        keystoreImportECDSAPassphraseInput->clear();
        onKeystoreListAccountsButtonClicked();
    } else {
        keystoreImportECDSAResult->setText("Import failed");
        emit statusUpdate("Failed to import ECDSA key");
        QMessageBox::warning(this, "Error", "Failed to import ECDSA key");
    }
}

void KeystoreWidget::onKeystoreSignTxButtonClicked() {
    QString address = keystoreSignTxAddressInput->text();
    QString txJSON = keystoreSignTxTxJSONInput->toPlainText();
    QString chainIDHex = keystoreSignTxChainIDInput->text();
    
    if (address.isEmpty() || txJSON.isEmpty() || chainIDHex.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Signing transaction...");
    QString signedTx = m_logos->accounts_module.keystoreSignTx(address, txJSON, chainIDHex);
    
    if (!signedTx.isEmpty()) {
        keystoreSignTxResult->setPlainText(signedTx);
        emit statusUpdate("Transaction signed successfully");
    } else {
        keystoreSignTxResult->setPlainText("Signing failed");
        emit statusUpdate("Failed to sign transaction");
        QMessageBox::warning(this, "Error", "Failed to sign transaction");
    }
}

void KeystoreWidget::onKeystoreSignTxWithPassphraseButtonClicked() {
    QString address = keystoreSignTxWithPassphraseAddressInput->text();
    QString passphrase = keystoreSignTxWithPassphrasePassphraseInput->text();
    QString txJSON = keystoreSignTxWithPassphraseTxJSONInput->toPlainText();
    QString chainIDHex = keystoreSignTxWithPassphraseChainIDInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty() || txJSON.isEmpty() || chainIDHex.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Signing transaction with passphrase...");
    QString signedTx = m_logos->accounts_module.keystoreSignTxWithPassphrase(address, passphrase, txJSON, chainIDHex);
    
    if (!signedTx.isEmpty()) {
        keystoreSignTxWithPassphraseResult->setPlainText(signedTx);
        emit statusUpdate("Transaction signed successfully");
    } else {
        keystoreSignTxWithPassphraseResult->setPlainText("Signing failed");
        emit statusUpdate("Failed to sign transaction");
        QMessageBox::warning(this, "Error", "Failed to sign transaction");
    }
}

void KeystoreWidget::onKeystoreFindButtonClicked() {
    QString address = keystoreFindAddressInput->text();
    QString url = keystoreFindUrlInput->text();
    
    if (address.isEmpty() || url.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Finding account...");
    QString result = m_logos->accounts_module.keystoreFind(address, url);
    
    if (!result.isEmpty()) {
        keystoreFindResult->setText(result);
        emit statusUpdate("Account found");
    } else {
        keystoreFindResult->setText("Not found");
        emit statusUpdate("Account not found");
    }
}

