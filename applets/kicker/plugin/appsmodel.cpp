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
#include "containmentinterface.h"

#include <QTimer>

#include <KLocalizedString>
#include <KRun>
#include <KSycoca>

AppGroupEntry::AppGroupEntry(KServiceGroup::Ptr group, QAbstractListModel *parentModel,
    bool flat, int appNameFormat)
{
    m_name = group->caption();
    m_icon = QIcon::fromTheme(group->icon());
    m_model = new AppsModel(group->entryPath(), flat, parentModel);
    static_cast<AppsModel *>(m_model.data())->setAppNameFormat(appNameFormat);
    QObject::connect(parentModel, SIGNAL(refreshing()), m_model, SLOT(deleteLater()));
    QObject::connect(m_model, SIGNAL(appLaunched(QString)), parentModel, SIGNAL(appLaunched(QString)));
}

AppEntry::AppEntry(KService::Ptr service, NameFormat nameFormat)
: m_service(service)
{
    const QString &name = service->name();
    QString genericName = service->genericName();

    if (genericName.isEmpty()) {
        genericName = service->comment();
    }

    if (nameFormat == NameOnly || genericName.isEmpty() || name == genericName) {
        m_name = name;
    } else if (nameFormat == GenericNameOnly) {
        m_name = genericName;
    } else if (nameFormat == NameAndGenericName) {
        m_name = i18nc("App name (Generic name)", "%1 (%2)", name, genericName);
    } else {
        m_name = i18nc("Generic name (App name)", "%1 (%2)", genericName, name);
    }

    m_icon = QIcon::fromTheme(service->icon());
    m_service = service;
}

ContainmentInterface *AppsModel::m_containmentInterface = 0;

AppsModel::AppsModel(const QString &entryPath, bool flat, QObject *parent)
: AbstractModel(parent)
, m_entryPath(entryPath)
, m_changeTimer(0)
, m_flat(flat)
, m_appNameFormat(AppEntry::NameOnly)
, m_sortNeeded(false)
{
    refresh();
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
        return m_entryList.at(index.row())->name();
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
    } else if (role == Kicker::HasActionListRole) {
        return (m_entryList.at(index.row())->type() == AbstractEntry::RunnableType);
    } else if (role == Kicker::ActionListRole) {
        if (m_containmentInterface) {
            QVariantList actionList;

            if (m_containmentInterface->mayAddLauncher(ContainmentInterface::Desktop)) {
                actionList << Kicker::createActionItem(i18n("Add to Desktop"), "addToDesktop");
            }

            if (m_containmentInterface->mayAddLauncher(ContainmentInterface::Panel)) {
                actionList << Kicker::createActionItem(i18n("Add to Panel"), "addToPanel");
            }

            if (m_containmentInterface->mayAddLauncher(ContainmentInterface::TaskManager,
                static_cast<AppEntry *>(m_entryList.at(index.row()))->service()->entryPath())) {
                actionList << Kicker::createActionItem(i18n("Add as Launcher"), "addToTaskManager");
            }

            return actionList;
        }
    } else if (role == Kicker::UrlRole) {
        if (m_entryList.at(index.row())->type() == AbstractEntry::RunnableType) {
            return QUrl::fromLocalFile(static_cast<AppEntry *>(m_entryList.at(index.row()))->service()->entryPath());
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
    Q_UNUSED(argument)

    if (row < 0 || row >= m_entryList.count() || m_entryList.at(row)->type() != AbstractEntry::RunnableType) {
        return false;
    }

    if (actionId == "addToDesktop" && m_containmentInterface
        && m_containmentInterface->mayAddLauncher(ContainmentInterface::Desktop)) {
        m_containmentInterface->addLauncher(ContainmentInterface::Desktop,
            static_cast<AppEntry *>(m_entryList.at(row))->service()->entryPath());
    } else if (actionId == "addToPanel" && m_containmentInterface
        && m_containmentInterface->mayAddLauncher(ContainmentInterface::Panel)) {
        m_containmentInterface->addLauncher(ContainmentInterface::Panel,
            static_cast<AppEntry *>(m_entryList.at(row))->service()->entryPath());
    } else if (actionId == "addToTaskManager" && m_containmentInterface
        && m_containmentInterface->mayAddLauncher(ContainmentInterface::TaskManager,
            static_cast<AppEntry *>(m_entryList.at(row))->service()->entryPath())) {
        m_containmentInterface->addLauncher(ContainmentInterface::TaskManager,
            static_cast<AppEntry *>(m_entryList.at(row))->service()->entryPath());
    } else if (actionId.isEmpty()) {
        KService::Ptr service = static_cast<AppEntry *>(m_entryList.at(row))->service();

        bool ran = KRun::run(*service, QList<QUrl>(), 0);

        if (ran) {
            emit appLaunched(service->storageId());
        }

        return ran;
    }

    return false;
}

AbstractModel *AppsModel::modelForRow(int row)
{
    if (row < 0 || row >= m_entryList.count() || m_entryList.at(row)->type() != AbstractEntry::GroupType) {
        return 0;
    }

    return static_cast<AbstractGroupEntry *>(m_entryList.at(row))->model();
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

QObject* AppsModel::appletInterface() const
{
    if (m_containmentInterface) {
        return m_containmentInterface;
    }

    return 0;
}

void AppsModel::setAppletInterface(QObject* appletInterface)
{
    if (!m_containmentInterface) {
        m_containmentInterface = new ContainmentInterface(this);
    }

    m_containmentInterface->setApplet(appletInterface);
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
                    m_entryList << new AppGroupEntry(subGroup, this, m_flat, m_appNameFormat);
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

        if (m_sortNeeded) {
            qSort(m_entryList.begin(), m_entryList.end(), AbstractEntry::lessThan);

            m_sortNeeded = false;
        }
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
                bool found = false;

                foreach (const AbstractEntry *entry, m_entryList) {
                    if (entry->type() == AbstractEntry::RunnableType
                        && static_cast<const AppEntry *>(entry)->service()->storageId() == service->storageId()) {
                        found = true;
                    }
                }

                if (!found) {
                    m_entryList << new AppEntry(service, m_appNameFormat);
                }
            }

        } else if (p->isType(KST_KServiceGroup)) {
            if (m_flat) {
                processServiceGroup(static_cast<KServiceGroup::Ptr>(p));

                m_sortNeeded = true;
            } else {
                const KServiceGroup::Ptr subGroup = static_cast<KServiceGroup::Ptr>(p);

                if (!subGroup->noDisplay() && subGroup->childCount() > 0) {
                    m_entryList << new AppGroupEntry(subGroup, this, m_flat, m_appNameFormat);
                }
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
