/* This file is part of the KDE project

   Copyright 2008 David Faure <faure@kde.org>

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

#include "konq_popupmenuinformation.h"
#include <kfileitemlistproperties.h>
#include <kfileitem.h>

class KonqPopupMenuInformationPrivate : public QSharedData, public KFileItemListProperties /*it all moved there*/
{
public:
    KonqPopupMenuInformationPrivate()
        : m_parentWidget(0)
    {}
    QWidget* m_parentWidget;
};

KonqPopupMenuInformation::KonqPopupMenuInformation()
    : d(new KonqPopupMenuInformationPrivate)
{
}

KonqPopupMenuInformation::~KonqPopupMenuInformation()
{
}

KonqPopupMenuInformation::KonqPopupMenuInformation(const KonqPopupMenuInformation& other)
    : d(other.d)
{
}

KonqPopupMenuInformation & KonqPopupMenuInformation::operator=(const KonqPopupMenuInformation& other)
{
    d = other.d;
    return *this;
}

void KonqPopupMenuInformation::setItems(const KFileItemList& items)
{
    d->setItems(items);
}

KFileItemList KonqPopupMenuInformation::items() const
{
    return d->items();
}

KUrl::List KonqPopupMenuInformation::urlList() const
{
    return d->urlList();
}

bool KonqPopupMenuInformation::isDirectory() const
{
    return d->isDirectory();
}

void KonqPopupMenuInformation::setParentWidget(QWidget* parentWidget)
{
    d->m_parentWidget = parentWidget;
}

QWidget* KonqPopupMenuInformation::parentWidget() const
{
    return d->m_parentWidget;
}

QString KonqPopupMenuInformation::mimeType() const
{
    return d->mimeType();
}

QString KonqPopupMenuInformation::mimeGroup() const
{
    return d->mimeGroup();
}

KonqFileItemCapabilities KonqPopupMenuInformation::capabilities() const
{
    return KonqFileItemCapabilities(d->items());
}

void KonqPopupMenuInformation::setItemListProperties(const KFileItemListProperties& items)
{
    (KFileItemListProperties &)*d = items;
}

KFileItemListProperties KonqPopupMenuInformation::itemListProperties() const
{
    return *d;
}
