/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                   *
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

#ifndef ROOTMODEL_H
#define ROOTMODEL_H

#include "appsmodel.h"

class FavoritesModel;
class RecentAppsModel;

class GroupEntry : public AbstractGroupEntry
{
    public:
        GroupEntry(const QString &name, const QString &icon,
            AbstractModel *model, AbstractModel *parentModel);
};

class RootModel : public AppsModel
{
    Q_OBJECT

    Q_PROPERTY(QObject* recentAppsModel READ recentAppsModel NOTIFY recentAppsModelChanged)

    public:
        explicit RootModel(QObject *parent = 0);
        ~RootModel();

        Q_INVOKABLE QObject *favoritesModelForPrefix(const QString &prefix);

        QObject *recentAppsModel();

    Q_SIGNALS:
        void recentAppsModelChanged() const;

    protected Q_SLOTS:
        void refresh();

    private Q_SLOTS:
        void childModelChanged();

    private:
        void extendEntryList();

        QHash<QString, FavoritesModel *> m_favoritesModels;
        RecentAppsModel *m_recentAppsModel;
};

#endif
