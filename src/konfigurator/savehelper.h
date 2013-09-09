
#ifndef _SAVEHELPER_H_
#define _SAVEHELPER_H_

#include <kauth.h>

using namespace KAuth;

class SaveHelper : public QObject {
    Q_OBJECT;
public slots:
    ActionReply save(QVariantMap args);
};

#endif // _SAVEHELPER_H_
