/* This file is part of the KDE project
   Copyright (C) 1998-2009 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "konq_menuactions.h"
#include <kfileitemlistproperties.h>
#include <kfileitemactions.h>
#include "konq_popupmenuinformation.h"

KonqMenuActions::KonqMenuActions()
    : d(new KFileItemActions)
{
}


KonqMenuActions::~KonqMenuActions()
{
    delete d;
}

void KonqMenuActions::setPopupMenuInfo(const KonqPopupMenuInformation& info)
{
    d->setItemListProperties(info.itemListProperties());
}

void KonqMenuActions::setItemListProperties(const KFileItemListProperties& itemListProperties)
{
    d->setItemListProperties(itemListProperties);
}

int KonqMenuActions::addActionsTo(QMenu* mainMenu)
{
    return d->addServiceActionsTo(mainMenu);
}

void KonqMenuActions::addOpenWithActionsTo(QMenu* topMenu, const QString& traderConstraint)
{
    d->addOpenWithActionsTo(topMenu, traderConstraint);
}

QAction* KonqMenuActions::preferredOpenWithAction(const QString& traderConstraint)
{
    return d->preferredOpenWithAction(traderConstraint);
}

void KonqMenuActions::setParentWidget(QWidget* widget)
{
    d->setParentWidget(widget);
}
