/*
    SPDX-FileCopyrightText: 2024 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "clipboardutils.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QString>
#include <QStringLiteral>

void setPasswordToClipboard(const QString &password)
{
    auto *mimeData = new QMimeData;
    mimeData->setText(password);
    setPasswordHintToMimeData(mimeData);
    QApplication::clipboard()->setMimeData(mimeData);
}

void setPasswordHintToMimeData(QMimeData *mimeData)
{
    if (!mimeData) {
        return;
    }
    mimeData->setData(QStringLiteral("x-kde-passwordManagerHint"), "secret");
}
