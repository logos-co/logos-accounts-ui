#include "KeysWidget.h"
#include <QDebug>
#include <QMessageBox>

KeysWidget::KeysWidget(LogosModules* logos, QWidget* parent)
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
    
    // Create Mnemonic frame
    mnemonicFrame = new QFrame(scrollContent);
    QVBoxLayout* mnemonicLayout = new QVBoxLayout(mnemonicFrame);
    mnemonicFrame->setLayout(mnemonicLayout);
    mnemonicFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mnemonicFrame->setLineWidth(1);
    
    QLabel* mnemonicTitle = new QLabel("<b>Mnemonic Operations</b>", scrollContent);
    mnemonicLayout->addWidget(mnemonicTitle);
    
    QHBoxLayout* mnemonicLayout2 = new QHBoxLayout();
    mnemonicLengthInput = new QLineEdit(scrollContent);
    mnemonicLengthInput->setPlaceholderText("Length (12, 15, 18, 21, 24) - leave empty for default");
    createMnemonicButton = new QPushButton("Create Random Mnemonic", scrollContent);
    mnemonicLayout2->addWidget(mnemonicLengthInput);
    mnemonicLayout2->addWidget(createMnemonicButton);
    mnemonicLayout->addLayout(mnemonicLayout2);
    mnemonicResultLabel = new QLabel("Mnemonic Result:", scrollContent);
    mnemonicLayout->addWidget(mnemonicResultLabel);
    mnemonicResult = new QTextEdit(scrollContent);
    mnemonicResult->setReadOnly(true);
    mnemonicResult->setPlaceholderText("Mnemonic will appear here...");
    mnemonicResult->setMaximumHeight(60);
    mnemonicLayout->addWidget(mnemonicResult);
    mnemonicLayout->addWidget(createSeparator());
    
    // Length To Entropy Strength
    QHBoxLayout* lengthToEntropyStrengthLayout = new QHBoxLayout();
    lengthToEntropyStrengthInput = new QLineEdit(scrollContent);
    lengthToEntropyStrengthInput->setPlaceholderText("Length (12, 15, 18, 21, 24)...");
    lengthToEntropyStrengthButton = new QPushButton("Length To Entropy Strength", scrollContent);
    lengthToEntropyStrengthLayout->addWidget(lengthToEntropyStrengthInput);
    lengthToEntropyStrengthLayout->addWidget(lengthToEntropyStrengthButton);
    mnemonicLayout->addLayout(lengthToEntropyStrengthLayout);
    lengthToEntropyStrengthResultLabel = new QLabel("Entropy Strength Result:", scrollContent);
    mnemonicLayout->addWidget(lengthToEntropyStrengthResultLabel);
    lengthToEntropyStrengthResult = new QLineEdit(scrollContent);
    lengthToEntropyStrengthResult->setReadOnly(true);
    lengthToEntropyStrengthResult->setPlaceholderText("Entropy strength will appear here...");
    mnemonicLayout->addWidget(lengthToEntropyStrengthResult);
    
    // Create Key Operations frame
    keyOpsFrame = new QFrame(scrollContent);
    QVBoxLayout* keyOpsLayout = new QVBoxLayout(keyOpsFrame);
    keyOpsFrame->setLayout(keyOpsLayout);
    keyOpsFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    keyOpsFrame->setLineWidth(1);
    
    QLabel* keyOpsTitle = new QLabel("<b>Key Operations</b>", scrollContent);
    keyOpsLayout->addWidget(keyOpsTitle);
    
    // Create Extended Key from Mnemonic
    QLabel* createExtKeyLabel = new QLabel("Create Extended Key from Mnemonic:", scrollContent);
    keyOpsLayout->addWidget(createExtKeyLabel);
    createExtKeyMnemonicInput = new QTextEdit(scrollContent);
    createExtKeyMnemonicInput->setPlaceholderText("Mnemonic phrase...");
    createExtKeyMnemonicInput->setMaximumHeight(60);
    keyOpsLayout->addWidget(createExtKeyMnemonicInput);
    QHBoxLayout* createExtKeyLayout = new QHBoxLayout();
    createExtKeyPassphraseInput = new QLineEdit(scrollContent);
    createExtKeyPassphraseInput->setPlaceholderText("Passphrase...");
    createExtKeyPassphraseInput->setEchoMode(QLineEdit::Password);
    createExtKeyFromMnemonicButton = new QPushButton("Create Extended Key", scrollContent);
    createExtKeyLayout->addWidget(createExtKeyPassphraseInput);
    createExtKeyLayout->addWidget(createExtKeyFromMnemonicButton);
    keyOpsLayout->addLayout(createExtKeyLayout);
    createExtKeyResultLabel = new QLabel("Extended Key Result:", scrollContent);
    keyOpsLayout->addWidget(createExtKeyResultLabel);
    createExtKeyResult = new QTextEdit(scrollContent);
    createExtKeyResult->setReadOnly(true);
    createExtKeyResult->setPlaceholderText("Extended key will appear here...");
    createExtKeyResult->setMaximumHeight(60);
    keyOpsLayout->addWidget(createExtKeyResult);
    
    // Derive Extended Key
    QLabel* deriveExtKeyLabel = new QLabel("Derive Extended Key:", scrollContent);
    keyOpsLayout->addWidget(deriveExtKeyLabel);
    QHBoxLayout* deriveExtKeyLayout = new QHBoxLayout();
    deriveExtKeyInput = new QLineEdit(scrollContent);
    deriveExtKeyInput->setPlaceholderText("Extended Key...");
    deriveExtKeyPathInput = new QLineEdit(scrollContent);
    deriveExtKeyPathInput->setPlaceholderText("Derivation Path (e.g., m/44'/60'/0'/0/0)...");
    deriveExtKeyButton = new QPushButton("Derive", scrollContent);
    deriveExtKeyLayout->addWidget(deriveExtKeyInput);
    deriveExtKeyLayout->addWidget(deriveExtKeyPathInput);
    deriveExtKeyLayout->addWidget(deriveExtKeyButton);
    keyOpsLayout->addLayout(deriveExtKeyLayout);
    deriveExtKeyResultLabel = new QLabel("Derived Key Result:", scrollContent);
    keyOpsLayout->addWidget(deriveExtKeyResultLabel);
    deriveExtKeyResult = new QTextEdit(scrollContent);
    deriveExtKeyResult->setReadOnly(true);
    deriveExtKeyResult->setPlaceholderText("Derived key will appear here...");
    deriveExtKeyResult->setMaximumHeight(60);
    keyOpsLayout->addWidget(deriveExtKeyResult);
    keyOpsLayout->addWidget(createSeparator());
    
    // Ext Key To ECDSA
    QHBoxLayout* extKeyToECDSALayout = new QHBoxLayout();
    extKeyToECDSAInput = new QLineEdit(scrollContent);
    extKeyToECDSAInput->setPlaceholderText("Extended Key...");
    extKeyToECDSAButton = new QPushButton("Ext Key To ECDSA", scrollContent);
    extKeyToECDSALayout->addWidget(extKeyToECDSAInput);
    extKeyToECDSALayout->addWidget(extKeyToECDSAButton);
    keyOpsLayout->addLayout(extKeyToECDSALayout);
    extKeyToECDSAResultLabel = new QLabel("ECDSA Private Key Result:", scrollContent);
    keyOpsLayout->addWidget(extKeyToECDSAResultLabel);
    extKeyToECDSAResult = new QLineEdit(scrollContent);
    extKeyToECDSAResult->setReadOnly(true);
    extKeyToECDSAResult->setPlaceholderText("ECDSA private key will appear here...");
    keyOpsLayout->addWidget(extKeyToECDSAResult);
    keyOpsLayout->addWidget(createSeparator());
    
    // ECDSA To Public Key
    QHBoxLayout* ecdsaToPublicKeyLayout = new QHBoxLayout();
    ecdsaToPublicKeyInput = new QLineEdit(scrollContent);
    ecdsaToPublicKeyInput->setPlaceholderText("ECDSA Private Key (hex)...");
    ecdsaToPublicKeyButton = new QPushButton("ECDSA To Public Key", scrollContent);
    ecdsaToPublicKeyLayout->addWidget(ecdsaToPublicKeyInput);
    ecdsaToPublicKeyLayout->addWidget(ecdsaToPublicKeyButton);
    keyOpsLayout->addLayout(ecdsaToPublicKeyLayout);
    ecdsaToPublicKeyResultLabel = new QLabel("Public Key Result:", scrollContent);
    keyOpsLayout->addWidget(ecdsaToPublicKeyResultLabel);
    ecdsaToPublicKeyResult = new QLineEdit(scrollContent);
    ecdsaToPublicKeyResult->setReadOnly(true);
    ecdsaToPublicKeyResult->setPlaceholderText("Public key will appear here...");
    keyOpsLayout->addWidget(ecdsaToPublicKeyResult);
    keyOpsLayout->addWidget(createSeparator());
    
    // Public Key To Address
    QHBoxLayout* publicKeyToAddressLayout = new QHBoxLayout();
    publicKeyToAddressInput = new QLineEdit(scrollContent);
    publicKeyToAddressInput->setPlaceholderText("Public Key (hex)...");
    publicKeyToAddressButton = new QPushButton("Public Key To Address", scrollContent);
    publicKeyToAddressLayout->addWidget(publicKeyToAddressInput);
    publicKeyToAddressLayout->addWidget(publicKeyToAddressButton);
    keyOpsLayout->addLayout(publicKeyToAddressLayout);
    publicKeyToAddressResultLabel = new QLabel("Address Result:", scrollContent);
    keyOpsLayout->addWidget(publicKeyToAddressResultLabel);
    publicKeyToAddressResult = new QLineEdit(scrollContent);
    publicKeyToAddressResult->setReadOnly(true);
    publicKeyToAddressResult->setPlaceholderText("Address will appear here...");
    keyOpsLayout->addWidget(publicKeyToAddressResult);
    
    // Add frames to scroll layout
    scrollLayout->addWidget(mnemonicFrame);
    scrollLayout->addWidget(keyOpsFrame);
    scrollLayout->addStretch();
    
    // Add scroll area to main layout
    mainLayout->addWidget(scrollArea);
    
    // Connect signals
    connect(createMnemonicButton, &QPushButton::clicked, this, &KeysWidget::onCreateMnemonicButtonClicked);
    connect(lengthToEntropyStrengthButton, &QPushButton::clicked, this, &KeysWidget::onLengthToEntropyStrengthButtonClicked);
    connect(createExtKeyFromMnemonicButton, &QPushButton::clicked, this, &KeysWidget::onCreateExtKeyFromMnemonicButtonClicked);
    connect(deriveExtKeyButton, &QPushButton::clicked, this, &KeysWidget::onDeriveExtKeyButtonClicked);
    connect(extKeyToECDSAButton, &QPushButton::clicked, this, &KeysWidget::onExtKeyToECDSAButtonClicked);
    connect(ecdsaToPublicKeyButton, &QPushButton::clicked, this, &KeysWidget::onEcdsaToPublicKeyButtonClicked);
    connect(publicKeyToAddressButton, &QPushButton::clicked, this, &KeysWidget::onPublicKeyToAddressButtonClicked);
}

