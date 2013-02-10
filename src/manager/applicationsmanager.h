
#ifndef APPLICATIONSMANAGER_H
#define APPLICATIONSMANAGER_H

#include "ui_applicationsmanager.h"
#include <QWidget>

namespace KWallet {
class Wallet;
}

class ApplicationsManager : public QWidget, public Ui::ApplicationsManager
{
    Q_OBJECT
public:
    ApplicationsManager(QWidget *parent);

    void setWallet(KWallet::Wallet *wallet);

private:
    KWallet::Wallet     *_wallet;
};

#endif // APPLICATIONSMANAGER_H
