#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>
#include <QListWidget>
#include <QScrollArea>
#include "logos_sdk.h"

class KeystoreWidget : public QWidget {
    Q_OBJECT

public:
    explicit KeystoreWidget(LogosModules* logos, QWidget* parent = nullptr);
    ~KeystoreWidget();

signals:
    void statusUpdate(const QString& message);

private slots:
    // Keystore operations (ordered as in interface)
    void onKeystoreInitButtonClicked();
    void onKeystoreCloseButtonClicked();
    void onKeystoreListAccountsButtonClicked();
    void onKeystoreNewAccountButtonClicked();
    void onKeystoreImportButtonClicked();
    void onKeystoreExportButtonClicked();
    void onKeystoreDeleteButtonClicked();
    void onKeystoreHasAddressButtonClicked();
    void onKeystoreUnlockButtonClicked();
    void onKeystoreLockButtonClicked();
    void onKeystoreTimedUnlockButtonClicked();
    void onKeystoreUpdateButtonClicked();
    void onKeystoreSignHashButtonClicked();
    void onKeystoreSignHashWithPassphraseButtonClicked();
    void onKeystoreImportECDSAButtonClicked();
    void onKeystoreSignTxButtonClicked();
    void onKeystoreSignTxWithPassphraseButtonClicked();
    void onKeystoreFindButtonClicked();

private:
    LogosModules* m_logos;
    
    // UI elements
    QScrollArea* scrollArea;
    QWidget* scrollContent;
    QVBoxLayout* mainLayout;
    QFrame* keystoreFrame;
    
    // Keystore UI elements
    QLineEdit* keystoreDirInput;
    QLineEdit* keystoreScryptNInput;
    QLineEdit* keystoreScryptPInput;
    QPushButton* keystoreInitButton;
    QPushButton* keystoreCloseButton;
    QPushButton* keystoreListAccountsButton;
    QListWidget* keystoreAccountsList;
    QLineEdit* keystoreNewAccountPassphraseInput;
    QPushButton* keystoreNewAccountButton;
    QTextEdit* keystoreImportKeyJSONInput;
    QLineEdit* keystoreImportPassphraseInput;
    QLineEdit* keystoreImportNewPassphraseInput;
    QPushButton* keystoreImportButton;
    QLineEdit* keystoreExportAddressInput;
    QLineEdit* keystoreExportPassphraseInput;
    QLineEdit* keystoreExportNewPassphraseInput;
    QPushButton* keystoreExportButton;
    QLabel* keystoreExportResultLabel;
    QTextEdit* keystoreExportResult;
    QLineEdit* keystoreDeleteAddressInput;
    QLineEdit* keystoreDeletePassphraseInput;
    QPushButton* keystoreDeleteButton;
    QLineEdit* keystoreHasAddressInput;
    QPushButton* keystoreHasAddressButton;
    QLabel* keystoreHasAddressResultLabel;
    QLineEdit* keystoreHasAddressResult;
    QLineEdit* keystoreUnlockAddressInput;
    QLineEdit* keystoreUnlockPassphraseInput;
    QPushButton* keystoreUnlockButton;
    QLineEdit* keystoreLockAddressInput;
    QPushButton* keystoreLockButton;
    QLineEdit* keystoreTimedUnlockAddressInput;
    QLineEdit* keystoreTimedUnlockPassphraseInput;
    QLineEdit* keystoreTimedUnlockTimeoutInput;
    QPushButton* keystoreTimedUnlockButton;
    QLineEdit* keystoreUpdateAddressInput;
    QLineEdit* keystoreUpdatePassphraseInput;
    QLineEdit* keystoreUpdateNewPassphraseInput;
    QPushButton* keystoreUpdateButton;
    QLineEdit* keystoreSignHashAddressInput;
    QLineEdit* keystoreSignHashHashInput;
    QPushButton* keystoreSignHashButton;
    QLabel* keystoreSignHashResultLabel;
    QLineEdit* keystoreSignHashResult;
    QLineEdit* keystoreSignHashWithPassphraseAddressInput;
    QLineEdit* keystoreSignHashWithPassphrasePassphraseInput;
    QLineEdit* keystoreSignHashWithPassphraseHashInput;
    QPushButton* keystoreSignHashWithPassphraseButton;
    QLabel* keystoreSignHashWithPassphraseResultLabel;
    QLineEdit* keystoreSignHashWithPassphraseResult;
    QLineEdit* keystoreImportECDSAPrivateKeyInput;
    QLineEdit* keystoreImportECDSAPassphraseInput;
    QPushButton* keystoreImportECDSAButton;
    QLabel* keystoreImportECDSAResultLabel;
    QLineEdit* keystoreImportECDSAResult;
    QLineEdit* keystoreSignTxAddressInput;
    QTextEdit* keystoreSignTxTxJSONInput;
    QLineEdit* keystoreSignTxChainIDInput;
    QPushButton* keystoreSignTxButton;
    QLabel* keystoreSignTxResultLabel;
    QTextEdit* keystoreSignTxResult;
    QLineEdit* keystoreSignTxWithPassphraseAddressInput;
    QLineEdit* keystoreSignTxWithPassphrasePassphraseInput;
    QTextEdit* keystoreSignTxWithPassphraseTxJSONInput;
    QLineEdit* keystoreSignTxWithPassphraseChainIDInput;
    QPushButton* keystoreSignTxWithPassphraseButton;
    QLabel* keystoreSignTxWithPassphraseResultLabel;
    QTextEdit* keystoreSignTxWithPassphraseResult;
    QLineEdit* keystoreFindAddressInput;
    QLineEdit* keystoreFindUrlInput;
    QPushButton* keystoreFindButton;
    QLabel* keystoreFindResultLabel;
    QLineEdit* keystoreFindResult;
    
    // Helper methods
    QFrame* createSeparator();
};

