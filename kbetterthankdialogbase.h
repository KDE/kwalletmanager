// TODO insert license here
#ifndef KBETTERTHANKDIALOGBASE_H
#define KBETTERTHANKDIALOGBASE_H

#include "ui_kbetterthankdialogbase.h"


class KBetterThanKDialogBase : public QDialog, private Ui_KBetterThanKDialogBase
{
    Q_OBJECT

public:
    KBetterThanKDialogBase( QWidget* parent = 0 );
    

public slots:
    virtual void setLabel( const QString & label );
    virtual void accept();
    virtual void reject();

private slots:
    virtual void clicked();
};

#endif
