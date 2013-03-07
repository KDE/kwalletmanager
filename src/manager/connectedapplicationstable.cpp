

#include "connectedapplicationstable.h"

ConnectedApplicationsTable::ConnectedApplicationsTable(QWidget* parent): QTableView(parent)
{
}


void ConnectedApplicationsTable::resizeEvent(QResizeEvent* resizeEvent)
{
    // this will keep disconnect buttons column at it's minimum size and
    // make the application names take the reminder of the horizontal space
    resizeColumnsToContents();
    int appColumnSize = contentsRect().width() - columnWidth(1) - 50;
    setColumnWidth(0, appColumnSize);
    QAbstractItemView::resizeEvent(resizeEvent);
}
