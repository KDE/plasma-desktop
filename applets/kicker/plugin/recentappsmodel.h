/***************************************************************************
 *   Copyright (C) 2013-2014 by Eike Hein <hein@kde.org>                   *
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

#ifndef RECENTAPPSMODEL_H
#define RECENTAPPSMODEL_H

#include "abstractmodel.h"

class RecentAppsModel : public AbstractModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList recentApps READ recentApps WRITE setRecentApps NOTIFY recentAppsChanged)

    public:
        explicit RecentAppsModel(QObject *parent = 0);
        ~RecentAppsModel();

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        int rowCount(const QModelIndex &parent = QModelIndex()) const;

        Q_INVOKABLE bool trigger(int row, const QString &actionId, const QVariant &argument);

        QStringList recentApps() const;
        void setRecentApps(const QStringList &recentApps);

    Q_SIGNALS:
        void recentAppsChanged() const;

    public Q_SLOTS:
        void addApp(const QString &storageId);

    private:
        void forgetApp(int row);
        void forgetAllApps();

        QList<QString> m_recentApps;
};

#endif
