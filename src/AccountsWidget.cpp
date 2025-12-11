#include "AccountsWidget.h"
#include "KeysWidget.h"
#include "KeystoreWidget.h"
#include "ExtKeystoreWidget.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonValue>
#include <QDir>
#include <iostream>
#include <csignal>
#include <QTimer>
#include "logos_api_client.h"

// Static pointer to the active AccountsWidget for callbacks
static AccountsWidget* activeWidget = nullptr;

AccountsWidget::AccountsWidget(QWidget* parent) 
    : QWidget(parent), 
      m_logosAPI(nullptr) {
    
    // Set as the active widget
    activeWidget = this;
    
    m_logosAPI = new LogosAPI("core", this);
    logos = new LogosModules(m_logosAPI);
    
    // Main vertical layout
    mainLayout = new QVBoxLayout(this);
    
    // Create status frame
    QFrame* statusFrame = new QFrame(this);
    QVBoxLayout* statusLayout = new QVBoxLayout(statusFrame);
    statusFrame->setLayout(statusLayout);
    statusFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    statusFrame->setLineWidth(1);

    // Create status label
    statusLabel = new QLabel("Status: Not initialized", this);
    statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // Add status label to layout
    statusLayout->addWidget(statusLabel);
    
    // Create tab widget
    tabWidget = new QTabWidget(this);
    
    // Create the three widget instances
    keysWidget = new KeysWidget(logos, this);
    keystoreWidget = new KeystoreWidget(logos, this);
    extKeystoreWidget = new ExtKeystoreWidget(logos, this);
    
    // Connect status update signals
    connect(keysWidget, &KeysWidget::statusUpdate, this, &AccountsWidget::updateStatus);
    connect(keystoreWidget, &KeystoreWidget::statusUpdate, this, &AccountsWidget::updateStatus);
    connect(extKeystoreWidget, &ExtKeystoreWidget::statusUpdate, this, &AccountsWidget::updateStatus);
    
    // Add tabs to tab widget
    tabWidget->addTab(keysWidget, "Keys");
    tabWidget->addTab(keystoreWidget, "Keystore");
    tabWidget->addTab(extKeystoreWidget, "ExtKeystore");
    
    // Add status frame and tab widget to main layout
    mainLayout->addWidget(statusFrame);
    mainLayout->addWidget(tabWidget);

    // Set spacing and margins
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);
}

AccountsWidget::~AccountsWidget() {
    // Reset the active widget if it's this instance
    if (activeWidget == this) {
        activeWidget = nullptr;
    }
}

void AccountsWidget::updateStatus(const QString& message) {
    statusLabel->setText("Status: " + message);
    qDebug() << message;
}
