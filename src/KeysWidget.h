#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include "logos_sdk.h"

class KeysWidget : public QWidget {
    Q_OBJECT

public:
    explicit KeysWidget(LogosModules* logos, QWidget* parent = nullptr);
    ~KeysWidget();

signals:
    void statusUpdate(const QString& message);

private slots:
    // Mnemonic operations
    void onCreateMnemonicButtonClicked();
    void onLengthToEntropyStrengthButtonClicked();
    
    // Key operations
    void onCreateExtKeyFromMnemonicButtonClicked();
    void onDeriveExtKeyButtonClicked();
    void onExtKeyToECDSAButtonClicked();
    void onEcdsaToPublicKeyButtonClicked();
    void onPublicKeyToAddressButtonClicked();

private:
    LogosModules* m_logos;
    
    // UI elements
    QScrollArea* scrollArea;
    QWidget* scrollContent;
    QVBoxLayout* mainLayout;
    
    // Mnemonic UI elements
    QFrame* mnemonicFrame;
    QLineEdit* mnemonicLengthInput;
    QPushButton* createMnemonicButton;
    QLabel* mnemonicResultLabel;
    QTextEdit* mnemonicResult;
    QLineEdit* lengthToEntropyStrengthInput;
    QPushButton* lengthToEntropyStrengthButton;
    QLabel* lengthToEntropyStrengthResultLabel;
    QLineEdit* lengthToEntropyStrengthResult;
    
    // Key operations UI elements
    QFrame* keyOpsFrame;
    QTextEdit* createExtKeyMnemonicInput;
    QLineEdit* createExtKeyPassphraseInput;
    QPushButton* createExtKeyFromMnemonicButton;
    QLabel* createExtKeyResultLabel;
    QTextEdit* createExtKeyResult;
    QLineEdit* deriveExtKeyInput;
    QLineEdit* deriveExtKeyPathInput;
    QPushButton* deriveExtKeyButton;
    QLabel* deriveExtKeyResultLabel;
    QTextEdit* deriveExtKeyResult;
    QLineEdit* extKeyToECDSAInput;
    QPushButton* extKeyToECDSAButton;
    QLabel* extKeyToECDSAResultLabel;
    QLineEdit* extKeyToECDSAResult;
    QLineEdit* ecdsaToPublicKeyInput;
    QPushButton* ecdsaToPublicKeyButton;
    QLabel* ecdsaToPublicKeyResultLabel;
    QLineEdit* ecdsaToPublicKeyResult;
    QLineEdit* publicKeyToAddressInput;
    QPushButton* publicKeyToAddressButton;
    QLabel* publicKeyToAddressResultLabel;
    QLineEdit* publicKeyToAddressResult;
    
    // Helper methods
    QFrame* createSeparator();
};

