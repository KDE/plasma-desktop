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
#include "kastatsfavoritesmodel.h"
#include "recentcontactsmodel.h"
#include "recentusagemodel.h"
#include "systemmodel.h"

#include <KLocalizedString>

#include <QCollator>

GroupEntry::GroupEntry(AppsModel *parentModel, const QString &name,
    const QString &iconName, AbstractModel *childModel)
: AbstractGroupEntry(parentModel)
, m_name(name)
, m_iconName(iconName)
, m_childModel(childModel)
{
    QObject::connect(parentModel, &RootModel::cleared, childModel, &AbstractModel::deleteLater);

    QObject::connect(childModel, &AbstractModel::countChanged,
        [parentModel, this] { if (parentModel) { parentModel->entryChanged(this); } }
    );
}

QString GroupEntry::name() const
{
    return m_name;
}

QIcon GroupEntry::icon() const
{
    return QIcon::fromTheme(m_iconName, QIcon::fromTheme(QStringLiteral("unknown")));
}

bool GroupEntry::hasChildren() const
{
    return m_childModel && m_childModel->count() > 0;
}

AbstractModel *GroupEntry::childModel() const
{
    return m_childModel;
}

RootModel::RootModel(QObject *parent) : AppsModel(QString(), parent)
, m_favorites(new KAStatsFavoritesModel(this))
, m_systemModel(nullptr)
, m_showAllApps(false)
, m_showRecentApps(true)
, m_showRecentDocs(true)
, m_showRecentContacts(false)
, m_recentOrdering(RecentUsageModel::Recent)
, m_showPowerSession(true)
, m_recentAppsModel(nullptr)
, m_recentDocsModel(nullptr)
, m_recentContactsModel(nullptr)
{
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
            AbstractModel *model = group->childModel();

            if (model == m_recentAppsModel
                || model == m_recentDocsModel
                || model == m_recentContactsModel) {
                if (role ==  Kicker::HasActionListRole) {
                    return true;
                } else if (role == Kicker::ActionListRole) {
                    QVariantList actionList;
                    actionList << model->actions();
                    actionList << Kicker::createSeparatorActionItem();
                    actionList << Kicker::createActionItem(i18n("Hide %1",
                        group->name()), QStringLiteral("hideCategory"));
                    return actionList;
                }
            }
        }
    }

    return AppsModel::data(index, role);
}

bool RootModel::trigger(int row, const QString& actionId, const QVariant& argument)
{
    const AbstractEntry *entry = m_entryList.at(row);

    if (entry->type() == AbstractEntry::GroupType) {
        if (actionId == QLatin1String("hideCategory")) {
            AbstractModel *model = entry->childModel();

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
        } else if (entry->childModel()->hasActions()) {
            return entry->childModel()->trigger(-1, actionId, QVariant());
        }
    }

    return AppsModel::trigger(row, actionId, argument);
}

bool RootModel::showAllApps() const
{
    return m_showAllApps;
}