KeysWidget::~KeysWidget() {
}

QFrame* KeysWidget::createSeparator() {
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    return separator;
}

// Mnemonic operations
void KeysWidget::onCreateMnemonicButtonClicked() {
    QString lengthStr = mnemonicLengthInput->text();
    
    emit statusUpdate("Creating random mnemonic...");
    QString mnemonic;
    
    if (lengthStr.isEmpty()) {
        mnemonic = m_logos->accounts_module.createRandomMnemonicWithDefaultLength();
    } else {
        int length = lengthStr.toInt();
        mnemonic = m_logos->accounts_module.createRandomMnemonic(length);
    }

    qDebug() << "Mnemonic: " << mnemonic;
    
    if (!mnemonic.isEmpty()) {
        mnemonicResult->setPlainText(mnemonic);
        emit statusUpdate("Mnemonic created successfully");
    } else {
        mnemonicResult->setPlainText("Failed to create mnemonic");
        emit statusUpdate("Failed to create mnemonic");
        QMessageBox::warning(this, "Error", "Failed to create mnemonic");
    }
}

void KeysWidget::onLengthToEntropyStrengthButtonClicked() {
    QString lengthStr = lengthToEntropyStrengthInput->text();
    
    if (lengthStr.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a length");
        return;
    }
    
    int length = lengthStr.toInt();
    emit statusUpdate("Calculating entropy strength...");
    int strength = m_logos->accounts_module.lengthToEntropyStrength(length);
    
    lengthToEntropyStrengthResult->setText(QString::number(strength));
    emit statusUpdate("Entropy strength calculated");
}

