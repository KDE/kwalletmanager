/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REGISTERCREATEACTIONMETHOD_H
#define REGISTERCREATEACTIONMETHOD_H

#include <QList>
#include <list>

class KActionCollection;

/**
 * This class is intended to avoid coupling between the main window and it's
 * contained widgets that handle actions.
 *
 * The main window it's a KXMLGuiWindow, that needs to have all the actions
 * created before the createGui method call. That would require for this class
 * to have knowledge for the actions of the child widgets, which is not very OO
 * wise.
 *
 * To avoid that, the main window will call createActions of this class that will
 * in turn delegate to the registered CreateActionsMethod's registered here.
 */
class RegisterCreateActionsMethod
{
public:
    using CreateActionsMethod = void (*)(KActionCollection *);

    explicit RegisterCreateActionsMethod(CreateActionsMethod method);

    static void createActions(KActionCollection *actionCollection);

};

#endif // REGISTERCREATEACTIONMETHOD_H
