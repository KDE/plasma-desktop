/***************************************************************************
 *   Copyright (C) 2012 Aurélien Gâteau <agateau@kde.org>                  *
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

#ifndef ABSTRACTENTRY_H
#define ABSTRACTENTRY_H

#include "abstractmodel.h"

#include <QIcon>
#include <QUrl>

class AbstractEntry
{
    public:
        explicit AbstractEntry(AbstractModel *owner);
        virtual ~AbstractEntry();

        enum EntryType { RunnableType, GroupType, SeparatorType };

        virtual EntryType type() const = 0;

        AbstractModel *owner() const;

        virtual bool isValid() const;

        virtual QIcon icon() const;
        virtual QString name() const;
        virtual QString group() const;
        virtual QString description() const;

        virtual QString id() const;
        virtual QUrl url() const;

        virtual bool hasChildren() const;
        virtual AbstractModel *childModel() const;

        virtual bool hasActions() const;
        virtual QVariantList actions() const;

        virtual bool run(const QString& actionId = QString(), const QVariant &argument = QVariant());

    protected:
        AbstractModel* m_owner;
};

class AbstractGroupEntry : public AbstractEntry
{
    public:
        explicit AbstractGroupEntry(AbstractModel *owner);

        EntryType type() const override { return GroupType; }
};

class SeparatorEntry : public AbstractEntry
{
    public:
        explicit SeparatorEntry(AbstractModel *owner);

        EntryType type() const override { return SeparatorType; }
};

#endif
