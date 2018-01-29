/***************************************************************************
 *   Copyright (C) 201 by Eike Hein <hein@kde.org>                         *
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

#ifndef CONTACTENTRY_H
#define CONTACTENTRY_H

#include "abstractentry.h"

namespace KPeople {
    class PersonData;
}

class ContactEntry : public AbstractEntry
{
    public:
        explicit ContactEntry(AbstractModel *owner, const QString &id);

        EntryType type() const override { return RunnableType; }

        bool isValid() const override;

        QIcon icon() const override;
        QString name() const override;

        QString id() const override;
        QUrl url() const override;

        bool hasActions() const override;
        QVariantList actions() const override;

        bool run(const QString& actionId = QString(), const QVariant &argument = QVariant()) override;

        static void showPersonDetailsDialog(const QString &id);

    private:
        KPeople::PersonData *m_personData;
};

#endif
