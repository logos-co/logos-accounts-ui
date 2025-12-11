#include "ExtKeystoreWidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QDir>

ExtKeystoreWidget::ExtKeystoreWidget(LogosModules* logos, QWidget* parent)
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
    extKeystoreFrame = new QFrame(scrollContent);
    QVBoxLayout* extKeystoreLayout = new QVBoxLayout(extKeystoreFrame);
    extKeystoreFrame->setLayout(extKeystoreLayout);
    extKeystoreFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    extKeystoreFrame->setLineWidth(1);
    
    QLabel* extKeystoreTitle = new QLabel("<b>Keystore Operations</b>", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreTitle);
    
    // Keystore Init
    QHBoxLayout* keystoreInitLayout = new QHBoxLayout();
    extKeystoreDirInput = new QLineEdit(scrollContent);
    extKeystoreDirInput->setPlaceholderText("Keystore Directory...");
    QString extKeystoreTempDir = QDir::tempPath() + QDir::separator() + "logos_keystore";
    extKeystoreDirInput->setText(extKeystoreTempDir);
    extKeystoreScryptNInput = new QLineEdit(scrollContent);
    extKeystoreScryptNInput->setPlaceholderText("Scrypt N (default: 262144)");
    extKeystoreScryptNInput->setText("262144");
    extKeystoreScryptPInput = new QLineEdit(scrollContent);
    extKeystoreScryptPInput->setPlaceholderText("Scrypt P (default: 1)");
    extKeystoreScryptPInput->setText("1");
    extKeystoreInitButton = new QPushButton("Init Extended Keystore", scrollContent);
    keystoreInitLayout->addWidget(extKeystoreDirInput);
    keystoreInitLayout->addWidget(extKeystoreScryptNInput);
    keystoreInitLayout->addWidget(extKeystoreScryptPInput);
    keystoreInitLayout->addWidget(extKeystoreInitButton);
    extKeystoreLayout->addLayout(keystoreInitLayout);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore Close
    extKeystoreCloseButton = new QPushButton("Close Extended Keystore", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreCloseButton);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore List Accounts
    QHBoxLayout* keystoreListLayout = new QHBoxLayout();
    extKeystoreListAccountsButton = new QPushButton("List Accounts", scrollContent);
    extKeystoreAccountsList = new QListWidget(scrollContent);
    extKeystoreAccountsList->setMaximumHeight(100);
    keystoreListLayout->addWidget(extKeystoreListAccountsButton);
    keystoreListLayout->addWidget(extKeystoreAccountsList, 1);
    extKeystoreLayout->addLayout(keystoreListLayout);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore New Account
    QHBoxLayout* extKeystoreNewAccountLayout = new QHBoxLayout();
    extKeystoreNewAccountPassphraseInput = new QLineEdit(scrollContent);
    extKeystoreNewAccountPassphraseInput->setPlaceholderText("Passphrase...");
    extKeystoreNewAccountPassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreNewAccountButton = new QPushButton("New Account", scrollContent);
    extKeystoreNewAccountLayout->addWidget(extKeystoreNewAccountPassphraseInput);
    extKeystoreNewAccountLayout->addWidget(extKeystoreNewAccountButton);
    extKeystoreLayout->addLayout(extKeystoreNewAccountLayout);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore Import
    QLabel* extKeystoreImportLabel = new QLabel("Import Account:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreImportLabel);
    extKeystoreImportKeyJSONInput = new QTextEdit(scrollContent);
    extKeystoreImportKeyJSONInput->setPlaceholderText("Key JSON...");
    extKeystoreImportKeyJSONInput->setMaximumHeight(80);
    extKeystoreLayout->addWidget(extKeystoreImportKeyJSONInput);
    QHBoxLayout* extKeystoreImportLayout = new QHBoxLayout();
    extKeystoreImportPassphraseInput = new QLineEdit(scrollContent);
    extKeystoreImportPassphraseInput->setPlaceholderText("Current Passphrase...");
    extKeystoreImportPassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreImportNewPassphraseInput = new QLineEdit(scrollContent);
    extKeystoreImportNewPassphraseInput->setPlaceholderText("New Passphrase...");
    extKeystoreImportNewPassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreImportButton = new QPushButton("Import", scrollContent);
    extKeystoreImportLayout->addWidget(extKeystoreImportPassphraseInput);
    extKeystoreImportLayout->addWidget(extKeystoreImportNewPassphraseInput);
    extKeystoreImportLayout->addWidget(extKeystoreImportButton);
    extKeystoreLayout->addLayout(extKeystoreImportLayout);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore Export
    QHBoxLayout* extKeystoreExportExtLayout = new QHBoxLayout();
    extKeystoreExportExtAddressInput = new QLineEdit(scrollContent);
    extKeystoreExportExtAddressInput->setPlaceholderText("Address...");
    extKeystoreExportExtPassphraseInput = new QLineEdit(scrollContent);
    extKeystoreExportExtPassphraseInput->setPlaceholderText("Passphrase...");
    extKeystoreExportExtPassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreExportExtNewPassphraseInput = new QLineEdit(scrollContent);
    extKeystoreExportExtNewPassphraseInput->setPlaceholderText("New Passphrase...");
    extKeystoreExportExtNewPassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreExportExtButton = new QPushButton("Export", scrollContent);
    extKeystoreExportExtLayout->addWidget(extKeystoreExportExtAddressInput);
    extKeystoreExportExtLayout->addWidget(extKeystoreExportExtPassphraseInput);
    extKeystoreExportExtLayout->addWidget(extKeystoreExportExtNewPassphraseInput);
    extKeystoreExportExtLayout->addWidget(extKeystoreExportExtButton);
    extKeystoreLayout->addLayout(extKeystoreExportExtLayout);
    extKeystoreExportExtResultLabel = new QLabel("Export Result:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreExportExtResultLabel);
    extKeystoreExportExtResult = new QTextEdit(scrollContent);
    extKeystoreExportExtResult->setPlaceholderText("Export result will appear here...");
    extKeystoreExportExtResult->setMaximumHeight(80);
    extKeystoreExportExtResult->setReadOnly(true);
    extKeystoreLayout->addWidget(extKeystoreExportExtResult);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Extended Keystore Export Priv
    QHBoxLayout* extKeystoreExportPrivLayout = new QHBoxLayout();
    extKeystoreExportPrivAddressInput = new QLineEdit(scrollContent);
    extKeystoreExportPrivAddressInput->setPlaceholderText("Address...");
    extKeystoreExportPrivPassphraseInput = new QLineEdit(scrollContent);
    extKeystoreExportPrivPassphraseInput->setPlaceholderText("Passphrase...");
    extKeystoreExportPrivPassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreExportPrivNewPassphraseInput = new QLineEdit(scrollContent);
    extKeystoreExportPrivNewPassphraseInput->setPlaceholderText("New Passphrase...");
    extKeystoreExportPrivNewPassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreExportPrivButton = new QPushButton("Export Private Key", scrollContent);
    extKeystoreExportPrivLayout->addWidget(extKeystoreExportPrivAddressInput);
    extKeystoreExportPrivLayout->addWidget(extKeystoreExportPrivPassphraseInput);
    extKeystoreExportPrivLayout->addWidget(extKeystoreExportPrivNewPassphraseInput);
    extKeystoreExportPrivLayout->addWidget(extKeystoreExportPrivButton);
    extKeystoreLayout->addLayout(extKeystoreExportPrivLayout);
    extKeystoreExportPrivResultLabel = new QLabel("Export Result:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreExportPrivResultLabel);
    extKeystoreExportPrivResult = new QTextEdit(scrollContent);
    extKeystoreExportPrivResult->setReadOnly(true);
    extKeystoreExportPrivResult->setPlaceholderText("Export result will appear here...");
    extKeystoreExportPrivResult->setMaximumHeight(80);
    extKeystoreLayout->addWidget(extKeystoreExportPrivResult);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Extended Keystore Delete
    QHBoxLayout* extKeystoreDeleteLayout = new QHBoxLayout();
    extKeystoreDeleteAddressInput = new QLineEdit(scrollContent);
    extKeystoreDeleteAddressInput->setPlaceholderText("Address...");
    extKeystoreDeletePassphraseInput = new QLineEdit(scrollContent);
    extKeystoreDeletePassphraseInput->setPlaceholderText("Passphrase...");
    extKeystoreDeletePassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreDeleteButton = new QPushButton("Delete Account", scrollContent);
    extKeystoreDeleteLayout->addWidget(extKeystoreDeleteAddressInput);
    extKeystoreDeleteLayout->addWidget(extKeystoreDeletePassphraseInput);
    extKeystoreDeleteLayout->addWidget(extKeystoreDeleteButton);
    extKeystoreLayout->addLayout(extKeystoreDeleteLayout);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore Has Address
    QHBoxLayout* extKeystoreHasAddressLayout = new QHBoxLayout();
    extKeystoreHasAddressInput = new QLineEdit(scrollContent);
    extKeystoreHasAddressInput->setPlaceholderText("Address...");
    extKeystoreHasAddressButton = new QPushButton("Has Address", scrollContent);
    extKeystoreHasAddressLayout->addWidget(extKeystoreHasAddressInput);
    extKeystoreHasAddressLayout->addWidget(extKeystoreHasAddressButton);
    extKeystoreLayout->addLayout(extKeystoreHasAddressLayout);
    extKeystoreHasAddressResultLabel = new QLabel("Has Address Result:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreHasAddressResultLabel);
    extKeystoreHasAddressResult = new QLineEdit(scrollContent);
    extKeystoreHasAddressResult->setReadOnly(true);
    extKeystoreHasAddressResult->setPlaceholderText("Result will appear here...");
    extKeystoreLayout->addWidget(extKeystoreHasAddressResult);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore Unlock/Lock
    QHBoxLayout* extKeystoreUnlockLayout = new QHBoxLayout();
    extKeystoreUnlockAddressInput = new QLineEdit(scrollContent);
    extKeystoreUnlockAddressInput->setPlaceholderText("Address...");
    extKeystoreUnlockPassphraseInput = new QLineEdit(scrollContent);
    extKeystoreUnlockPassphraseInput->setPlaceholderText("Passphrase...");
    extKeystoreUnlockPassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreUnlockButton = new QPushButton("Unlock", scrollContent);
    extKeystoreUnlockLayout->addWidget(extKeystoreUnlockAddressInput);
    extKeystoreUnlockLayout->addWidget(extKeystoreUnlockPassphraseInput);
    extKeystoreUnlockLayout->addWidget(extKeystoreUnlockButton);
    extKeystoreLayout->addLayout(extKeystoreUnlockLayout);
    
    QHBoxLayout* extKeystoreLockLayout = new QHBoxLayout();
    extKeystoreLockAddressInput = new QLineEdit(scrollContent);
    extKeystoreLockAddressInput->setPlaceholderText("Address...");
    extKeystoreLockButton = new QPushButton("Lock", scrollContent);
    extKeystoreLockLayout->addWidget(extKeystoreLockAddressInput);
    extKeystoreLockLayout->addWidget(extKeystoreLockButton);
    extKeystoreLayout->addLayout(extKeystoreLockLayout);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore Timed Unlock
    QHBoxLayout* extKeystoreTimedUnlockLayout = new QHBoxLayout();
    extKeystoreTimedUnlockAddressInput = new QLineEdit(scrollContent);
    extKeystoreTimedUnlockAddressInput->setPlaceholderText("Address...");
    extKeystoreTimedUnlockPassphraseInput = new QLineEdit(scrollContent);
    extKeystoreTimedUnlockPassphraseInput->setPlaceholderText("Passphrase...");
    extKeystoreTimedUnlockPassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreTimedUnlockTimeoutInput = new QLineEdit(scrollContent);
    extKeystoreTimedUnlockTimeoutInput->setPlaceholderText("Timeout (seconds)...");
    extKeystoreTimedUnlockButton = new QPushButton("Timed Unlock", scrollContent);
    extKeystoreTimedUnlockLayout->addWidget(extKeystoreTimedUnlockAddressInput);
    extKeystoreTimedUnlockLayout->addWidget(extKeystoreTimedUnlockPassphraseInput);
    extKeystoreTimedUnlockLayout->addWidget(extKeystoreTimedUnlockTimeoutInput);
    extKeystoreTimedUnlockLayout->addWidget(extKeystoreTimedUnlockButton);
    extKeystoreLayout->addLayout(extKeystoreTimedUnlockLayout);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore Update
    QHBoxLayout* extKeystoreUpdateLayout = new QHBoxLayout();
    extKeystoreUpdateAddressInput = new QLineEdit(scrollContent);
    extKeystoreUpdateAddressInput->setPlaceholderText("Address...");
    extKeystoreUpdatePassphraseInput = new QLineEdit(scrollContent);
    extKeystoreUpdatePassphraseInput->setPlaceholderText("Current Passphrase...");
    extKeystoreUpdatePassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreUpdateNewPassphraseInput = new QLineEdit(scrollContent);
    extKeystoreUpdateNewPassphraseInput->setPlaceholderText("New Passphrase...");
    extKeystoreUpdateNewPassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreUpdateButton = new QPushButton("Update Passphrase", scrollContent);
    extKeystoreUpdateLayout->addWidget(extKeystoreUpdateAddressInput);
    extKeystoreUpdateLayout->addWidget(extKeystoreUpdatePassphraseInput);
    extKeystoreUpdateLayout->addWidget(extKeystoreUpdateNewPassphraseInput);
    extKeystoreUpdateLayout->addWidget(extKeystoreUpdateButton);
    extKeystoreLayout->addLayout(extKeystoreUpdateLayout);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore Sign Hash
    QHBoxLayout* extKeystoreSignHashLayout = new QHBoxLayout();
    extKeystoreSignHashAddressInput = new QLineEdit(scrollContent);
    extKeystoreSignHashAddressInput->setPlaceholderText("Address...");
    extKeystoreSignHashHashInput = new QLineEdit(scrollContent);
    extKeystoreSignHashHashInput->setPlaceholderText("Hash (hex)...");
    extKeystoreSignHashButton = new QPushButton("Sign Hash", scrollContent);
    extKeystoreSignHashLayout->addWidget(extKeystoreSignHashAddressInput);
    extKeystoreSignHashLayout->addWidget(extKeystoreSignHashHashInput);
    extKeystoreSignHashLayout->addWidget(extKeystoreSignHashButton);
    extKeystoreLayout->addLayout(extKeystoreSignHashLayout);
    extKeystoreSignHashResultLabel = new QLabel("Signature Result:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreSignHashResultLabel);
    extKeystoreSignHashResult = new QLineEdit(scrollContent);
    extKeystoreSignHashResult->setReadOnly(true);
    extKeystoreSignHashResult->setPlaceholderText("Signature result will appear here...");
    extKeystoreLayout->addWidget(extKeystoreSignHashResult);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore Sign Hash With Passphrase
    QHBoxLayout* extKeystoreSignHashWithPassphraseLayout = new QHBoxLayout();
    extKeystoreSignHashWithPassphraseAddressInput = new QLineEdit(scrollContent);
    extKeystoreSignHashWithPassphraseAddressInput->setPlaceholderText("Address...");
    extKeystoreSignHashWithPassphrasePassphraseInput = new QLineEdit(scrollContent);
    extKeystoreSignHashWithPassphrasePassphraseInput->setPlaceholderText("Passphrase...");
    extKeystoreSignHashWithPassphrasePassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreSignHashWithPassphraseHashInput = new QLineEdit(scrollContent);
    extKeystoreSignHashWithPassphraseHashInput->setPlaceholderText("Hash (hex)...");
    extKeystoreSignHashWithPassphraseButton = new QPushButton("Sign Hash With Passphrase", scrollContent);
    extKeystoreSignHashWithPassphraseLayout->addWidget(extKeystoreSignHashWithPassphraseAddressInput);
    extKeystoreSignHashWithPassphraseLayout->addWidget(extKeystoreSignHashWithPassphrasePassphraseInput);
    extKeystoreSignHashWithPassphraseLayout->addWidget(extKeystoreSignHashWithPassphraseHashInput);
    extKeystoreSignHashWithPassphraseLayout->addWidget(extKeystoreSignHashWithPassphraseButton);
    extKeystoreLayout->addLayout(extKeystoreSignHashWithPassphraseLayout);
    extKeystoreSignHashWithPassphraseResultLabel = new QLabel("Signature Result:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreSignHashWithPassphraseResultLabel);
    extKeystoreSignHashWithPassphraseResult = new QLineEdit(scrollContent);
    extKeystoreSignHashWithPassphraseResult->setReadOnly(true);
    extKeystoreSignHashWithPassphraseResult->setPlaceholderText("Signature result will appear here...");
    extKeystoreLayout->addWidget(extKeystoreSignHashWithPassphraseResult);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore Import ECDSA
    QHBoxLayout* extKeystoreImportECDSALayout = new QHBoxLayout();
    extKeystoreImportExtendedKeyInput = new QLineEdit(scrollContent);
    extKeystoreImportExtendedKeyInput->setPlaceholderText("Private Key (hex)...");
    extKeystoreImportExtendedKeyPassphraseInput = new QLineEdit(scrollContent);
    extKeystoreImportExtendedKeyPassphraseInput->setPlaceholderText("Passphrase...");
    extKeystoreImportExtendedKeyPassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreImportExtendedKeyButton = new QPushButton("Import ECDSA", scrollContent);
    extKeystoreImportECDSALayout->addWidget(extKeystoreImportExtendedKeyInput);
    extKeystoreImportECDSALayout->addWidget(extKeystoreImportExtendedKeyPassphraseInput);
    extKeystoreImportECDSALayout->addWidget(extKeystoreImportExtendedKeyButton);
    extKeystoreLayout->addLayout(extKeystoreImportECDSALayout);
    extKeystoreImportExtendedKeyResultLabel = new QLabel("Import Result:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreImportExtendedKeyResultLabel);
    extKeystoreImportExtendedKeyResult = new QLineEdit(scrollContent);
    extKeystoreImportExtendedKeyResult->setReadOnly(true);
    extKeystoreImportExtendedKeyResult->setPlaceholderText("Address will appear here...");
    extKeystoreLayout->addWidget(extKeystoreImportExtendedKeyResult);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore Sign Tx
    QLabel* extKeystoreSignTxLabel = new QLabel("Sign Transaction:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreSignTxLabel);
    extKeystoreSignTxAddressInput = new QLineEdit(scrollContent);
    extKeystoreSignTxAddressInput->setPlaceholderText("Address...");
    extKeystoreSignTxTxJSONInput = new QTextEdit(scrollContent);
    extKeystoreSignTxTxJSONInput->setPlaceholderText("Transaction JSON...");
    extKeystoreSignTxTxJSONInput->setMaximumHeight(80);
    extKeystoreLayout->addWidget(extKeystoreSignTxAddressInput);
    extKeystoreLayout->addWidget(extKeystoreSignTxTxJSONInput);
    QHBoxLayout* extKeystoreSignTxLayout = new QHBoxLayout();
    extKeystoreSignTxChainIDInput = new QLineEdit(scrollContent);
    extKeystoreSignTxChainIDInput->setPlaceholderText("Chain ID (hex)...");
    extKeystoreSignTxButton = new QPushButton("Sign Transaction", scrollContent);
    extKeystoreSignTxLayout->addWidget(extKeystoreSignTxChainIDInput);
    extKeystoreSignTxLayout->addWidget(extKeystoreSignTxButton);
    extKeystoreLayout->addLayout(extKeystoreSignTxLayout);
    extKeystoreSignTxResultLabel = new QLabel("Signed Transaction Result:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreSignTxResultLabel);
    extKeystoreSignTxResult = new QTextEdit(scrollContent);
    extKeystoreSignTxResult->setReadOnly(true);
    extKeystoreSignTxResult->setPlaceholderText("Signed transaction will appear here...");
    extKeystoreSignTxResult->setMaximumHeight(80);
    extKeystoreLayout->addWidget(extKeystoreSignTxResult);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Keystore Sign Tx With Passphrase
    QLabel* extKeystoreSignTxWithPassphraseLabel = new QLabel("Sign Transaction With Passphrase:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreSignTxWithPassphraseLabel);
    extKeystoreSignTxWithPassphraseAddressInput = new QLineEdit(scrollContent);
    extKeystoreSignTxWithPassphraseAddressInput->setPlaceholderText("Address...");
    extKeystoreSignTxWithPassphrasePassphraseInput = new QLineEdit(scrollContent);
    extKeystoreSignTxWithPassphrasePassphraseInput->setPlaceholderText("Passphrase...");
    extKeystoreSignTxWithPassphrasePassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreSignTxWithPassphraseTxJSONInput = new QTextEdit(scrollContent);
    extKeystoreSignTxWithPassphraseTxJSONInput->setPlaceholderText("Transaction JSON...");
    extKeystoreSignTxWithPassphraseTxJSONInput->setMaximumHeight(80);
    extKeystoreLayout->addWidget(extKeystoreSignTxWithPassphraseAddressInput);
    extKeystoreLayout->addWidget(extKeystoreSignTxWithPassphrasePassphraseInput);
    extKeystoreLayout->addWidget(extKeystoreSignTxWithPassphraseTxJSONInput);
    QHBoxLayout* extKeystoreSignTxWithPassphraseLayout = new QHBoxLayout();
    extKeystoreSignTxWithPassphraseChainIDInput = new QLineEdit(scrollContent);
    extKeystoreSignTxWithPassphraseChainIDInput->setPlaceholderText("Chain ID (hex)...");
    extKeystoreSignTxWithPassphraseButton = new QPushButton("Sign Transaction With Passphrase", scrollContent);
    extKeystoreSignTxWithPassphraseLayout->addWidget(extKeystoreSignTxWithPassphraseChainIDInput);
    extKeystoreSignTxWithPassphraseLayout->addWidget(extKeystoreSignTxWithPassphraseButton);
    extKeystoreLayout->addLayout(extKeystoreSignTxWithPassphraseLayout);
    extKeystoreSignTxWithPassphraseResultLabel = new QLabel("Signed Transaction Result:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreSignTxWithPassphraseResultLabel);
    extKeystoreSignTxWithPassphraseResult = new QTextEdit(scrollContent);
    extKeystoreSignTxWithPassphraseResult->setReadOnly(true);
    extKeystoreSignTxWithPassphraseResult->setPlaceholderText("Signed transaction will appear here...");
    extKeystoreSignTxWithPassphraseResult->setMaximumHeight(80);
    extKeystoreLayout->addWidget(extKeystoreSignTxWithPassphraseResult);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Extended Keystore Derive
    QHBoxLayout* extKeystoreDeriveLayout = new QHBoxLayout();
    extKeystoreDeriveAddressInput = new QLineEdit(scrollContent);
    extKeystoreDeriveAddressInput->setPlaceholderText("Address...");
    extKeystoreDerivePathInput = new QLineEdit(scrollContent);
    extKeystoreDerivePathInput->setPlaceholderText("Derivation Path (e.g., m/44'/60'/0'/0/0)...");
    extKeystoreDerivePinInput = new QLineEdit(scrollContent);
    extKeystoreDerivePinInput->setPlaceholderText("PIN...");
    extKeystoreDeriveButton = new QPushButton("Derive", scrollContent);
    extKeystoreDeriveLayout->addWidget(extKeystoreDeriveAddressInput);
    extKeystoreDeriveLayout->addWidget(extKeystoreDerivePathInput);
    extKeystoreDeriveLayout->addWidget(extKeystoreDerivePinInput);
    extKeystoreDeriveLayout->addWidget(extKeystoreDeriveButton);
    extKeystoreLayout->addLayout(extKeystoreDeriveLayout);
    extKeystoreDeriveResultLabel = new QLabel("Derived Address Result:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreDeriveResultLabel);
    extKeystoreDeriveResult = new QLineEdit(scrollContent);
    extKeystoreDeriveResult->setReadOnly(true);
    extKeystoreDeriveResult->setPlaceholderText("Derived address will appear here...");
    extKeystoreLayout->addWidget(extKeystoreDeriveResult);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Extended Keystore Derive With Passphrase
    QHBoxLayout* extKeystoreDeriveWithPassphraseLayout = new QHBoxLayout();
    extKeystoreDeriveWithPassphraseAddressInput = new QLineEdit(scrollContent);
    extKeystoreDeriveWithPassphraseAddressInput->setPlaceholderText("Address...");
    extKeystoreDeriveWithPassphrasePathInput = new QLineEdit(scrollContent);
    extKeystoreDeriveWithPassphrasePathInput->setPlaceholderText("Derivation Path...");
    extKeystoreDeriveWithPassphrasePinInput = new QLineEdit(scrollContent);
    extKeystoreDeriveWithPassphrasePinInput->setPlaceholderText("PIN...");
    extKeystoreDeriveWithPassphrasePassphraseInput = new QLineEdit(scrollContent);
    extKeystoreDeriveWithPassphrasePassphraseInput->setPlaceholderText("Passphrase...");
    extKeystoreDeriveWithPassphrasePassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreDeriveWithPassphraseNewPassphraseInput = new QLineEdit(scrollContent);
    extKeystoreDeriveWithPassphraseNewPassphraseInput->setPlaceholderText("New Passphrase...");
    extKeystoreDeriveWithPassphraseNewPassphraseInput->setEchoMode(QLineEdit::Password);
    extKeystoreDeriveWithPassphraseButton = new QPushButton("Derive With Passphrase", scrollContent);
    extKeystoreDeriveWithPassphraseLayout->addWidget(extKeystoreDeriveWithPassphraseAddressInput);
    extKeystoreDeriveWithPassphraseLayout->addWidget(extKeystoreDeriveWithPassphrasePathInput);
    extKeystoreDeriveWithPassphraseLayout->addWidget(extKeystoreDeriveWithPassphrasePinInput);
    extKeystoreDeriveWithPassphraseLayout->addWidget(extKeystoreDeriveWithPassphrasePassphraseInput);
    extKeystoreDeriveWithPassphraseLayout->addWidget(extKeystoreDeriveWithPassphraseNewPassphraseInput);
    extKeystoreDeriveWithPassphraseLayout->addWidget(extKeystoreDeriveWithPassphraseButton);
    extKeystoreLayout->addLayout(extKeystoreDeriveWithPassphraseLayout);
    extKeystoreDeriveWithPassphraseResultLabel = new QLabel("Derived Address Result:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreDeriveWithPassphraseResultLabel);
    extKeystoreDeriveWithPassphraseResult = new QLineEdit(scrollContent);
    extKeystoreDeriveWithPassphraseResult->setReadOnly(true);
    extKeystoreDeriveWithPassphraseResult->setPlaceholderText("Derived address will appear here...");
    extKeystoreLayout->addWidget(extKeystoreDeriveWithPassphraseResult);
    extKeystoreLayout->addWidget(createSeparator());
    
    // Extended Keystore Find
    QHBoxLayout* extKeystoreFindLayout = new QHBoxLayout();
    extKeystoreFindAddressInput = new QLineEdit(scrollContent);
    extKeystoreFindAddressInput->setPlaceholderText("Address...");
    extKeystoreFindUrlInput = new QLineEdit(scrollContent);
    extKeystoreFindUrlInput->setPlaceholderText("URL...");
    extKeystoreFindButton = new QPushButton("Find", scrollContent);
    extKeystoreFindLayout->addWidget(extKeystoreFindAddressInput);
    extKeystoreFindLayout->addWidget(extKeystoreFindUrlInput);
    extKeystoreFindLayout->addWidget(extKeystoreFindButton);
    extKeystoreLayout->addLayout(extKeystoreFindLayout);
    extKeystoreFindResultLabel = new QLabel("Find Result:", scrollContent);
    extKeystoreLayout->addWidget(extKeystoreFindResultLabel);
    extKeystoreFindResult = new QLineEdit(scrollContent);
    extKeystoreFindResult->setReadOnly(true);
    extKeystoreFindResult->setPlaceholderText("Result will appear here...");
    extKeystoreLayout->addWidget(extKeystoreFindResult);
    
    // Add frame to scroll layout
    scrollLayout->addWidget(extKeystoreFrame);
    scrollLayout->addStretch();
    
    // Add scroll area to main layout
    mainLayout->addWidget(scrollArea);
    
    // Connect signals
    connect(extKeystoreInitButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreInitButtonClicked);
    connect(extKeystoreCloseButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreCloseButtonClicked);
    connect(extKeystoreListAccountsButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreListAccountsButtonClicked);
    connect(extKeystoreNewAccountButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreNewAccountButtonClicked);
    connect(extKeystoreImportButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreImportButtonClicked);
    connect(extKeystoreExportExtButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreExportExtButtonClicked);
    connect(extKeystoreExportPrivButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreExportPrivButtonClicked);
    connect(extKeystoreDeleteButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreDeleteButtonClicked);
    connect(extKeystoreHasAddressButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreHasAddressButtonClicked);
    connect(extKeystoreUnlockButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreUnlockButtonClicked);
    connect(extKeystoreLockButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreLockButtonClicked);
    connect(extKeystoreTimedUnlockButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreTimedUnlockButtonClicked);
    connect(extKeystoreUpdateButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreUpdateButtonClicked);
    connect(extKeystoreSignHashButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreSignHashButtonClicked);
    connect(extKeystoreSignHashWithPassphraseButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreSignHashWithPassphraseButtonClicked);
    connect(extKeystoreImportExtendedKeyButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreImportExtendedKeyButtonClicked);
    connect(extKeystoreSignTxButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreSignTxButtonClicked);
    connect(extKeystoreSignTxWithPassphraseButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreSignTxWithPassphraseButtonClicked);
    connect(extKeystoreDeriveButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreDeriveButtonClicked);
    connect(extKeystoreDeriveWithPassphraseButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreDeriveWithPassphraseButtonClicked);
    connect(extKeystoreFindButton, &QPushButton::clicked, this, &ExtKeystoreWidget::onExtKeystoreFindButtonClicked);
}

ExtKeystoreWidget::~ExtKeystoreWidget() {
}

QFrame* ExtKeystoreWidget::createSeparator() {
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    return separator;
}

// Extended keystore operations
void ExtKeystoreWidget::onExtKeystoreInitButtonClicked() {
    QString dir = extKeystoreDirInput->text();
    int scryptN = extKeystoreScryptNInput->text().toInt();
    int scryptP = extKeystoreScryptPInput->text().toInt();
    
    if (dir.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a keystore directory");
        return;
    }
    
    emit statusUpdate("Initializing keystore...");
    bool success = m_logos->accounts_module.initExtKeystore(dir, scryptN, scryptP);
    
    if (success) {
        emit statusUpdate("Keystore initialized successfully");
        QMessageBox::information(this, "Success", "Keystore initialized successfully");
        onExtKeystoreListAccountsButtonClicked(); // Refresh list
    } else {
        emit statusUpdate("Failed to initialize keystore");
        QMessageBox::warning(this, "Error", "Failed to initialize keystore");
    }
}

void ExtKeystoreWidget::onExtKeystoreCloseButtonClicked() {
    emit statusUpdate("Closing extended keystore...");
    bool success = m_logos->accounts_module.closeExtKeystore();
    
    if (success) {
        emit statusUpdate("Keystore closed successfully");
        QMessageBox::information(this, "Success", "Keystore closed successfully");
    } else {
        emit statusUpdate("Failed to close keystore");
        QMessageBox::warning(this, "Error", "Failed to close keystore");
    }
}

void ExtKeystoreWidget::onExtKeystoreListAccountsButtonClicked() {
    emit statusUpdate("Listing accounts...");
    QStringList accounts = m_logos->accounts_module.extKeystoreAccounts();
    
    extKeystoreAccountsList->clear();
    for (const QString& account : accounts) {
        extKeystoreAccountsList->addItem(account);
    }
    
    emit statusUpdate(QString("Found %1 accounts").arg(accounts.size()));
}

void ExtKeystoreWidget::onExtKeystoreNewAccountButtonClicked() {
    QString passphrase = extKeystoreNewAccountPassphraseInput->text();
    
    if (passphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a passphrase");
        return;
    }
    
    emit statusUpdate("Creating new account...");
    QString address = m_logos->accounts_module.extKeystoreNewAccount(passphrase);
    
    if (!address.isEmpty()) {
        emit statusUpdate(QString("New account created: %1").arg(address));
        QMessageBox::information(this, "Success", QString("New account created:\n%1").arg(address));
        extKeystoreNewAccountPassphraseInput->clear();
        onExtKeystoreListAccountsButtonClicked(); // Refresh list
    } else {
        emit statusUpdate("Failed to create new account");
        QMessageBox::warning(this, "Error", "Failed to create new account");
    }
}

void ExtKeystoreWidget::onExtKeystoreImportButtonClicked() {
    QString keyJSON = extKeystoreImportKeyJSONInput->toPlainText();
    QString passphrase = extKeystoreImportPassphraseInput->text();
    QString newPassphrase = extKeystoreImportNewPassphraseInput->text();
    
    if (keyJSON.isEmpty() || passphrase.isEmpty() || newPassphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Importing account...");
    QString address = m_logos->accounts_module.extKeystoreImport(keyJSON, passphrase, newPassphrase);
    
    if (!address.isEmpty()) {
        emit statusUpdate(QString("Account imported: %1").arg(address));
        QMessageBox::information(this, "Success", QString("Account imported:\n%1").arg(address));
        extKeystoreImportKeyJSONInput->clear();
        extKeystoreImportPassphraseInput->clear();
        extKeystoreImportNewPassphraseInput->clear();
        onExtKeystoreListAccountsButtonClicked(); // Refresh list
    } else {
        emit statusUpdate("Failed to import account");
        QMessageBox::warning(this, "Error", "Failed to import account");
    }
}

void ExtKeystoreWidget::onExtKeystoreExportExtButtonClicked() {
    QString address = extKeystoreExportExtAddressInput->text();
    QString passphrase = extKeystoreExportExtPassphraseInput->text();
    QString newPassphrase = extKeystoreExportExtNewPassphraseInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty() || newPassphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Exporting extended key...");
    QString keyJSON = m_logos->accounts_module.extKeystoreExportExt(address, passphrase, newPassphrase);
    
    if (!keyJSON.isEmpty()) {
        extKeystoreExportExtResult->setPlainText(keyJSON);
        emit statusUpdate(QString("Extended key exported: %1").arg(address));
    } else {
        extKeystoreExportExtResult->setPlainText("Export failed");
        emit statusUpdate("Failed to export extended key");
        QMessageBox::warning(this, "Error", "Failed to export extended key");
    }
}

void ExtKeystoreWidget::onExtKeystoreExportPrivButtonClicked() {
    QString address = extKeystoreExportPrivAddressInput->text();
    QString passphrase = extKeystoreExportPrivPassphraseInput->text();
    QString newPassphrase = extKeystoreExportPrivNewPassphraseInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty() || newPassphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Exporting private key...");
    QString keyJSON = m_logos->accounts_module.extKeystoreExportPriv(address, passphrase, newPassphrase);
    
    if (!keyJSON.isEmpty()) {
        extKeystoreExportPrivResult->setPlainText(keyJSON);
        emit statusUpdate(QString("Private key exported: %1").arg(address));
    } else {
        extKeystoreExportPrivResult->setPlainText("Export failed");
        emit statusUpdate("Failed to export private key");
        QMessageBox::warning(this, "Error", "Failed to export private key");
    }
}

void ExtKeystoreWidget::onExtKeystoreDeleteButtonClicked() {
    QString address = extKeystoreDeleteAddressInput->text();
    QString passphrase = extKeystoreDeletePassphraseInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete", 
        QString("Are you sure you want to delete account %1?").arg(address),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        emit statusUpdate("Deleting account...");
        bool success = m_logos->accounts_module.extKeystoreDelete(address, passphrase);
        
        if (success) {
            emit statusUpdate(QString("Account deleted: %1").arg(address));
            QMessageBox::information(this, "Success", "Account deleted successfully");
            extKeystoreDeleteAddressInput->clear();
            extKeystoreDeletePassphraseInput->clear();
            onExtKeystoreListAccountsButtonClicked(); // Refresh list
        } else {
            emit statusUpdate("Failed to delete account");
            QMessageBox::warning(this, "Error", "Failed to delete account");
        }
    }
}

void ExtKeystoreWidget::onExtKeystoreHasAddressButtonClicked() {
    QString address = extKeystoreHasAddressInput->text();
    
    if (address.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter an address");
        return;
    }
    
    emit statusUpdate("Checking if address exists...");
    bool hasAddress = m_logos->accounts_module.extKeystoreHasAddress(address);
    
    extKeystoreHasAddressResult->setText(hasAddress ? "true" : "false");
    emit statusUpdate(hasAddress ? "Address exists" : "Address does not exist");
}

void ExtKeystoreWidget::onExtKeystoreUnlockButtonClicked() {
    QString address = extKeystoreUnlockAddressInput->text();
    QString passphrase = extKeystoreUnlockPassphraseInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Unlocking account...");
    bool success = m_logos->accounts_module.extKeystoreUnlock(address, passphrase);
    
    if (success) {
        emit statusUpdate(QString("Account unlocked: %1").arg(address));
        QMessageBox::information(this, "Success", "Account unlocked successfully");
        extKeystoreUnlockPassphraseInput->clear();
    } else {
        emit statusUpdate("Failed to unlock account");
        QMessageBox::warning(this, "Error", "Failed to unlock account");
    }
}

void ExtKeystoreWidget::onExtKeystoreLockButtonClicked() {
    QString address = extKeystoreLockAddressInput->text();
    
    if (address.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter an address");
        return;
    }
    
    emit statusUpdate("Locking account...");
    bool success = m_logos->accounts_module.extKeystoreLock(address);
    
    if (success) {
        emit statusUpdate(QString("Account locked: %1").arg(address));
        QMessageBox::information(this, "Success", "Account locked successfully");
        extKeystoreLockAddressInput->clear();
    } else {
        emit statusUpdate("Failed to lock account");
        QMessageBox::warning(this, "Error", "Failed to lock account");
    }
}

void ExtKeystoreWidget::onExtKeystoreTimedUnlockButtonClicked() {
    QString address = extKeystoreTimedUnlockAddressInput->text();
    QString passphrase = extKeystoreTimedUnlockPassphraseInput->text();
    QString timeoutStr = extKeystoreTimedUnlockTimeoutInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty() || timeoutStr.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    uint timeout = timeoutStr.toUInt();
    emit statusUpdate("Unlocking account with timeout...");
    bool success = m_logos->accounts_module.extKeystoreTimedUnlock(address, passphrase, timeout);
    
    if (success) {
        emit statusUpdate(QString("Account unlocked with timeout: %1").arg(address));
        QMessageBox::information(this, "Success", "Account unlocked successfully");
        extKeystoreTimedUnlockPassphraseInput->clear();
    } else {
        emit statusUpdate("Failed to unlock account");
        QMessageBox::warning(this, "Error", "Failed to unlock account");
    }
}

void ExtKeystoreWidget::onExtKeystoreUpdateButtonClicked() {
    QString address = extKeystoreUpdateAddressInput->text();
    QString passphrase = extKeystoreUpdatePassphraseInput->text();
    QString newPassphrase = extKeystoreUpdateNewPassphraseInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty() || newPassphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Updating passphrase...");
    bool success = m_logos->accounts_module.extKeystoreUpdate(address, passphrase, newPassphrase);
    
    if (success) {
        emit statusUpdate(QString("Passphrase updated: %1").arg(address));
        QMessageBox::information(this, "Success", "Passphrase updated successfully");
        extKeystoreUpdatePassphraseInput->clear();
        extKeystoreUpdateNewPassphraseInput->clear();
    } else {
        emit statusUpdate("Failed to update passphrase");
        QMessageBox::warning(this, "Error", "Failed to update passphrase");
    }
}

void ExtKeystoreWidget::onExtKeystoreSignHashButtonClicked() {
    QString address = extKeystoreSignHashAddressInput->text();
    QString hashHex = extKeystoreSignHashHashInput->text();
    
    if (address.isEmpty() || hashHex.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Signing hash...");
    QString signature = m_logos->accounts_module.extKeystoreSignHash(address, hashHex);
    
    if (!signature.isEmpty()) {
        extKeystoreSignHashResult->setText(signature);
        emit statusUpdate("Hash signed successfully");
    } else {
        extKeystoreSignHashResult->setText("Signing failed");
        emit statusUpdate("Failed to sign hash");
        QMessageBox::warning(this, "Error", "Failed to sign hash");
    }
}

void ExtKeystoreWidget::onExtKeystoreSignHashWithPassphraseButtonClicked() {
    QString address = extKeystoreSignHashWithPassphraseAddressInput->text();
    QString passphrase = extKeystoreSignHashWithPassphrasePassphraseInput->text();
    QString hashHex = extKeystoreSignHashWithPassphraseHashInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty() || hashHex.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Signing hash with passphrase...");
    QString signature = m_logos->accounts_module.extKeystoreSignHashWithPassphrase(address, passphrase, hashHex);
    
    if (!signature.isEmpty()) {
        extKeystoreSignHashWithPassphraseResult->setText(signature);
        emit statusUpdate("Hash signed successfully");
    } else {
        extKeystoreSignHashWithPassphraseResult->setText("Signing failed");
        emit statusUpdate("Failed to sign hash");
        QMessageBox::warning(this, "Error", "Failed to sign hash");
    }
}

void ExtKeystoreWidget::onExtKeystoreImportExtendedKeyButtonClicked() {
    QString extKeyStr = extKeystoreImportExtendedKeyInput->text();
    QString passphrase = extKeystoreImportExtendedKeyPassphraseInput->text();
    
    if (extKeyStr.isEmpty() || passphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Importing extended key...");
    QString address = m_logos->accounts_module.extKeystoreImportExtendedKey(extKeyStr, passphrase);
    
    if (!address.isEmpty()) {
        extKeystoreImportExtendedKeyResult->setText(address);
        emit statusUpdate(QString("Extended key imported: %1").arg(address));
        QMessageBox::information(this, "Success", QString("Extended key imported:\n%1").arg(address));
        extKeystoreImportExtendedKeyInput->clear();
        extKeystoreImportExtendedKeyPassphraseInput->clear();
        onExtKeystoreListAccountsButtonClicked();
    } else {
        extKeystoreImportExtendedKeyResult->setText("Import failed");
        emit statusUpdate("Failed to import extended key");
        QMessageBox::warning(this, "Error", "Failed to import extended key");
    }
}

void ExtKeystoreWidget::onExtKeystoreSignTxButtonClicked() {
    QString address = extKeystoreSignTxAddressInput->text();
    QString txJSON = extKeystoreSignTxTxJSONInput->toPlainText();
    QString chainIDHex = extKeystoreSignTxChainIDInput->text();
    
    if (address.isEmpty() || txJSON.isEmpty() || chainIDHex.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Signing transaction...");
    QString signedTx = m_logos->accounts_module.extKeystoreSignTx(address, txJSON, chainIDHex);
    
    if (!signedTx.isEmpty()) {
        extKeystoreSignTxResult->setPlainText(signedTx);
        emit statusUpdate("Transaction signed successfully");
    } else {
        extKeystoreSignTxResult->setPlainText("Signing failed");
        emit statusUpdate("Failed to sign transaction");
        QMessageBox::warning(this, "Error", "Failed to sign transaction");
    }
}

void ExtKeystoreWidget::onExtKeystoreSignTxWithPassphraseButtonClicked() {
    QString address = extKeystoreSignTxWithPassphraseAddressInput->text();
    QString passphrase = extKeystoreSignTxWithPassphrasePassphraseInput->text();
    QString txJSON = extKeystoreSignTxWithPassphraseTxJSONInput->toPlainText();
    QString chainIDHex = extKeystoreSignTxWithPassphraseChainIDInput->text();
    
    if (address.isEmpty() || passphrase.isEmpty() || txJSON.isEmpty() || chainIDHex.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Signing transaction with passphrase...");
    QString signedTx = m_logos->accounts_module.extKeystoreSignTxWithPassphrase(address, passphrase, txJSON, chainIDHex);
    
    if (!signedTx.isEmpty()) {
        extKeystoreSignTxWithPassphraseResult->setPlainText(signedTx);
        emit statusUpdate("Transaction signed successfully");
    } else {
        extKeystoreSignTxWithPassphraseResult->setPlainText("Signing failed");
        emit statusUpdate("Failed to sign transaction");
        QMessageBox::warning(this, "Error", "Failed to sign transaction");
    }
}

void ExtKeystoreWidget::onExtKeystoreDeriveButtonClicked() {
    QString address = extKeystoreDeriveAddressInput->text();
    QString derivationPath = extKeystoreDerivePathInput->text();
    int pin = extKeystoreDerivePinInput->text().toInt();
    
    if (address.isEmpty() || derivationPath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in address and derivation path");
        return;
    }
    
    emit statusUpdate("Deriving address...");
    QString derivedAddress = m_logos->accounts_module.extKeystoreDerive(address, derivationPath, pin);
    
    if (!derivedAddress.isEmpty()) {
        extKeystoreDeriveResult->setText(derivedAddress);
        emit statusUpdate("Address derived successfully");
    } else {
        extKeystoreDeriveResult->setText("Derivation failed");
        emit statusUpdate("Failed to derive address");
        QMessageBox::warning(this, "Error", "Failed to derive address");
    }
}

void ExtKeystoreWidget::onExtKeystoreDeriveWithPassphraseButtonClicked() {
    QString address = extKeystoreDeriveWithPassphraseAddressInput->text();
    QString derivationPath = extKeystoreDeriveWithPassphrasePathInput->text();
    QString pinStr = extKeystoreDeriveWithPassphrasePinInput->text();
    QString passphrase = extKeystoreDeriveWithPassphrasePassphraseInput->text();
    QString newPassphrase = extKeystoreDeriveWithPassphraseNewPassphraseInput->text();
    
    if (address.isEmpty() || derivationPath.isEmpty() || pinStr.isEmpty() || passphrase.isEmpty() || newPassphrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    int pin = pinStr.toInt();
    emit statusUpdate("Deriving address with passphrase...");
    QString derivedAddress = m_logos->accounts_module.extKeystoreDeriveWithPassphrase(address, derivationPath, pin, passphrase, newPassphrase);
    
    if (!derivedAddress.isEmpty()) {
        extKeystoreDeriveWithPassphraseResult->setText(derivedAddress);
        emit statusUpdate("Address derived successfully");
    } else {
        extKeystoreDeriveWithPassphraseResult->setText("Derivation failed");
        emit statusUpdate("Failed to derive address");
        QMessageBox::warning(this, "Error", "Failed to derive address");
    }
}

void ExtKeystoreWidget::onExtKeystoreFindButtonClicked() {
    QString address = extKeystoreFindAddressInput->text();
    QString url = extKeystoreFindUrlInput->text();
    
    if (address.isEmpty() || url.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Finding account...");
    QString result = m_logos->accounts_module.extKeystoreFind(address, url);
    
    if (!result.isEmpty()) {
        extKeystoreFindResult->setText(result);
        emit statusUpdate("Account found");
    } else {
        extKeystoreFindResult->setText("Not found");
        emit statusUpdate("Account not found");
    }
}

