/*
    SPDX-FileCopyrightText: 2024 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLIPBOARDUTILS_H
#define CLIPBOARDUTILS_H

#include <QMimeData>
#include <QString>

void setPasswordToClipboard(const QString &password);
void setPasswordHintToMimeData(QMimeData *mimeData);

#endif
