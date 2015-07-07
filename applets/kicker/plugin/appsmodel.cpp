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

#include <QCollator>
#include <QQmlPropertyMap>
#include <QTimer>

#include <KLocalizedString>
#include <KSycoca>

AppsModel::AppsModel(const QString &entryPath, bool flat, QObject *parent)
: AbstractModel(parent)
, m_separatorCount(0)
, m_entryPath(entryPath)
, m_changeTimer(0)
, m_flat(flat)
, m_sortNeeded(false)
, m_appNameFormat(AppEntry::NameOnly)
{
    if (!m_entryPath.isEmpty()) {
        refresh();
    }
}

AppsModel::~AppsModel()
{
    qDeleteAll(m_entryList);
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
    } else if (role == Kicker::FavoriteIdRole && entry->type() == AbstractEntry::RunnableType) {
        return entry->id();
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
            actionList << Kicker::createActionItem(i18n("Unhide Applications in this Submenu"), "unhideSiblingApplications");
        }

        const AppsModel *appsModel = qobject_cast<const AppsModel *>(entry->childModel());

        if (appsModel && !appsModel->hiddenEntries().isEmpty()) {
            actionList << Kicker::createActionItem(i18n("Unhide Applications in '%1'", entry->name()), "unhideChildApplications");
        }

        return actionList;
    }

    return QVariant();
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

    if (actionId == "hideApplication" && entry->type() == AbstractEntry::RunnableType) {
        QObject *appletInterface = rootModel()->property("appletInterface").value<QObject *>();
        QQmlPropertyMap *appletConfig = qobject_cast<QQmlPropertyMap *>(appletInterface->property("configuration").value<QObject *>());

        if (appletConfig && appletConfig->contains("hiddenApplications")) {
            QStringList hiddenApps = appletConfig->value("hiddenApplications").toStringList();

            KService::Ptr service = static_cast<const AppEntry *>(entry)->service();

            if (!hiddenApps.contains(service->menuId())) {
                hiddenApps << service->menuId();

                appletConfig->insert("hiddenApplications", hiddenApps);
                QMetaObject::invokeMethod(appletConfig, "valueChanged", Qt::DirectConnection,
                    Q_ARG(QString, "hiddenApplications"),
                    Q_ARG(QVariant, hiddenApps));

                refresh();

                emit hiddenEntriesChanged();
            }
        }

        return false;
    } else if (actionId == "unhideSiblingApplications") {
        QObject *appletInterface = rootModel()->property("appletInterface").value<QObject *>();
        QQmlPropertyMap *appletConfig = qobject_cast<QQmlPropertyMap *>(appletInterface->property("configuration").value<QObject *>());

        if (appletConfig && appletConfig->contains("hiddenApplications")) {
            QStringList hiddenApps = appletConfig->value("hiddenApplications").toStringList();

            foreach(const QString& app, m_hiddenEntries) {
                hiddenApps.removeOne(app);
            }

            appletConfig->insert("hiddenApplications", hiddenApps);
            QMetaObject::invokeMethod(appletConfig, "valueChanged", Qt::DirectConnection,
                Q_ARG(QString, "hiddenApplications"),
                Q_ARG(QVariant, hiddenApps));

            m_hiddenEntries.clear();

            refresh();

            emit hiddenEntriesChanged();
        }

        return false;
    } else if (actionId == "unhideChildApplications") {
        QObject *appletInterface = rootModel()->property("appletInterface").value<QObject *>();
        QQmlPropertyMap *appletConfig = qobject_cast<QQmlPropertyMap *>(appletInterface->property("configuration").value<QObject *>());

        if (entry->type() == AbstractEntry::GroupType
            && appletConfig && appletConfig->contains("hiddenApplications")) {

            const AppsModel *appsModel = qobject_cast<const AppsModel *>(entry->childModel());

            if (!appsModel) {
                return false;
            }

            QStringList hiddenApps = appletConfig->value("hiddenApplications").toStringList();

            foreach(const QString& app, appsModel->hiddenEntries()) {
                hiddenApps.removeOne(app);
            }

            appletConfig->insert("hiddenApplications", hiddenApps);
            QMetaObject::invokeMethod(appletConfig, "valueChanged", Qt::DirectConnection,
                Q_ARG(QString, "hiddenApplications"),
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
        return 0;
    }

    return m_entryList.at(row)->childModel();
}

int AppsModel::separatorCount() const
{
    return m_separatorCount;
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

QStringList AppsModel::hiddenEntries() const
{
    return m_hiddenEntries;
}

void AppsModel::refresh()
{
    beginResetModel();

    emit refreshing();

    qDeleteAll(m_entryList);
    m_entryList.clear();
    m_hiddenEntries.clear();
    m_separatorCount = 0;
    m_sortNeeded = false;

    if (m_entryPath.isEmpty()) {
        KServiceGroup::Ptr group = KServiceGroup::root();

        bool sortByGenericName = (appNameFormat() == AppEntry::GenericNameOnly || appNameFormat() == AppEntry::GenericNameAndName);

        KServiceGroup::List list = group->entries(true /* sorted */, true /* excludeNoDisplay */,
            true /* allowSeparators */, sortByGenericName /* sortByGenericName */);

        for (KServiceGroup::List::ConstIterator it = list.constBegin(); it != list.constEnd(); it++) {
            const KSycocaEntry::Ptr p = (*it);

            if (p->isType(KST_KServiceGroup)) {
                KServiceGroup::Ptr subGroup(static_cast<KServiceGroup*>(p.data()));

                if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                    AppGroupEntry *groupEntry = new AppGroupEntry(this, subGroup, m_flat, m_appNameFormat);
                    m_entryList << groupEntry;
                }
            }
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

        if (m_sortNeeded) {
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
    }

    endResetModel();

    emit countChanged();
    emit separatorCountChanged();
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

    if (appletConfig && appletConfig->contains("hiddenApplications")) {
        hiddenApps = appletConfig->value("hiddenApplications").toStringList();
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
                }
            }

            if (!found) {
                m_entryList << new AppEntry(this, service, m_appNameFormat);
            }
        } else if (p->isType(KST_KServiceSeparator)) {
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
                m_sortNeeded = true;
                const KServiceGroup::Ptr serviceGroup(static_cast<KServiceGroup*>(p.data()));
                processServiceGroup(serviceGroup);
            } else {
                AppGroupEntry *groupEntry = new AppGroupEntry(this, subGroup, m_flat, m_appNameFormat);
                m_entryList << groupEntry;
            }
        }
    }
}

void AppsModel::checkSycocaChanges(const QStringList &changes)
{
    if (changes.contains("services") || changes.contains("apps") || changes.contains("xdgdata-apps")) {
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
