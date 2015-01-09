/***************************************************************************
 *   Copyright (C) 2012 Aurélien Gâteau <agateau@kde.org>                  *
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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

#ifndef ABSTRACTENTRY_H
#define ABSTRACTENTRY_H

#include "abstractmodel.h"

#include <QIcon>
#include <QPointer>
#include <QString>

class AbstractEntry
{
    public:
        enum EntryType { RunnableType, GroupType };

        virtual ~AbstractEntry();

        virtual EntryType type() const = 0;

        QIcon icon() const { return m_icon; }
        QString iconName() const { return m_icon.name(); }
        QString name() const { return m_name; }

    protected:
        QIcon m_icon;
        QString m_name;
};

class AbstractGroupEntry : public AbstractEntry
{
    public:
        EntryType type() const { return GroupType; }

        AbstractModel *model() const { return m_model; }

    protected:
        QPointer<AbstractModel> m_model;
};

#endif