// Key operations
void KeysWidget::onCreateExtKeyFromMnemonicButtonClicked() {
    QString phrase = createExtKeyMnemonicInput->toPlainText();
    QString passphrase = createExtKeyPassphraseInput->text();
    
    if (phrase.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a mnemonic phrase");
        return;
    }
    
    emit statusUpdate("Creating extended key from mnemonic...");
    QString extKey = m_logos->accounts_module.createExtKeyFromMnemonic(phrase, passphrase);
    
    if (!extKey.isEmpty()) {
        createExtKeyResult->setPlainText(extKey);
        emit statusUpdate("Extended key created successfully");
    } else {
        createExtKeyResult->setPlainText("Failed to create extended key");
        emit statusUpdate("Failed to create extended key");
        QMessageBox::warning(this, "Error", "Failed to create extended key");
    }
}

void KeysWidget::onDeriveExtKeyButtonClicked() {
    QString extKeyStr = deriveExtKeyInput->text();
    QString pathStr = deriveExtKeyPathInput->text();
    
    if (extKeyStr.isEmpty() || pathStr.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }
    
    emit statusUpdate("Deriving extended key...");
    QString derivedKey = m_logos->accounts_module.deriveExtKey(extKeyStr, pathStr);
    
    if (!derivedKey.isEmpty()) {
        deriveExtKeyResult->setPlainText(derivedKey);
        emit statusUpdate("Extended key derived successfully");
    } else {
        deriveExtKeyResult->setPlainText("Derivation failed");
        emit statusUpdate("Failed to derive extended key");
        QMessageBox::warning(this, "Error", "Failed to derive extended key");
    }
}

