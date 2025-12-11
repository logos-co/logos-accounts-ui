#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include "AccountsWidget.h"

class AccountsWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit AccountsWindow(QWidget* parent = nullptr);
    ~AccountsWindow();

private slots:
    void onAboutAction();
    
private:
    void setupMenu();
    void setupStatusBar();
    
    AccountsWidget* accountsWidget;
    QStatusBar* statusBar;
}; 