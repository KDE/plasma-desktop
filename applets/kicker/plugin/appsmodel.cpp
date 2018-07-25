/***************************************************************************
 *   Copyright (C) 2012 Aurélien Gâteau <agateau@kde.org>                  *
 *   Copyright (C) 2013-2015 by Eike Hein <hein@kde.org>                   *
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

#include "appsmodel.h"
#include "actionlist.h"
#include "rootmodel.h"

#include <QCollator>
#include <QDebug>
#include <QQmlPropertyMap>
#include <QTimer>

#include <KLocalizedString>
#include <KSycoca>

AppsModel::AppsModel(const QString &entryPath, bool paginate, int pageSize, bool flat,
    bool sorted, bool separators, QObject *parent)
: AbstractModel(parent)
, m_complete(false)
, m_paginate(paginate)
, m_pageSize(pageSize)
, m_deleteEntriesOnDestruction(true)
, m_separatorCount(0)
, m_showSeparators(separators)
, m_showTopLevelItems(false)
, m_appletInterface(nullptr)
, m_autoPopulate(true)
, m_description(i18n("Applications"))
, m_entryPath(entryPath)
, m_staticEntryList(false)
, m_changeTimer(nullptr)
, m_flat(flat)
, m_sorted(sorted)
, m_appNameFormat(AppEntry::NameOnly)
{
    if (!m_entryPath.isEmpty()) {
        componentComplete();
    }
}

AppsModel::AppsModel(const QList<AbstractEntry *> entryList, bool deleteEntriesOnDestruction, QObject *parent)
: AbstractModel(parent)
, m_complete(false)
, m_paginate(false)
, m_pageSize(24)
, m_deleteEntriesOnDestruction(deleteEntriesOnDestruction)
, m_separatorCount(0)
, m_showSeparators(false)
, m_showTopLevelItems(false)
, m_appletInterface(nullptr)
, m_autoPopulate(true)
, m_description(i18n("Applications"))
, m_entryPath(QString())
, m_staticEntryList(true)
, m_changeTimer(nullptr)
, m_flat(true)
, m_sorted(true)
, m_appNameFormat(AppEntry::NameOnly)
{
    foreach(AbstractEntry *suggestedEntry, entryList) {
        bool found = false;

        foreach (const AbstractEntry *entry, m_entryList) {
            if (entry->type() == AbstractEntry::RunnableType
                && static_cast<const AppEntry *>(entry)->service()->storageId()
                == static_cast<const AppEntry *>(suggestedEntry)->service()->storageId()) {
                found = true;
                break;
            }
        }

        if (!found) {
            m_entryList << suggestedEntry;
        }
    }

    sortEntries();
}

AppsModel::~AppsModel()
{
    if (m_deleteEntriesOnDestruction) {
        qDeleteAll(m_entryList);
    }
}

bool AppsModel::autoPopulate() const
{
    return m_autoPopulate;
}

void AppsModel::setAutoPopulate(bool populate)
{
    if (m_autoPopulate != populate) {
        m_autoPopulate = populate;

        emit autoPopulateChanged();
    }
}

QString AppsModel::description() const
{
    return m_description;
}

void AppsModel::setDescription(const QString &text)
{
    if (m_description != text) {
        m_description = text;

        emit descriptionChanged();
    }
}

QVariant AppsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entryList.count()) {
        return QVariant();
    }

    const AbstractEntry *entry = m_entryList.at(index.row());

    if (role == Qt::DisplayRole) {
        return entry->name();
    } else if (role == Qt::DecorationRole) {
        return entry->icon();
    } else if (role == Kicker::DescriptionRole) {
        return entry->description();
    } else if (role == Kicker::FavoriteIdRole && entry->type() == AbstractEntry::RunnableType) {
        return entry->id();
    } else if (role == Kicker::UrlRole && entry->type() == AbstractEntry::RunnableType) {
        return entry->url();
    } else if (role == Kicker::IsParentRole) {
        return (entry->type() == AbstractEntry::GroupType);
    } else if (role == Kicker::IsSeparatorRole) {
        return (entry->type() == AbstractEntry::SeparatorType);
    } else if (role == Kicker::HasChildrenRole) {
        return entry->hasChildren();
    } else if (role == Kicker::HasActionListRole) {
        const AppsModel *appsModel = qobject_cast<const AppsModel *>(entry->childModel());

        return entry->hasActions() || (appsModel && !appsModel->hiddenEntries().isEmpty());
    } else if (role == Kicker::ActionListRole) {
        QVariantList actionList = entry->actions();

        if (!m_hiddenEntries.isEmpty()) {
            actionList << Kicker::createSeparatorActionItem();
            actionList << Kicker::createActionItem(i18n("Unhide Applications in this Submenu"), QStringLiteral("unhideSiblingApplications"));
        }

        const AppsModel *appsModel = qobject_cast<const AppsModel *>(entry->childModel());

        if (appsModel && !appsModel->hiddenEntries().isEmpty()) {
            actionList << Kicker::createActionItem(i18n("Unhide Applications in '%1'", entry->name()), QStringLiteral("unhideChildApplications"));
        }

        return actionList;
    }

    return QVariant();
}

QModelIndex AppsModel::index(int row, int column, const QModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column, m_entryList.at(row)) : QModelIndex();
}


int AppsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entryList.count();
}

bool AppsModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    if (row < 0 || row >= m_entryList.count()) {
        return false;
    }

    AbstractEntry *entry = m_entryList.at(row);

    if (actionId == QLatin1String("hideApplication") && entry->type() == AbstractEntry::RunnableType) {
        QObject *appletInterface = rootModel()->property("appletInterface").value<QObject *>();
        QQmlPropertyMap *appletConfig = qobject_cast<QQmlPropertyMap *>(appletInterface->property("configuration").value<QObject *>());

        if (appletConfig && appletConfig->contains(QLatin1String("hiddenApplications"))) {
            QStringList hiddenApps = appletConfig->value(QLatin1String("hiddenApplications")).toStringList();

            KService::Ptr service = static_cast<const AppEntry *>(entry)->service();

            if (!hiddenApps.contains(service->menuId())) {
                hiddenApps << service->menuId();

                appletConfig->insert(QLatin1String("hiddenApplications"), hiddenApps);
                QMetaObject::invokeMethod(appletConfig, "valueChanged", Qt::DirectConnection,
                    Q_ARG(QString, QStringLiteral("hiddenApplications")),
                    Q_ARG(QVariant, hiddenApps));

                refresh();

                emit hiddenEntriesChanged();
            }
        }

        return false;
    } else if (actionId == QLatin1String("unhideSiblingApplications")) {
        QObject *appletInterface = rootModel()->property("appletInterface").value<QObject *>();
        QQmlPropertyMap *appletConfig = qobject_cast<QQmlPropertyMap *>(appletInterface->property("configuration").value<QObject *>());

        if (appletConfig && appletConfig->contains(QLatin1String("hiddenApplications"))) {
            QStringList hiddenApps = appletConfig->value(QLatin1String("hiddenApplications")).toStringList();

            foreach(const QString& app, m_hiddenEntries) {
                hiddenApps.removeOne(app);
            }

            appletConfig->insert(QStringLiteral("hiddenApplications"), hiddenApps);
            QMetaObject::invokeMethod(appletConfig, "valueChanged", Qt::DirectConnection,
                Q_ARG(QString, QStringLiteral("hiddenApplications")),
                Q_ARG(QVariant, hiddenApps));

            m_hiddenEntries.clear();

            refresh();

            emit hiddenEntriesChanged();
        }

        return false;
    } else if (actionId == QLatin1String("unhideChildApplications")) {
        QObject *appletInterface = rootModel()->property("appletInterface").value<QObject *>();
        QQmlPropertyMap *appletConfig = qobject_cast<QQmlPropertyMap *>(appletInterface->property("configuration").value<QObject *>());

        if (entry->type() == AbstractEntry::GroupType
            && appletConfig && appletConfig->contains(QLatin1String("hiddenApplications"))) {

            const AppsModel *appsModel = qobject_cast<const AppsModel *>(entry->childModel());

            if (!appsModel) {
                return false;
            }

            QStringList hiddenApps = appletConfig->value(QLatin1String("hiddenApplications")).toStringList();

            foreach(const QString& app, appsModel->hiddenEntries()) {
                hiddenApps.removeOne(app);
            }

            appletConfig->insert(QStringLiteral("hiddenApplications"), hiddenApps);
            QMetaObject::invokeMethod(appletConfig, "valueChanged", Qt::DirectConnection,
                Q_ARG(QString, QStringLiteral("hiddenApplications")),
                Q_ARG(QVariant, hiddenApps));

            refresh();

            emit hiddenEntriesChanged();
        }

        return false;
    }

    return entry->run(actionId, argument);
}

AbstractModel *AppsModel::modelForRow(int row)
{
    if (row < 0 || row >= m_entryList.count()) {
        return nullptr;
    }

    return m_entryList.at(row)->childModel();
}

int AppsModel::rowForModel(AbstractModel *model)
{
    for (int i = 0; i < m_entryList.count(); ++i) {
        if (m_entryList.at(i)->childModel() == model) {
            return i;
        }
    }

    return -1;
}

int AppsModel::separatorCount() const
{
    return m_separatorCount;
}

bool AppsModel::paginate() const
{
    return m_paginate;
}

void AppsModel::setPaginate(bool paginate)
{
    if (m_paginate != paginate) {
        m_paginate = paginate;

        refresh();

        emit paginateChanged();
    }
}

int AppsModel::pageSize() const
{
    return m_pageSize;
}

void AppsModel::setPageSize(int size)
{
    if (m_pageSize != size) {
        m_pageSize = size;

        refresh();

        emit pageSizeChanged();
    }
}

bool AppsModel::flat() const
{
    return m_flat;
}

void AppsModel::setFlat(bool flat)
{
    if (m_flat != flat) {
        m_flat = flat;

        refresh();

        emit flatChanged();
    }
}

bool AppsModel::sorted() const
{
    return m_sorted;
}

void AppsModel::setSorted(bool sorted)
{
    if (m_sorted != sorted) {
        m_sorted = sorted;

        refresh();

        emit sortedChanged();
    }
}

bool AppsModel::showSeparators() const
{
    return m_showSeparators;
}

void AppsModel::setShowSeparators(bool showSeparators) {
    if (m_showSeparators != showSeparators) {
        m_showSeparators = showSeparators;

        refresh();

        emit showSeparatorsChanged();
    }
}

bool AppsModel::showTopLevelItems() const
{
    return m_showTopLevelItems;
}

void AppsModel::setShowTopLevelItems(bool showTopLevelItems) {
    if (m_showTopLevelItems != showTopLevelItems) {
        m_showTopLevelItems = showTopLevelItems;

        refresh();

        emit showTopLevelItemsChanged();
    }
}

int AppsModel::appNameFormat() const
{
    return m_appNameFormat;
}

void AppsModel::setAppNameFormat(int format)
{
    if (m_appNameFormat != (AppEntry::NameFormat)format) {
        m_appNameFormat = (AppEntry::NameFormat)format;

        refresh();

        emit appNameFormatChanged();
    }
}

QObject* AppsModel::appletInterface() const
{
    return m_appletInterface;
}

void AppsModel::setAppletInterface(QObject* appletInterface)
{
    if (m_appletInterface != appletInterface) {
        m_appletInterface = appletInterface;

        refresh();

        emit appletInterfaceChanged();
    }
}

QStringList AppsModel::hiddenEntries() const
{
    return m_hiddenEntries;
}

void AppsModel::refresh()
{
    if (!m_complete) {
        return;
    }

    if (m_staticEntryList) {
        return;
    }

    if (rootModel() == this && !m_appletInterface) {
        return;
    }

    beginResetModel();

    refreshInternal();

    endResetModel();

    if (favoritesModel()) {
        favoritesModel()->refresh();
    }

    emit countChanged();
    emit separatorCountChanged();
}

void AppsModel::refreshInternal()
{
    if (m_staticEntryList) {
        return;
    }

    if (m_entryList.count()) {
        qDeleteAll(m_entryList);
        m_entryList.clear();
        emit cleared();
    }

    m_hiddenEntries.clear();
    m_separatorCount = 0;

    if (m_entryPath.isEmpty()) {
        KServiceGroup::Ptr group = KServiceGroup::root();
        if (!group) {
            return;
        }

        bool sortByGenericName = (appNameFormat() == AppEntry::GenericNameOnly || appNameFormat() == AppEntry::GenericNameAndName);

        KServiceGroup::List list = group->entries(true /* sorted */, true /* excludeNoDisplay */,
            true /* allowSeparators */, sortByGenericName /* sortByGenericName */);

        for (KServiceGroup::List::ConstIterator it = list.constBegin(); it != list.constEnd(); it++) {
            const KSycocaEntry::Ptr p = (*it);

            if (p->isType(KST_KServiceGroup)) {
                KServiceGroup::Ptr subGroup(static_cast<KServiceGroup*>(p.data()));

                if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                    AppGroupEntry *groupEntry = new AppGroupEntry(this, subGroup, m_paginate, m_pageSize, m_flat,
                        m_sorted, m_showSeparators, m_appNameFormat);
                    m_entryList << groupEntry;
                }
            } else if (p->isType(KST_KService) && m_showTopLevelItems) {
                const KService::Ptr service(static_cast<KService*>(p.data()));

                if (service->noDisplay()) {
                    continue;
                }

                bool found = false;

                foreach (const AbstractEntry *entry, m_entryList) {
                    if (entry->type() == AbstractEntry::RunnableType
                        && static_cast<const AppEntry *>(entry)->service()->storageId() == service->storageId()) {
                        found = true;
                    }
                }

                if (!found) {
                    m_entryList << new AppEntry(this, service, m_appNameFormat);
                }
             } else if (p->isType(KST_KServiceSeparator) && m_showSeparators && m_showTopLevelItems) {
                if (!m_entryList.count()) {
                    continue;
                }

                if (m_entryList.last()->type() == AbstractEntry::SeparatorType) {
                    continue;
                }

                m_entryList << new SeparatorEntry(this);
                ++m_separatorCount;
            }
        }

        if (m_entryList.count()) {
            while (m_entryList.last()->type() == AbstractEntry::SeparatorType) {
                m_entryList.removeLast();
                --m_separatorCount;
            }
        }

        if (m_sorted) {
            sortEntries();
        }

        m_changeTimer = new QTimer(this);
        m_changeTimer->setSingleShot(true);
        m_changeTimer->setInterval(100);
        connect(m_changeTimer, SIGNAL(timeout()), this, SLOT(refresh()));

        connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), SLOT(checkSycocaChanges(QStringList)));
    } else {
        KServiceGroup::Ptr group = KServiceGroup::group(m_entryPath);
        processServiceGroup(group);

        if (m_entryList.count()) {
            while (m_entryList.last()->type() == AbstractEntry::SeparatorType) {
                m_entryList.removeLast();
                --m_separatorCount;
            }
        }

        if (m_sorted) {
            sortEntries();
        }

        if (m_paginate) {
            QList<AbstractEntry *> groups;

            int at = 0;
            QList<AbstractEntry *> page;

            foreach(AbstractEntry *app, m_entryList) {
                page.append(app);

                if (at == (m_pageSize - 1)) {
                    at = 0;
                    AppsModel *model = new AppsModel(page, true, this);
                    groups.append(new GroupEntry(this, QString(), QString(), model));
                    page.clear();
                } else {
                    ++at;
                }
            }

            if (page.count()) {
                AppsModel *model = new AppsModel(page, true, this);
                groups.append(new GroupEntry(this, QString(), QString(), model));
            }

            m_entryList = groups;
        }
    }
}