void KeysWidget::onExtKeyToECDSAButtonClicked() {
    QString extKeyStr = extKeyToECDSAInput->text();
    
    if (extKeyStr.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter an extended key");
        return;
    }
    
    emit statusUpdate("Converting extended key to ECDSA...");
    QString ecdsaKey = m_logos->accounts_module.extKeyToECDSA(extKeyStr);
    
    if (!ecdsaKey.isEmpty()) {
        extKeyToECDSAResult->setText(ecdsaKey);
        emit statusUpdate("Conversion successful");
    } else {
        extKeyToECDSAResult->setText("Conversion failed");
        emit statusUpdate("Failed to convert extended key");
        QMessageBox::warning(this, "Error", "Failed to convert extended key");
    }
}

void KeysWidget::onEcdsaToPublicKeyButtonClicked() {
    QString privateKeyHex = ecdsaToPublicKeyInput->text();
    
    if (privateKeyHex.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter an ECDSA private key");
        return;
    }
    
    emit statusUpdate("Converting ECDSA to public key...");
    QString publicKey = m_logos->accounts_module.ecdsaToPublicKey(privateKeyHex);
    
    if (!publicKey.isEmpty()) {
        ecdsaToPublicKeyResult->setText(publicKey);
        emit statusUpdate("Conversion successful");
    } else {
        ecdsaToPublicKeyResult->setText("Conversion failed");
        emit statusUpdate("Failed to convert ECDSA key");
        QMessageBox::warning(this, "Error", "Failed to convert ECDSA key");
    }
}

void KeysWidget::onPublicKeyToAddressButtonClicked() {
    QString publicKeyHex = publicKeyToAddressInput->text();
    
    if (publicKeyHex.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a public key");
        return;
    }
    
    emit statusUpdate("Converting public key to address...");
    QString address = m_logos->accounts_module.publicKeyToAddress(publicKeyHex);
    
    if (!address.isEmpty()) {
        publicKeyToAddressResult->setText(address);
        emit statusUpdate("Conversion successful");
    } else {
        publicKeyToAddressResult->setText("Conversion failed");
        emit statusUpdate("Failed to convert public key");
        QMessageBox::warning(this, "Error", "Failed to convert public key");
    }
}

