/*

    This file is part of the KDE project.
    SPDX-FileCopyrightText: 2003-2005 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
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

void KBetterThanKDialogBase::setLabel(const QString &label)
{
    _label->setText(label);
}

KBetterThanKDialogBase::KBetterThanKDialogBase(QWidget *parent)
    : QDialog(parent), Ui_KBetterThanKDialogBase()
{
    setupUi(this);
    connect(_allowOnce, &QPushButton::clicked, this, &KBetterThanKDialogBase::clicked);
    connect(_allowAlways, &QPushButton::clicked, this, &KBetterThanKDialogBase::clicked);
    connect(_deny, &QPushButton::clicked, this, &KBetterThanKDialogBase::clicked);
    connect(_denyForever, &QPushButton::clicked, this, &KBetterThanKDialogBase::clicked);
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

