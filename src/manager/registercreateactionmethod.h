/*
   Copyright (C) 2013 Valentin Rusu <kde@rusu.info>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef REGISTERCREATEACTIONSMETHOD_H
#define REGISTERCREATEACTIONSMETHOD_H

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
class RegisterCreateActionsMethod {
public:
    typedef void (*CreateActionsMethod)(KActionCollection*);

    RegisterCreateActionsMethod(CreateActionsMethod method);

    static void createActions(KActionCollection *actionCollection);

};

#endif // REGISTERCREATEACTIONSMETHOD_H
