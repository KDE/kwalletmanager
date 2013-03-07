
#include "applicationsmanager.h"
#include "kwallet.h"

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStandardItemModel>
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

class DisconnectAppItem : public QStandardItem {
public:
    DisconnectAppItem(const QString& appName, KWallet::Wallet *wallet) :
            _appName(appName), _wallet(wallet) {
        setText(tr2i18n("Disconnect"));
        setEditable(false);
        setSelectable(false);
    }
private:
    QString             _appName;
    KWallet::Wallet     *_wallet;
};

class ConnectedAppModel : public QStandardItemModel
{
public:
    explicit ConnectedAppModel(KWallet::Wallet *wallet) : QStandardItemModel() {
        _connectedApps = KWallet::Wallet::users(wallet->walletName());
        int row =0;
        Q_FOREACH(QString appName, _connectedApps ) {
            // for un unknown reason, kwalletd returs empty strings so lets avoid inserting them
            // FIXME: find out why kwalletd returns empty strings here
            if (appName.length()>0) {
                QStandardItem *item = new QStandardItem(appName);
                item->setEditable(false);
                setItem(row, 0, item);
                setItem(row, 1, new DisconnectAppItem(appName, wallet));
                row++;
            }
        }
    }

protected:
//     virtual int columnCount(const QModelIndex& parent =QModelIndex()) const { return 1; }
//     virtual int rowCount(const QModelIndex& parent =QModelIndex()) const {
//         if (parent.isValid())
//             return 0; // see QT QAbstractItemModel::rowCount documentation about this
//         else
//             return _connectedApps.count();
//     }
//     QVariant data(const QModelIndex& index, int role =Qt::DisplayRole) const {
//         QVariant result;
//         QString appName;
//         switch (index.column()) {
//             case 0:
//                 appName = _connectedApps.at(index.row());
//                 result = appName;
//                 break;
//             default:
//                 break; // nothing for the other columns
//         }
//         return result;
//     }

private:
    QStringList _connectedApps;
};

ApplicationsManager::ApplicationsManager(QWidget* parent):
    QWidget(parent),
    _wallet(0),
    _connectedAppsModel(0)
{
    setupUi(this);
//    _connectedApps->setItemDelegate(new ConnectedAppItemDelegate());
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
