
#ifndef APPLICATIONSMANAGER_H
#define APPLICATIONSMANAGER_H

#include "ui_applicationsmanager.h"
#include <QWidget>

namespace KWallet {
class Wallet;
}

class QStringListModel;

class ApplicationsManager : public QWidget, public Ui::ApplicationsManager
{
    Q_OBJECT
public:
    ApplicationsManager(QWidget *parent);
    virtual ~ApplicationsManager();

    void setWallet(KWallet::Wallet *wallet);

private:
    KWallet::Wallet     *_wallet;
    QStringListModel    *_connectedAppsModel;
};

#endif // APPLICATIONSMANAGER_H
