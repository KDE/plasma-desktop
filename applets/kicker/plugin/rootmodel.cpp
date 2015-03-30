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

#include "rootmodel.h"
#include "actionlist.h"
#include "favoritesmodel.h"
#include "recentappsmodel.h"
#include "recentdocsmodel.h"
#include "recentcontactsmodel.h"
#include "systemmodel.h"

#include <KLocalizedString>

GroupEntry::GroupEntry(const QString &name, const QString &icon,
    AbstractModel *model, AbstractModel *parentModel)
{
    m_name = name;
    m_icon = QIcon::fromTheme(icon);
    m_model = model;
    QObject::connect(parentModel, SIGNAL(refreshing()), m_model, SLOT(deleteLater()));
}

RootModel::RootModel(QObject *parent) : AppsModel(QString(), parent)
, m_showRecentApps(true)
, m_showRecentDocs(true)
, m_showRecentContacts(true)
, m_recentAppsModel(0)
, m_recentDocsModel(0)
, m_recentContactsModel(0)
{
    FavoritesModel *favoritesModel = new FavoritesModel(this);
    connect(favoritesModel, SIGNAL(appLaunched(QString)), this, SIGNAL(appLaunched(QString)));
    m_favoritesModels["app"] = favoritesModel;

    favoritesModel = new FavoritesModel(this);
    m_favoritesModels["sys"] = favoritesModel;

    extendEntryList();
}

RootModel::~RootModel()
{
}

QVariant RootModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_entryList.count()) {
        return QVariant();
    }

    if (role == Kicker::HasActionListRole || role == Kicker::ActionListRole) {
        const AbstractEntry *entry = m_entryList.at(index.row());

        if (entry->type() == AbstractEntry::GroupType) {
            const GroupEntry *group = static_cast<const GroupEntry *>(entry);
            AbstractModel *model = group->model();

            if (model == m_recentAppsModel
                || model == m_recentDocsModel
                || model == m_recentContactsModel) {
                if (role ==  Kicker::HasActionListRole) {
                    return true;
                } else if (role == Kicker::ActionListRole) {
                    return QVariantList() << Kicker::createActionItem(i18n("Hide %1",
                        group->name()), "hideCategory");
                }
            }
        }
    }

    return AppsModel::data(index, role);
}

bool RootModel::trigger(int row, const QString& actionId, const QVariant& argument)
{
    if (actionId == "hideCategory") {
        const AbstractEntry *entry = m_entryList.at(row);

        if (entry->type() == AbstractEntry::GroupType) {
            const GroupEntry *group = static_cast<const GroupEntry *>(entry);
            AbstractModel *model = group->model();

            if (model == m_recentAppsModel) {
                setShowRecentApps(false);

                return true;
            } else if (model == m_recentDocsModel) {
                setShowRecentDocs(false);

                return true;
            } else if (model == m_recentContactsModel) {
                setShowRecentContacts(false);

                return true;
            }
        }
    }

    return AppsModel::trigger(row, actionId, argument);
}

bool RootModel::showRecentApps() const
{
    return m_showRecentApps;
}

void RootModel::setShowRecentApps(bool show)
{
    if (show != m_showRecentApps) {
        m_showRecentApps = show;

        refresh();

        emit showRecentAppsChanged();
    }
}

bool RootModel::showRecentDocs() const
{
    return m_showRecentDocs;
}

void RootModel::setShowRecentDocs(bool show)
{
    if (show != m_showRecentDocs) {
        m_showRecentDocs = show;

        refresh();

        emit showRecentDocsChanged();
    }
}

bool RootModel::showRecentContacts() const
{
    return m_showRecentContacts;
}

void RootModel::setShowRecentContacts(bool show)
{
    if (show != m_showRecentContacts) {
        m_showRecentContacts = show;

        refresh();

        emit showRecentContactsChanged();
    }
}

QObject *RootModel::favoritesModelForPrefix(const QString &prefix)
{
    return m_favoritesModels.value(prefix);
}

QObject *RootModel::recentAppsModel()
{
    return m_recentAppsModel;
}

void RootModel::refresh()
{
    AppsModel::refresh();
    extendEntryList();

    m_favoritesModels["app"]->refresh();
}

void RootModel::extendEntryList()
{
    m_recentAppsModel = 0;

    if (m_showRecentApps) {
        m_recentAppsModel = new RecentAppsModel(this);
        connect(m_recentAppsModel, SIGNAL(countChanged()), this, SLOT(childModelChanged()));
        connect(this, SIGNAL(appLaunched(QString)), m_recentAppsModel, SLOT(addApp(QString)));
    }

    emit recentAppsModelChanged();

    m_recentDocsModel = 0;

    if (m_showRecentDocs) {
        m_recentDocsModel = new RecentDocsModel(this);
        connect(m_recentDocsModel, SIGNAL(countChanged()), this, SLOT(childModelChanged()));
    }

    m_recentContactsModel = 0;

    if (m_showRecentContacts) {
        m_recentContactsModel = new RecentContactsModel(this);
        connect(m_recentContactsModel, SIGNAL(countChanged()), this, SLOT(childModelChanged()));
    }

    SystemModel *systemModel = new SystemModel(this);
    m_favoritesModels["sys"]->setSourceModel(systemModel);

    int insertCount = 0;
    if (m_recentAppsModel) ++insertCount;
    if (m_recentDocsModel) ++insertCount;
    if (m_recentContactsModel) ++insertCount;

    if (insertCount) {
        beginInsertRows(QModelIndex(), 0, insertCount - 1);

        GroupEntry *entry = 0;

        if (m_recentContactsModel) {
            entry = new GroupEntry(i18n("Recent Contacts"), QString(), m_recentContactsModel, this);
            m_entryList.prepend(entry);
        }

        if (m_recentDocsModel) {
            entry = new GroupEntry(i18n("Recent Documents"), QString(), m_recentDocsModel, this);
            m_entryList.prepend(entry);
        }

        if (m_recentAppsModel) {
            entry = new GroupEntry(i18n("Recent Applications"), QString(), m_recentAppsModel, this);
            m_entryList.prepend(entry);
        }

        endInsertRows();
    }

    beginInsertRows(QModelIndex(), m_entryList.size(), m_entryList.size());
    m_entryList << new GroupEntry(i18n("Power / Session"), QString(), systemModel, this);
    endInsertRows();

    emit countChanged();
}

void RootModel::childModelChanged()
{
    QObject *model = sender();

    for (int i = 0; i < m_entryList.size(); ++i) {
        const AbstractEntry *entry = m_entryList.at(i);

        if (entry->type() == AbstractEntry::GroupType) {
            const GroupEntry *group = static_cast<const GroupEntry *>(entry);

            if (group->model() == model) {
                const QModelIndex &idx = index(i, 0);

                emit dataChanged(idx, idx);

                break;
            }
        }

    }
}
