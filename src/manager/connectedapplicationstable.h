
#ifndef CONNECTEDAPPLICATIONSTABLE_H
#define CONNECTEDAPPLICATIONSTABLE_H
#include <QTableView>

class ConnectedApplicationsTable : public QTableView
{
    Q_OBJECT
public:
    explicit ConnectedApplicationsTable(QWidget *parent);

protected:
    virtual void resizeEvent(QResizeEvent *resizeEvent);
};

#endif // CONNECTEDAPPLICATIONSTABLE_H