void AppsModel::processServiceGroup(KServiceGroup::Ptr group)
{
    if (!group || !group->isValid()) {
        return;
    }

    bool hasSubGroups = false;

    foreach(KServiceGroup::Ptr subGroup, group->groupEntries(KServiceGroup::ExcludeNoDisplay)) {
        if (subGroup->childCount() > 0) {
            hasSubGroups = true;

            break;
        }
    }

    bool sortByGenericName = (appNameFormat() == AppEntry::GenericNameOnly || appNameFormat() == AppEntry::GenericNameAndName);

    KServiceGroup::List list = group->entries(true /* sorted */,
        true /* excludeNoDisplay */,
        (!m_flat || (m_flat && !hasSubGroups)) /* allowSeparators */,
        sortByGenericName /* sortByGenericName */);

    QStringList hiddenApps;

    QObject *appletInterface = rootModel()->property("appletInterface").value<QObject *>();
    QQmlPropertyMap *appletConfig = qobject_cast<QQmlPropertyMap *>(appletInterface->property("configuration").value<QObject *>());

    if (appletConfig && appletConfig->contains(QLatin1String("hiddenApplications"))) {
        hiddenApps = appletConfig->value(QLatin1String("hiddenApplications")).toStringList();
    }

    for (KServiceGroup::List::ConstIterator it = list.constBegin();
        it != list.constEnd(); it++) {
        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KService)) {
            const KService::Ptr service(static_cast<KService*>(p.data()));

            if (service->noDisplay()) {
                continue;
            }

            if (hiddenApps.contains(service->menuId())) {
                m_hiddenEntries << service->menuId();

                continue;
            }

            bool found = false;

            foreach (const AbstractEntry *entry, m_entryList) {
                if (entry->type() == AbstractEntry::RunnableType
                    && static_cast<const AppEntry *>(entry)->service()->storageId() == service->storageId()) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                m_entryList << new AppEntry(this, service, m_appNameFormat);
            }
        } else if (p->isType(KST_KServiceSeparator) && m_showSeparators) {
            if (!m_entryList.count()) {
                continue;
            }

            if (m_entryList.last()->type() == AbstractEntry::SeparatorType) {
                continue;
            }

            m_entryList << new SeparatorEntry(this);
            ++m_separatorCount;
        } else if (p->isType(KST_KServiceGroup)) {
            const KServiceGroup::Ptr subGroup(static_cast<KServiceGroup*>(p.data()));

            if (subGroup->childCount() == 0) {
                continue;
            }

            if (m_flat) {
                m_sorted = true;
                const KServiceGroup::Ptr serviceGroup(static_cast<KServiceGroup*>(p.data()));
                processServiceGroup(serviceGroup);
            } else {
                AppGroupEntry *groupEntry = new AppGroupEntry(this, subGroup, m_paginate, m_pageSize, m_flat,
                    m_sorted, m_showSeparators, m_appNameFormat);
                m_entryList << groupEntry;
            }
        }
    }
}

void AppsModel::sortEntries()
{
    QCollator c;

    std::sort(m_entryList.begin(), m_entryList.end(),
        [&c](AbstractEntry* a, AbstractEntry* b) {
            if (a->type() != b->type()) {
                return a->type() > b->type();
            } else {
                return c.compare(a->name(), b->name()) < 0;
            }
        });
}

void AppsModel::checkSycocaChanges(const QStringList &changes)
{
    if (changes.contains(QLatin1String("services")) || changes.contains(QLatin1String("apps")) || changes.contains(QLatin1String("xdgdata-apps"))) {
        m_changeTimer->start();
    }
}

void AppsModel::entryChanged(AbstractEntry *entry)
{
    int i = m_entryList.indexOf(entry);

    if (i != -1) {
        QModelIndex idx = index(i, 0);
        emit dataChanged(idx, idx);
    }
}

void AppsModel::classBegin()
{

}

void AppsModel::componentComplete()
{
    m_complete = true;

    if (m_autoPopulate) {
        refresh();
    }
}
