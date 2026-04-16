#ifndef ACCOUNTS_PLUGIN_H
#define ACCOUNTS_PLUGIN_H

#include <QObject>
#include <QString>
#include <QtPlugin>          // for Q_PLUGIN_METADATA, Q_INTERFACES
#include "AccountsInterface.h"
#include "LogosViewPluginBase.h"

class LogosAPI;
class AccountsBackend;

// Thin plugin entry point. Holds an AccountsBackend and lets the
// generated view-plugin base expose it to ui-host.
class AccountsPlugin : public QObject,
                       public AccountsInterface,
                       public AccountsBackendViewPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AccountsInterface_iid FILE "../metadata.json")
    Q_INTERFACES(AccountsInterface)

public:
    explicit AccountsPlugin(QObject* parent = nullptr);
    ~AccountsPlugin() override;

    QString name()    const override { return "accounts_ui"; }
    QString version() const override { return "1.0.0"; }

    // Called by ui-host after plugin load. Creates the backend and wires
    // it up with the provided LogosAPI.
    Q_INVOKABLE void initLogos(LogosAPI* api);

private:
    AccountsBackend* m_backend = nullptr;
};

#endif
