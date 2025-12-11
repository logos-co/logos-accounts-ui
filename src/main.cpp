#include <QApplication>
#include <QDebug>
#include "AccountsWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    qDebug() << "Starting Accounts Qt Application";
    
    // Create and show the main window
    AccountsWindow mainWindow;
    mainWindow.setWindowTitle("Logos Accounts App");
    mainWindow.resize(800, 600);
    mainWindow.show();
    
    return app.exec();
} 