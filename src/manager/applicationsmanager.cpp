
#include "applicationsmanager.h"
#include "kwallet.h"

ApplicationsManager::ApplicationsManager(QWidget* parent):
    QWidget(parent),
    _wallet(0)
{
    setupUi(this);
}

void ApplicationsManager::setWallet(KWallet::Wallet* wallet)
{

}
