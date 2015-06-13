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

#include "registercreateactionmethod.h"

Q_GLOBAL_STATIC(std::list<RegisterCreateActionsMethod::CreateActionsMethod>, createActionMethodList)

RegisterCreateActionsMethod::RegisterCreateActionsMethod(RegisterCreateActionsMethod::CreateActionsMethod method)
{
    createActionMethodList->push_back(method);
}

void RegisterCreateActionsMethod::createActions(KActionCollection *actionCollection)
{
    std::list<CreateActionsMethod>::const_iterator it = createActionMethodList->begin();
    std::list<CreateActionsMethod>::const_iterator end = createActionMethodList->end();
    for (; it != end; it++) {
        (*it)(actionCollection);
    }
}
