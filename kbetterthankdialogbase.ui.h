/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/


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


void KBetterThanKDialogBase::init()
{
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
