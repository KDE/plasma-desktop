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

GroupEntry::GroupEntry(RootModel *parentModel, const QString &name,
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

QVariant GroupEntry::icon() const
{
    return QIcon::fromTheme(m_iconName, QIcon::fromTheme("unknown"));
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
, m_favorites(new FavoritesModel(this))
, m_systemModel(nullptr)
, m_showAllSubtree(false)
, m_showRecentApps(true)
, m_showRecentDocs(true)
, m_showRecentContacts(false)
, m_recentAppsModel(0)
, m_recentDocsModel(0)
, m_recentContactsModel(0)
, m_appletInterface(0)
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
                        group->name()), "hideCategory");
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
        if (actionId == "hideCategory") {
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

bool RootModel::showAllSubtree() const
{
    return m_showAllSubtree;
}

void RootModel::setShowAllSubtree(bool show)
{
    if (m_showAllSubtree != show) {
        m_showAllSubtree = show;

        refresh();

        emit showAllSubtreeChanged();
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

QObject* RootModel::appletInterface() const
{
    return m_appletInterface;
}

void RootModel::setAppletInterface(QObject* appletInterface)
{
    if (m_appletInterface != appletInterface) {
        m_appletInterface = appletInterface;

        refresh();

        emit appletInterfaceChanged();
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
    if (!m_appletInterface) {
        return;
    }

    beginResetModel();

    AppsModel::refreshInternal();

    AppsModel *allModel = nullptr;
    m_recentAppsModel = nullptr;
    m_recentDocsModel = nullptr;
    m_recentContactsModel = nullptr;

    if (m_showAllSubtree) {
        QHash<QString, QList<AbstractEntry *>> m_categoryHash;

        foreach (const AbstractEntry *groupEntry, m_entryList) {
            AbstractModel *model = groupEntry->childModel();

            for (int i = 0; i < model->count(); ++i) {
                AbstractEntry *appEntry = static_cast<AbstractEntry *>(model->index(i, 0).internalPointer());

                if (appEntry->name().isEmpty()) {
                    continue;
                }

                const QChar &first = appEntry->name().at(0).toUpper();
                m_categoryHash[first.isDigit() ? QStringLiteral("0-9") : first].append(appEntry);
            }
        }

        QList<AbstractEntry *> groups;
        QHashIterator<QString, QList<AbstractEntry *>> i(m_categoryHash);

        while (i.hasNext()) {
            i.next();
            AppsModel *model = new AppsModel(i.value(), false, this);
            model->setDescription(i.key());
            groups.append(new GroupEntry(this, i.key(), QString(), model));
        }

        allModel = new AppsModel(groups, true, this);
        allModel->setDescription(QStringLiteral("KICKER_ALL_MODEL")); // Intentionally no i18n.
    }

    if (m_showSeparators) {
        m_entryList.prepend(new SeparatorEntry(this));
        ++m_separatorCount;
    }

    if (allModel) {
        m_entryList.prepend(new GroupEntry(this, i18n("All Applications"), QString(), allModel));
    }

    if (m_showRecentContacts) {
        m_recentContactsModel = new RecentContactsModel(this);
        m_entryList.prepend(new GroupEntry(this, i18n("Recent Contacts"), QString(), m_recentContactsModel));
    }

    if (m_showRecentDocs) {
        m_recentDocsModel = new RecentDocsModel(this);
        m_entryList.prepend(new GroupEntry(this, i18n("Recent Documents"), QString(), m_recentDocsModel));
    }

    if (m_showRecentApps) {
        m_recentAppsModel = new RecentAppsModel(this);
        m_entryList.prepend(new GroupEntry(this, i18n("Recent Applications"), QString(), m_recentAppsModel));
    }

    m_systemModel = new SystemModel(this);
    m_entryList << new GroupEntry(this, i18n("Power / Session"), QString(), m_systemModel);

    endResetModel();

    m_favorites->refresh();

    emit systemFavoritesModelChanged();
    emit countChanged();
    emit separatorCountChanged();

    emit refreshed();
}

void RootModel::extendEntryList()
{

}
