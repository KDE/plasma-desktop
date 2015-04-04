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
class RecentDocsModel;
class RecentContactsModel;

class GroupEntry : public AbstractGroupEntry
{
    public:
        GroupEntry(const QString &name, const QString &icon,
            AbstractModel *model, AbstractModel *parentModel);
};

class RootModel : public AppsModel
{
    Q_OBJECT

    Q_PROPERTY(bool showRecentApps READ showRecentApps WRITE setShowRecentApps NOTIFY showRecentAppsChanged)
    Q_PROPERTY(bool showRecentDocs READ showRecentDocs WRITE setShowRecentDocs NOTIFY showRecentDocsChanged)
    Q_PROPERTY(bool showRecentContacts READ showRecentContacts WRITE setShowRecentContacts NOTIFY showRecentContactsChanged)

    public:
        explicit RootModel(QObject *parent = 0);
        ~RootModel();

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        Q_INVOKABLE virtual bool trigger(int row, const QString &actionId, const QVariant &argument);

        bool showRecentApps() const;
        void setShowRecentApps(bool show);

        bool showRecentDocs() const;
        void setShowRecentDocs(bool show);

        bool showRecentContacts() const;
        void setShowRecentContacts(bool show);

        Q_INVOKABLE QObject *favoritesModelForPrefix(const QString &prefix);

    Q_SIGNALS:
        void showRecentAppsChanged() const;
        void showRecentDocsChanged() const;
        void showRecentContactsChanged() const;
        void recentAppsModelChanged() const;

    protected Q_SLOTS:
        void refresh();

    private Q_SLOTS:
        void childModelChanged();

    private:
        void extendEntryList();

        bool m_showRecentApps;
        bool m_showRecentDocs;
        bool m_showRecentContacts;

        QHash<QString, FavoritesModel *> m_favoritesModels;
        RecentAppsModel *m_recentAppsModel;
        RecentDocsModel *m_recentDocsModel;
        RecentContactsModel *m_recentContactsModel;
};

#endif
