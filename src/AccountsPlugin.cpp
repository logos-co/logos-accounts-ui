#include "AccountsPlugin.h"
#include "AccountsBackend.h"

#include <QDebug>

AccountsPlugin::AccountsPlugin(QObject* parent)
    : QObject(parent)
{
}

AccountsPlugin::~AccountsPlugin() = default;

void AccountsPlugin::initLogos(LogosAPI* api)
{
    if (m_backend) return;
    m_backend = new AccountsBackend(api, this);
    setBackend(m_backend);
    qDebug() << "AccountsPlugin: backend initialized";
}
