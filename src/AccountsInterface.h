#ifndef ACCOUNTS_INTERFACE_H
#define ACCOUNTS_INTERFACE_H

#include <QtPlugin>          // for Q_DECLARE_INTERFACE
#include "interface.h"

class AccountsInterface : public PluginInterface
{
public:
    virtual ~AccountsInterface() = default;
};

#define AccountsInterface_iid "org.logos.AccountsInterface"
Q_DECLARE_INTERFACE(AccountsInterface, AccountsInterface_iid)

#endif
