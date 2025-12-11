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

class ExtKeystoreWidget : public QWidget {
    Q_OBJECT

public:
    explicit ExtKeystoreWidget(LogosModules* logos, QWidget* parent = nullptr);
    ~ExtKeystoreWidget();

signals:
    void statusUpdate(const QString& message);

private slots:
    // Extended keystore operations (ordered as in interface)
    void onExtKeystoreInitButtonClicked();
    void onExtKeystoreCloseButtonClicked();
    void onExtKeystoreListAccountsButtonClicked();
    void onExtKeystoreNewAccountButtonClicked();
    void onExtKeystoreImportButtonClicked();
    void onExtKeystoreImportExtendedKeyButtonClicked();
    void onExtKeystoreExportExtButtonClicked();
    void onExtKeystoreExportPrivButtonClicked();
    void onExtKeystoreDeleteButtonClicked();
    void onExtKeystoreHasAddressButtonClicked();
    void onExtKeystoreUnlockButtonClicked();
    void onExtKeystoreLockButtonClicked();
    void onExtKeystoreTimedUnlockButtonClicked();
    void onExtKeystoreUpdateButtonClicked();
    void onExtKeystoreSignHashButtonClicked();
    void onExtKeystoreSignHashWithPassphraseButtonClicked();
    void onExtKeystoreSignTxButtonClicked();
    void onExtKeystoreSignTxWithPassphraseButtonClicked();
    void onExtKeystoreDeriveButtonClicked();
    void onExtKeystoreDeriveWithPassphraseButtonClicked();
    void onExtKeystoreFindButtonClicked();

private:
    LogosModules* m_logos;
    
    // UI elements
    QScrollArea* scrollArea;
    QWidget* scrollContent;
    QVBoxLayout* mainLayout;
    QFrame* extKeystoreFrame;
    
