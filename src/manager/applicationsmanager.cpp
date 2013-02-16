
#include "applicationsmanager.h"
#include "kwallet.h"

#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <kdebug.h>

class ConnectedAppItemDelegate : public QStyledItemDelegate
{
public:
    explicit ConnectedAppItemDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}
    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) 
    {
        QStyledItemDelegate::paint(painter, option, index);
        kDebug() << "paint";
    }
};

class ConnectedAppModel : public QAbstractTableModel
{
public:
    explicit ConnectedAppModel(KWallet::Wallet *wallet) : QAbstractTableModel(0) {
        _connectedApps = KWallet::Wallet::users(wallet->walletName());
    }

protected:
    int columnCount(const QModelIndex& parent =QModelIndex()) const { return 2; }
    int rowCount(const QModelIndex& parent =QModelIndex()) const {
        if (parent.isValid())
            return 0; // see QT QAbstractItemModel::rowCount documentation about this
        else
            return _connectedApps.count();
    }
    QVariant data(const QModelIndex& index, int role =Qt::DisplayRole) const {
        QVariant result;
        switch (index.column()) {
            case 0:
                result = QVariant(_connectedApps.at(index.row()));
                break;
            default:
                break; // nothing for the other columns
        }
        return result;
    }

private:
    QStringList _connectedApps;
};

ApplicationsManager::ApplicationsManager(QWidget* parent):
    QWidget(parent),
    _wallet(0),
    _connectedAppsModel(0)
{
    setupUi(this);
    _connectedApps->setItemDelegate(new ConnectedAppItemDelegate());
}

ApplicationsManager::~ApplicationsManager()
{
    delete _connectedAppsModel;
}

void ApplicationsManager::setWallet(KWallet::Wallet* wallet)
{
    Q_ASSERT(wallet != 0);
    _wallet = wallet;

    // create the disconnect widget menu
    _connectedApps->setModel(new ConnectedAppModel(_wallet));
}
