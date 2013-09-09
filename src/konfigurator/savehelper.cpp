
#include "savehelper.h"

#include <kdebug.h>
#include <unistd.h>

ActionReply SaveHelper::save(QVariantMap args)
{
    __uid_t uid = getuid();
    kDebug() << "executing uid=" << uid;

    return ActionReply::SuccessReply;
}

KDE4_AUTH_HELPER_MAIN("org.kde.kcontrol.kcmkwallet", SaveHelper)
