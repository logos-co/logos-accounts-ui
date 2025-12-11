#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QFrame>
#include "logos_api.h"
#include "logos_api_client.h"
#include "logos_sdk.h"

class KeysWidget;
class KeystoreWidget;
class ExtKeystoreWidget;

class AccountsWidget : public QWidget {
    Q_OBJECT

public:
    explicit AccountsWidget(QWidget* parent = nullptr);
    ~AccountsWidget();

private:
    // UI elements
    QVBoxLayout* mainLayout;
    QTabWidget* tabWidget;
    QLabel* statusLabel;
    
    // Widget instances
    KeysWidget* keysWidget;
    KeystoreWidget* keystoreWidget;
    ExtKeystoreWidget* extKeystoreWidget;

    // LogosAPI instance for remote method calls
    LogosAPI* m_logosAPI;
    LogosModules* logos;
    
    // Helper methods
    void updateStatus(const QString& message);
};
