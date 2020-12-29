/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "savehelper.h"

#include <KAuthHelperSupport>

#include <QCoreApplication>
#include <QDebug>

ActionReply SaveHelper::save(const QVariantMap &args)
{
    Q_UNUSED(args);
    const qint64 uid = QCoreApplication::applicationPid();
    qDebug() << "executing uid=" << uid;

    return ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.kde.kcontrol.kcmkwallet5", SaveHelper)
