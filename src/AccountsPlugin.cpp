#include "AccountsPlugin.h"
#include "AccountsBackend.h"
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QFile>
#include <QFileInfo>

QWidget* AccountsPlugin::createWidget(LogosAPI* logosAPI) {
    QQuickWidget* quickWidget = new QQuickWidget();
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    qmlRegisterType<AccountsBackend>("AccountsBackend", 1, 0, "AccountsBackend");

    AccountsBackend* backend = new AccountsBackend(logosAPI, quickWidget);
    quickWidget->rootContext()->setContextProperty("backend", backend);

    // Allow QML hot reload via environment variable
    QString qmlPath = "qrc:/AccountsView.qml";
    QString envPath = qgetenv("ACCOUNTS_UI_QML_PATH");
    if (!envPath.isEmpty() && QFile::exists(envPath)) {
        qmlPath = QUrl::fromLocalFile(QFileInfo(envPath).absoluteFilePath()).toString();
    }

    quickWidget->setSource(QUrl(qmlPath));

    return quickWidget;
}

void AccountsPlugin::destroyWidget(QWidget* widget) {
    delete widget;
}