void RootModel::setShowAllApps(bool show)
{
    if (m_showAllApps != show) {
        m_showAllApps = show;

        refresh();

        emit showAllAppsChanged();
    }
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

int RootModel::recentOrdering() const
{
    return m_recentOrdering;
}

void RootModel::setRecentOrdering(int ordering)
{
    if (ordering != m_recentOrdering) {
        m_recentOrdering = ordering;

        refresh();

        emit recentOrderingChanged();
    }
}

bool RootModel::showPowerSession() const
{
    return m_showPowerSession;
}

void RootModel::setShowPowerSession(bool show)
{
    if (show != m_showPowerSession) {
        m_showPowerSession = show;

        refresh();

        emit showPowerSessionChanged();
    }
}

AbstractModel* RootModel::favoritesModel()
{
    return m_favorites;
}

AbstractModel* RootModel::systemFavoritesModel()
{
    if (m_systemModel) {
        return m_systemModel->favoritesModel();
    }

    return nullptr;
}

void RootModel::refresh()
{
    if (!m_complete) {
        return;
    }

    beginResetModel();

    AppsModel::refreshInternal();

    AppsModel *allModel = nullptr;
    m_recentAppsModel = nullptr;
    m_recentDocsModel = nullptr;
    m_recentContactsModel = nullptr;

    if (m_showAllApps) {
        QList<AbstractEntry *> groups;

        if (m_paginate) {
            m_favorites = new KAStatsFavoritesModel(this);
            emit favoritesModelChanged();

            QHash<QString, AppEntry *> appsHash;
            QList<AppEntry *> apps;

            foreach (const AbstractEntry *groupEntry, m_entryList) {
                AbstractModel *model = groupEntry->childModel();

                if (!model) continue;

                for (int i = 0; i < model->count(); ++i) {
                    GroupEntry *subGroupEntry = static_cast<GroupEntry*>(model->index(i, 0).internalPointer());
                    AbstractModel *subModel = subGroupEntry->childModel();

                    for (int j = 0; j < subModel->count(); ++j) {
                        AppEntry *appEntry = static_cast<AppEntry*>(subModel->index(j, 0).internalPointer());

                        if (appEntry->name().isEmpty()) {
                            continue;
                        }

                        appsHash.insert(appEntry->service()->menuId(), appEntry);
                    }
                }
            }

            apps = appsHash.values();

            QCollator c;

            std::sort(apps.begin(), apps.end(),
                [&c](AbstractEntry* a, AbstractEntry* b) {
                    if (a->type() != b->type()) {
                        return a->type() > b->type();
                    } else {
                        return c.compare(a->name(), b->name()) < 0;
                    }
                });


            int at = 0;
            QList<AbstractEntry *> page;
            page.reserve(m_pageSize);

            foreach(AppEntry *app, apps) {
                page.append(app);

                if (at == (m_pageSize - 1)) {
                    at = 0;
                    AppsModel *model = new AppsModel(page, false, this);
                    groups.append(new GroupEntry(this, QString(), QString(), model));
                    page.clear();
                } else {
                    ++at;
                }
            }

            if (!page.isEmpty()) {
                AppsModel *model = new AppsModel(page, false, this);
                groups.append(new GroupEntry(this, QString(), QString(), model));
            }

            groups.prepend(new GroupEntry(this, QString(), QString(), m_favorites));
        } else {
            QHash<QString, QList<AbstractEntry *>> m_categoryHash;

            foreach (const AbstractEntry *groupEntry, m_entryList) {
                AbstractModel *model = groupEntry->childModel();

                if (!model) continue;

                for (int i = 0; i < model->count(); ++i) {
                    AbstractEntry *appEntry = static_cast<AbstractEntry *>(model->index(i, 0).internalPointer());

                    if (appEntry->name().isEmpty()) {
                        continue;
                    }

                    const QChar &first = appEntry->name().at(0).toUpper();
                    m_categoryHash[first.isDigit() ? QStringLiteral("0-9") : first].append(appEntry);
                }
            }

            QHashIterator<QString, QList<AbstractEntry *>> i(m_categoryHash);

            while (i.hasNext()) {
                i.next();
                AppsModel *model = new AppsModel(i.value(), false, this);
                model->setDescription(i.key());
                groups.append(new GroupEntry(this, i.key(), QString(), model));
            }
        }

        allModel = new AppsModel(groups, true, this);
        allModel->setDescription(QStringLiteral("KICKER_ALL_MODEL")); // Intentionally no i18n.
    }

    int separatorPosition = 0;

    if (allModel) {
        m_entryList.prepend(new GroupEntry(this, i18n("All Applications"), QString(), allModel));
        ++separatorPosition;
    }

    if (m_showRecentContacts) {
        m_recentContactsModel = new RecentContactsModel(this);
        m_entryList.prepend(new GroupEntry(this, i18n("Recent Contacts"), QString(), m_recentContactsModel));
        ++separatorPosition;
    }

    if (m_showRecentDocs) {
        m_recentDocsModel = new RecentUsageModel(this, RecentUsageModel::OnlyDocs, m_recentOrdering);
        m_entryList.prepend(new GroupEntry(this,
                    m_recentOrdering == RecentUsageModel::Recent
                        ? i18n("Recent Documents")
                        : i18n("Often Used Documents"),
                    QString(),
                    m_recentDocsModel));
        ++separatorPosition;
    }

    if (m_showRecentApps) {
        m_recentAppsModel = new RecentUsageModel(this, RecentUsageModel::OnlyApps, m_recentOrdering);
        m_entryList.prepend(new GroupEntry(this,
                    m_recentOrdering == RecentUsageModel::Recent
                        ? i18n("Recent Applications")
                        : i18n("Often Used Applications"),
                    QString(),
                    m_recentAppsModel));
        ++separatorPosition;
    }

    if (m_showSeparators && separatorPosition > 0) {
        m_entryList.insert(separatorPosition, new SeparatorEntry(this));
        ++m_separatorCount;
    }

    m_systemModel = new SystemModel(this);

    if (m_showPowerSession) {
        m_entryList << new GroupEntry(this, i18n("Power / Session"), QString(), m_systemModel);
    }

    endResetModel();

    m_favorites->refresh();

    emit systemFavoritesModelChanged();
    emit countChanged();
    emit separatorCountChanged();

    emit refreshed();
}
