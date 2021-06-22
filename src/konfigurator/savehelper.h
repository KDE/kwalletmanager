/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _SAVEHELPER_H_
#define _SAVEHELPER_H_

#include <KAuth>

using namespace KAuth;

class SaveHelper : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    KAuth::ActionReply save(const QVariantMap &args);
};

#endif // _SAVEHELPER_H_
