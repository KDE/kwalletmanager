/*
    SPDX-FileCopyrightText: 2024 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWTEXTEDIT_H
#define KWTEXTEDIT_H

#include <QTextEdit>
#include <QWidget>

class KWTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit KWTextEdit(QWidget *parent = nullptr);

protected:
    QMimeData *createMimeDataFromSelection() const override;
};

#endif // KWTEXTEDIT_H
