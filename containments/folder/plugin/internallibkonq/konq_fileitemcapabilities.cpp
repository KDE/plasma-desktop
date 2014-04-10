/* This file is part of the KDE project
   Copyright (C) 2008 by Peter Penz <peter.penz19@gmail.com>
   Copyright (C) 2008 by George Goldberg <grundleborg@googlemail.com>
   Copyright     2009 David Faure <faure@kde.org>

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

#include "konq_fileitemcapabilities.h"
#include <kfileitemlistproperties.h>
#include <kfileitem.h>

class KonqFileItemCapabilitiesPrivate : public QSharedData, public KFileItemListProperties /*it all moved there*/
{
public:
    KonqFileItemCapabilitiesPrivate()
    { }
};


KonqFileItemCapabilities::KonqFileItemCapabilities()
    : d(new KonqFileItemCapabilitiesPrivate)
{
}

KonqFileItemCapabilities::KonqFileItemCapabilities(const KFileItemList& items)
    : d(new KonqFileItemCapabilitiesPrivate)
{
    setItems(items);
}

void KonqFileItemCapabilities::setItems(const KFileItemList& items)
{
    d->setItems(items);
}

KonqFileItemCapabilities::KonqFileItemCapabilities(const KonqFileItemCapabilities& other)
    : d(other.d)
{ }


KonqFileItemCapabilities& KonqFileItemCapabilities::operator=(const KonqFileItemCapabilities& other)
{
    d = other.d;
    return *this;
}

KonqFileItemCapabilities::~KonqFileItemCapabilities()
{
}

bool KonqFileItemCapabilities::supportsReading() const
{
    return d->supportsReading();
}

bool KonqFileItemCapabilities::supportsDeleting() const
{
    return d->supportsDeleting();
}

bool KonqFileItemCapabilities::supportsWriting() const
{
    return d->supportsWriting();
}

bool KonqFileItemCapabilities::supportsMoving() const
{
    return d->supportsMoving();
}

bool KonqFileItemCapabilities::isLocal() const
{
    return d->isLocal();
}