    // Extended keystore UI elements
    QLineEdit* extKeystoreDirInput;
    QLineEdit* extKeystoreScryptNInput;
    QLineEdit* extKeystoreScryptPInput;
    QPushButton* extKeystoreInitButton;
    QPushButton* extKeystoreCloseButton;
    QPushButton* extKeystoreListAccountsButton;
    QListWidget* extKeystoreAccountsList;
    QLineEdit* extKeystoreNewAccountPassphraseInput;
    QPushButton* extKeystoreNewAccountButton;
    QTextEdit* extKeystoreImportKeyJSONInput;
    QLineEdit* extKeystoreImportPassphraseInput;
    QLineEdit* extKeystoreImportNewPassphraseInput;
    QPushButton* extKeystoreImportButton;
    QLineEdit* extKeystoreImportExtendedKeyInput;
    QLineEdit* extKeystoreImportExtendedKeyPassphraseInput;
    QPushButton* extKeystoreImportExtendedKeyButton;
    QLabel* extKeystoreImportExtendedKeyResultLabel;
    QLineEdit* extKeystoreImportExtendedKeyResult;
    QLineEdit* extKeystoreExportExtAddressInput;
    QLineEdit* extKeystoreExportExtPassphraseInput;
    QLineEdit* extKeystoreExportExtNewPassphraseInput;
    QPushButton* extKeystoreExportExtButton;
    QLabel* extKeystoreExportExtResultLabel;
    QTextEdit* extKeystoreExportExtResult;
    QLineEdit* extKeystoreExportPrivAddressInput;
    QLineEdit* extKeystoreExportPrivPassphraseInput;
    QLineEdit* extKeystoreExportPrivNewPassphraseInput;
    QPushButton* extKeystoreExportPrivButton;
    QLabel* extKeystoreExportPrivResultLabel;
    QTextEdit* extKeystoreExportPrivResult;
    QLineEdit* extKeystoreDeleteAddressInput;
    QLineEdit* extKeystoreDeletePassphraseInput;
    QPushButton* extKeystoreDeleteButton;
    QLineEdit* extKeystoreHasAddressInput;
    QPushButton* extKeystoreHasAddressButton;
    QLabel* extKeystoreHasAddressResultLabel;
    QLineEdit* extKeystoreHasAddressResult;
    QLineEdit* extKeystoreUnlockAddressInput;
    QLineEdit* extKeystoreUnlockPassphraseInput;
    QPushButton* extKeystoreUnlockButton;
    QLineEdit* extKeystoreLockAddressInput;
    QPushButton* extKeystoreLockButton;
    QLineEdit* extKeystoreTimedUnlockAddressInput;
    QLineEdit* extKeystoreTimedUnlockPassphraseInput;
    QLineEdit* extKeystoreTimedUnlockTimeoutInput;
    QPushButton* extKeystoreTimedUnlockButton;
    QLineEdit* extKeystoreUpdateAddressInput;
    QLineEdit* extKeystoreUpdatePassphraseInput;
    QLineEdit* extKeystoreUpdateNewPassphraseInput;
    QPushButton* extKeystoreUpdateButton;
    QLineEdit* extKeystoreSignHashAddressInput;
    QLineEdit* extKeystoreSignHashHashInput;
    QPushButton* extKeystoreSignHashButton;
    QLabel* extKeystoreSignHashResultLabel;
    QLineEdit* extKeystoreSignHashResult;
    QLineEdit* extKeystoreSignHashWithPassphraseAddressInput;
    QLineEdit* extKeystoreSignHashWithPassphrasePassphraseInput;
    QLineEdit* extKeystoreSignHashWithPassphraseHashInput;
    QPushButton* extKeystoreSignHashWithPassphraseButton;
    QLabel* extKeystoreSignHashWithPassphraseResultLabel;
    QLineEdit* extKeystoreSignHashWithPassphraseResult;
    QLineEdit* extKeystoreSignTxAddressInput;
    QTextEdit* extKeystoreSignTxTxJSONInput;
    QLineEdit* extKeystoreSignTxChainIDInput;
    QPushButton* extKeystoreSignTxButton;
    QLabel* extKeystoreSignTxResultLabel;
    QTextEdit* extKeystoreSignTxResult;
    QLineEdit* extKeystoreSignTxWithPassphraseAddressInput;
    QLineEdit* extKeystoreSignTxWithPassphrasePassphraseInput;
    QTextEdit* extKeystoreSignTxWithPassphraseTxJSONInput;
    QLineEdit* extKeystoreSignTxWithPassphraseChainIDInput;
    QPushButton* extKeystoreSignTxWithPassphraseButton;
    QLabel* extKeystoreSignTxWithPassphraseResultLabel;
    QTextEdit* extKeystoreSignTxWithPassphraseResult;
    QLineEdit* extKeystoreDeriveAddressInput;
    QLineEdit* extKeystoreDerivePathInput;
    QLineEdit* extKeystoreDerivePinInput;
    QPushButton* extKeystoreDeriveButton;
    QLabel* extKeystoreDeriveResultLabel;
    QLineEdit* extKeystoreDeriveResult;
    QLineEdit* extKeystoreDeriveWithPassphraseAddressInput;
    QLineEdit* extKeystoreDeriveWithPassphrasePathInput;
    QLineEdit* extKeystoreDeriveWithPassphrasePinInput;
    QLineEdit* extKeystoreDeriveWithPassphrasePassphraseInput;
    QLineEdit* extKeystoreDeriveWithPassphraseNewPassphraseInput;
    QPushButton* extKeystoreDeriveWithPassphraseButton;
    QLabel* extKeystoreDeriveWithPassphraseResultLabel;
    QLineEdit* extKeystoreDeriveWithPassphraseResult;
    QLineEdit* extKeystoreFindAddressInput;
    QLineEdit* extKeystoreFindUrlInput;
    QPushButton* extKeystoreFindButton;
    QLabel* extKeystoreFindResultLabel;
    QLineEdit* extKeystoreFindResult;
    
    // Helper methods
    QFrame* createSeparator();
};

