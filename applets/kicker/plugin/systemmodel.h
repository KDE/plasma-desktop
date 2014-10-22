/***************************************************************************
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

#ifndef SYSTEMMODEL_H
#define SYSTEMMODEL_H

#include "abstractentry.h"
#include "abstractmodel.h"

#include <kworkspace5/kdisplaymanager.h>

class SystemEntry : public AbstractEntry
{
    public:
        enum Action
        {
            LockSession,
            LogoutSession,
            SaveSession,
            NewSession,
            SuspendToRam,
            SuspendToDisk,
            Reboot,
            Shutdown
        };

        SystemEntry(Action action, const QString &name, const QString &icon);

        EntryType type() const { return RunnableType; }

        Action action() const { return m_action; }

    private:
        Action m_action;
};

class SystemModel : public AbstractModel
{
    Q_OBJECT

    public:
        explicit SystemModel(QObject *parent = 0);
        ~SystemModel();

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        int rowCount(const QModelIndex &parent = QModelIndex()) const;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument);

        int rowForFavoriteId(const QString &favoriteId);

    private:
        QList<SystemEntry *> m_entryList;
        QHash<SystemEntry::Action, QString> m_favoriteIds;
        KDisplayManager m_displayManager;
};

#endif
