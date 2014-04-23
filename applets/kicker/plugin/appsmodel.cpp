/***************************************************************************
 *   Copyright (C) 2012 Aurélien Gâteau <agateau@kde.org>                  *
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

#include "appsmodel.h"
#include "actionlist.h"

#include <QTimer>

#include <KLocalizedString>
#include <KRun>
#include <KSycoca>

AppGroupEntry::AppGroupEntry(KServiceGroup::Ptr group, QAbstractListModel *parentModel, bool preferGenericNames)
{
    m_name = group->caption();
    m_icon = group->icon();
    m_model = new AppsModel(group->entryPath(), parentModel);
    static_cast<AppsModel *>(m_model.data())->setPreferGenericNames(preferGenericNames);
    QObject::connect(parentModel, SIGNAL(refreshing()), m_model, SLOT(deleteLater()));
    QObject::connect(m_model, SIGNAL(appLaunched(QString)), parentModel, SIGNAL(appLaunched(QString)));
}

AppEntry::AppEntry(KService::Ptr service)
: m_service(service)
{
    m_name = service->name();
    m_genericName = service->genericName();
    m_icon = service->icon();
    m_service = service;
}

AppsModel::AppsModel(const QString &entryPath, QObject *parent)
: AbstractModel(parent)
, m_entryPath(entryPath)
, m_changeTimer(0)
, m_preferGenericNames(false)
{
    refresh();

    // HACK: Reserve action names for message freeze.
    i18n("Add to Desktop");
    i18n("Add to Panel");
    i18n("Add as Launcher");
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

    if (role == Qt::DisplayRole) {
        AbstractEntry *entry = m_entryList.at(index.row());

        if (entry->type() == AbstractEntry::RunnableType) {
            AppEntry *appEntry = static_cast<AppEntry *>(entry);

            if (!appEntry->genericName().isEmpty() && appEntry->name() != appEntry->genericName()) {
                if (m_preferGenericNames) {
                    return i18nc("Generic name (App name)", "%1 (%2)", appEntry->genericName(), appEntry->name());
                } else {
                    return i18nc("App name (Generic name)", "%1 (%2)", appEntry->name(), appEntry->genericName());
                }
            } else {
                return appEntry->name();
            }
        } else {
            return m_entryList.at(index.row())->name();
        }
    } else if (role == Qt::DecorationRole) {
        return m_entryList.at(index.row())->icon();
    } else if (role == Kicker::HasChildrenRole) {
        AbstractEntry *entry = m_entryList.at(index.row());

        if (entry->type() == AbstractEntry::GroupType) {
            AbstractGroupEntry *groupEntry = static_cast<AbstractGroupEntry *>(entry);

            if (groupEntry->model() && groupEntry->model()->count()) {
                return true;
            }
        }
    } else if (role == Kicker::FavoriteIdRole) {
        if (m_entryList.at(index.row())->type() == AbstractEntry::RunnableType) {
            return QVariant("app:" + static_cast<AppEntry *>(m_entryList.at(index.row()))->service()->storageId());
        }
    }

    return QVariant();
}

int AppsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entryList.count();
}

bool AppsModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    Q_UNUSED(actionId)
    Q_UNUSED(argument)

    if (row < 0 || row >= m_entryList.count() || m_entryList.at(row)->type() != AbstractEntry::RunnableType) {
        return false;
    }

    KService::Ptr service = static_cast<AppEntry *>(m_entryList.at(row))->service();

    bool ran = KRun::run(*service, QList<QUrl>(), 0);

    if (ran) {
        emit appLaunched(service->storageId());
    }

    return ran;
}

QObject *AppsModel::modelForRow(int row)
{
    if (row < 0 || row >= m_entryList.count() || m_entryList.at(row)->type() != AbstractEntry::GroupType) {
        return 0;
    }

    return static_cast<AbstractGroupEntry *>(m_entryList.at(row))->model();
}

bool AppsModel::preferGenericNames() const
{
    return m_preferGenericNames;
}

void AppsModel::setPreferGenericNames(bool preferGenericNames)
{
    if (m_preferGenericNames != preferGenericNames) {
        m_preferGenericNames = preferGenericNames;

        refresh();

        emit preferGenericNamesChanged();
    }
}

void AppsModel::refresh()
{
    beginResetModel();

    emit refreshing();

    qDeleteAll(m_entryList);
    m_entryList.clear();

    if (m_entryPath.isEmpty()) {
        KServiceGroup::Ptr group = KServiceGroup::root();
        KServiceGroup::List list = group->entries(true);

        for (KServiceGroup::List::ConstIterator it = list.constBegin(); it != list.constEnd(); it++) {
            const KSycocaEntry::Ptr p = (*it);

            if (p->isType(KST_KServiceGroup)) {
                KServiceGroup::Ptr subGroup = static_cast<KServiceGroup::Ptr >(p);

                if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                    m_entryList << new AppGroupEntry(subGroup, this, m_preferGenericNames);
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
    }

    endResetModel();

    emit countChanged();
}

void AppsModel::processServiceGroup(KServiceGroup::Ptr group)
{
    if (!group || !group->isValid()) {
        return;
    }

    KServiceGroup::List list = group->entries(true);

    for (KServiceGroup::List::ConstIterator it = list.constBegin();
        it != list.constEnd(); it++) {
        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KService)) {
            const KService::Ptr service = static_cast<KService::Ptr>(p);

            if (!service->noDisplay()) {
                QString genericName = service->genericName();
                if (genericName.isNull()) {
                    genericName = service->comment();
                }

                bool found = false;

                foreach (const AbstractEntry *entry, m_entryList) {
                    if (entry->type() == AbstractEntry::RunnableType
                        && static_cast<const AppEntry *>(entry)->service()->storageId() == service->storageId()) {
                        found = true;
                    }
                }

                if (!found) {
                    m_entryList << new AppEntry(service);
                }
            }

        } else if (p->isType(KST_KServiceGroup)) {
            const KServiceGroup::Ptr subGroup = static_cast<KServiceGroup::Ptr>(p);

            if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                m_entryList << new AppGroupEntry(subGroup, this, m_preferGenericNames);
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
