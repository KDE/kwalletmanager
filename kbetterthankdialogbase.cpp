/*
 *
 * This file is part of the KDE project.
 * Copyright (C) 2003-2005 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


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
