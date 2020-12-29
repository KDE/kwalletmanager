/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
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
