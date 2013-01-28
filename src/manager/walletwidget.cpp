/*
   Copyright (C) 2003-2005 George Staikos <staikos@kde.org>
   Copyright (C) 2005 Isaac Clerencia <isaac@warp.es>
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

#include "walletwidget.h"
#include "allyourbase.h"
#include "kwmapeditor.h"

#include <QCheckBox>
#include <ktreewidgetsearchline.h>

WalletWidget::WalletWidget(QWidget* parent) :
    QWidget( parent ),
    _wallet(0)
{
    setupUi( this );

    _splitter->setStretchFactor(0, 1);
    _splitter->setStretchFactor(1, 2);

    _undoChanges->setIcon(KIcon( QLatin1String( "edit-undo" )));
    _saveChanges->setIcon(KIcon( QLatin1String( "document-save" )));

    QVBoxLayout *box = new QVBoxLayout(_entryListFrame);
    box->setSpacing( KDialog::spacingHint() );
    box->setMargin( KDialog::marginHint() );
    _entryList = new KWalletEntryList(_entryListFrame, "Wallet Entry List");
    _entryList->setContextMenuPolicy(Qt::CustomContextMenu);
    box->addWidget(new KTreeWidgetSearchLine(_entryListFrame, _entryList));
    box->addWidget(_entryList);

    _entryStack->setEnabled(true);

    box = new QVBoxLayout(_entryStack->widget(2));
    _mapEditorShowHide = new QCheckBox(i18n("&Show values"), _entryStack->widget(2));
    connect(_mapEditorShowHide, SIGNAL(toggled(bool)), this, SLOT(showHideMapEditorValue(bool)));
    _mapEditor = new KWMapEditor(_currentMap, _entryStack->widget(2));
    box->addWidget(_mapEditorShowHide);
    box->addWidget(_mapEditor);

//     // load splitter size
//     KConfigGroup cg(KGlobal::config(), "WalletEditor");
//     QList<int> splitterSize = cg.readEntry("SplitterSize", QList<int>());
//     if (splitterSize.size() != 2) {
//         splitterSize.clear();
//         splitterSize.append(_splitter->width()/2);
//         splitterSize.append(_splitter->width()/2);
//     }
//     _splitter->setSizes(splitterSize);
}

void WalletWidget::forgetWallet()
{
    _wallet = 0;
}

void WalletWidget::startWalletEditing(KWallet::Wallet* wallet)
{
    _wallet = wallet;
}
