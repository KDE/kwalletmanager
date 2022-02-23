/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _SAVEHELPER_H_
#define _SAVEHELPER_H_

#include <QObject>

#include <kauth_version.h>
#if KAUTH_VERSION >= QT_VERSION_CHECK(5, 92, 0)
#include <KAuth/ActionReply>
#else
#include <KAuth>
#endif

using namespace KAuth;

class SaveHelper : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    KAuth::ActionReply save(const QVariantMap &args);
};

#endif // _SAVEHELPER_H_
