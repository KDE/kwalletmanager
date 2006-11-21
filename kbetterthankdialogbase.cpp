#include "kbetterthankdialogbase.h"

void KBetterThanKDialogBase::clicked()
{
    if (sender() == _allowOnce) {
	done(3);
    } else if (sender() == _allowAlways) {
	done(1);
    } else if (sender() == _deny) {
	done(4);
    } else if (sender() == _denyForever) {
	done(2);
    }
}


void KBetterThanKDialogBase::setLabel( const QString & label )
{
    _label->setText(label);
}


KBetterThanKDialogBase::KBetterThanKDialogBase( QWidget* parent )
    : QDialog( parent ), Ui_KBetterThanKDialogBase()
{
    setupUi( this );
    connect(_allowOnce, SIGNAL(clicked()), this, SLOT(clicked()));
    connect(_allowAlways, SIGNAL(clicked()), this, SLOT(clicked()));
    connect(_deny,SIGNAL(clicked()), this, SLOT(clicked()));
    connect(_denyForever,SIGNAL(clicked()), this, SLOT(clicked()));
    _allowOnce->setFocus();
}


void KBetterThanKDialogBase::accept()
{
    setResult(3);
}


void KBetterThanKDialogBase::reject()
{
    QDialog::reject();
    setResult(4);
}
#include "kbetterthankdialogbase.moc"
