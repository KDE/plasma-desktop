/***************************************************************************
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "abstractentry.h"

#include <QDebug>

AbstractEntry::AbstractEntry(AbstractModel *owner)
: m_owner(owner)
{
}

AbstractEntry::~AbstractEntry()
{
}

AbstractModel *AbstractEntry::owner() const
{
    return m_owner;
}

bool AbstractEntry::isValid() const
{
    return true;
}

QIcon AbstractEntry::icon() const
{
    return QIcon();
}

QString AbstractEntry::name() const
{
    return QString();
}


QString AbstractEntry::group() const
{
    return QString();
}

QString AbstractEntry::description() const
{
    return QString();
}

QString AbstractEntry::id() const
{
    return QString();
}

QUrl AbstractEntry::url() const
{
    return QUrl();
}

bool AbstractEntry::hasChildren() const
{
    return false;
}

AbstractModel *AbstractEntry::childModel() const
{
    return nullptr;
}

bool AbstractEntry::hasActions() const
{
    return false;
}

QVariantList AbstractEntry::actions() const
{
    return QVariantList();
}

bool AbstractEntry::run(const QString& actionId, const QVariant &argument)
{
    Q_UNUSED(actionId)
    Q_UNUSED(argument)

    return false;
}

AbstractGroupEntry::AbstractGroupEntry(AbstractModel *owner) : AbstractEntry(owner)
{
}

SeparatorEntry::SeparatorEntry(AbstractModel *owner) : AbstractEntry(owner)
{
}
