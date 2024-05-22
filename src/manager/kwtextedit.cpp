/*
    SPDX-FileCopyrightText: 2024 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwtextedit.h"
#include "clipboardutils.h"
#include <QTextEdit>
#include <QWidget>

KWTextEdit::KWTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
}

QMimeData *KWTextEdit::createMimeDataFromSelection() const
{
    QMimeData *mimeData = QTextEdit::createMimeDataFromSelection();
    setPasswordHintToMimeData(mimeData);
    return mimeData;
}

#include "moc_kwtextedit.cpp"
