#include "AccountsWindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDebug>

AccountsWindow::AccountsWindow(QWidget* parent) : QMainWindow(parent) {
    // Create central widget
    accountsWidget = new AccountsWidget(this);
    setCentralWidget(accountsWidget);
    
    // Setup the UI
    setupMenu();
    setupStatusBar();
    
    // Set window properties
    setMinimumSize(640, 480);
}

AccountsWindow::~AccountsWindow() {
    // AccountsWidget will be deleted automatically as it's a child of this window
}

void AccountsWindow::setupMenu() {
    // Create main menu
    QMenu* fileMenu = menuBar()->addMenu("&File");
    //QMenu* wakuMenu = menuBar()->addMenu("&Waku");
    QMenu* helpMenu = menuBar()->addMenu("&Help");
    
    // File menu actions
    QAction* exitAction = fileMenu->addAction("E&xit");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // Help menu actions
    QAction* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, &AccountsWindow::onAboutAction);
}

void AccountsWindow::setupStatusBar() {
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Ready");
}

void AccountsWindow::onAboutAction() {
    QMessageBox::about(this, "About Logos Accounts",
                     "Logos Accounts Application\n\n"
                     "A sample Qt application demonstrating Accounts module integration.");
}
