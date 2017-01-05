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

#ifndef ROOTMODEL_H
#define ROOTMODEL_H

#include "appsmodel.h"

#include <QQmlParserStatus>

class FavoritesModel;
class RecentContactsModel;
class RecentUsageModel;
class SystemModel;

class RootModel;

class GroupEntry : public AbstractGroupEntry
{
    public:
        GroupEntry(AppsModel *parentModel, const QString &name,
            const QString &iconName, AbstractModel *childModel);

        QIcon icon() const;
        QString name() const;

        bool hasChildren() const;
        AbstractModel *childModel() const;

    private:
        QString m_name;
        QString m_iconName;
        QPointer<AbstractModel> m_childModel;
};

class RootModel : public AppsModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(bool autoPopulate READ autoPopulate WRITE setAutoPopulate NOTIFY autoPopulateChanged)

    Q_PROPERTY(QObject* systemFavoritesModel READ systemFavoritesModel NOTIFY systemFavoritesModelChanged)
    Q_PROPERTY(bool showAllApps READ showAllApps WRITE setShowAllApps NOTIFY showAllAppsChanged)
    Q_PROPERTY(bool showRecentApps READ showRecentApps WRITE setShowRecentApps NOTIFY showRecentAppsChanged)
    Q_PROPERTY(bool showRecentDocs READ showRecentDocs WRITE setShowRecentDocs NOTIFY showRecentDocsChanged)
    Q_PROPERTY(bool showRecentContacts READ showRecentContacts WRITE setShowRecentContacts NOTIFY showRecentContactsChanged)
    Q_PROPERTY(bool showPowerSession READ showPowerSession WRITE setShowPowerSession NOTIFY showPowerSessionChanged)

    public:
        explicit RootModel(QObject *parent = 0);
        ~RootModel();

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        Q_INVOKABLE virtual bool trigger(int row, const QString &actionId, const QVariant &argument);

        bool autoPopulate() const;
        void setAutoPopulate(bool populate);

        bool showAllApps() const;
        void setShowAllApps(bool show);

        bool showRecentApps() const;
        void setShowRecentApps(bool show);

        bool showRecentDocs() const;
        void setShowRecentDocs(bool show);

        bool showRecentContacts() const;
        void setShowRecentContacts(bool show);

        bool showPowerSession() const;
        void setShowPowerSession(bool show);

        AbstractModel* favoritesModel();
        AbstractModel* systemFavoritesModel();

        void classBegin() override;
        void componentComplete() override;

    Q_SIGNALS:
        void refreshed() const;
        void systemFavoritesModelChanged() const;
        void autoPopulateChanged() const;
        void showAllAppsChanged() const;
        void showRecentAppsChanged() const;
        void showRecentDocsChanged() const;
        void showRecentContactsChanged() const;
        void showPowerSessionChanged() const;
        void recentAppsModelChanged() const;

    protected Q_SLOTS:
        void refresh();

    private:
        bool m_complete;

        FavoritesModel *m_favorites;
        SystemModel *m_systemModel;

        bool m_autoPopulate;

        bool m_showAllApps;
        bool m_showRecentApps;
        bool m_showRecentDocs;
        bool m_showRecentContacts;
        bool m_showPowerSession;

        RecentUsageModel *m_recentAppsModel;
        RecentUsageModel *m_recentDocsModel;
        RecentContactsModel *m_recentContactsModel;
};

#endif
