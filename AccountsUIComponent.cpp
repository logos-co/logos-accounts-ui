#include "AccountsUIComponent.h"
#include "src/AccountsWidget.h"

QWidget* AccountsUIComponent::createWidget(LogosAPI* logosAPI) {
    // LogosAPI parameter available but not used - AccountsWidget creates its own
    return new AccountsWidget();
}

void AccountsUIComponent::destroyWidget(QWidget* widget) {
    delete widget;
}
