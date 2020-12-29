/*

    This file is part of the KDE project.
    SPDX-FileCopyrightText: 2003-2005 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KBETTERTHANKDIALOGBASE_H
#define KBETTERTHANKDIALOGBASE_H

#include "ui_kbetterthankdialogbase.h"

class KBetterThanKDialogBase : public QDialog, private Ui_KBetterThanKDialogBase
{
    Q_OBJECT

public:
    explicit KBetterThanKDialogBase(QWidget *parent = nullptr);

public Q_SLOTS:
    virtual void setLabel(const QString &label);
    void accept() override;
    void reject() override;

private Q_SLOTS:
    virtual void clicked();
};

#endif
