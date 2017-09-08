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

#include "systemmodel.h"
#include "actionlist.h"
#include "simplefavoritesmodel.h"
#include "systementry.h"

#include <QStandardPaths>

#include <KDirWatch>
#include <KLocalizedString>

SystemModel::SystemModel(QObject *parent) : AbstractModel(parent)
{
    init();

    m_favoritesModel = new SimpleFavoritesModel(this);

    const QString configFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/ksmserverrc";

    KDirWatch *watch = new KDirWatch(this);

    watch->addFile(configFile);

    connect(watch, &KDirWatch::dirty, this, &SystemModel::refresh);
    connect(watch, &KDirWatch::created, this, &SystemModel::refresh);
}

SystemModel::~SystemModel()
{
    qDeleteAll(m_entryList);
}

void SystemModel::init()
{
    QList<SystemEntry *> actions;

    actions << new SystemEntry(this, SystemEntry::LockSession);
    actions << new SystemEntry(this, SystemEntry::LogoutSession);
    actions << new SystemEntry(this, SystemEntry::SaveSession);
    actions << new SystemEntry(this, SystemEntry::SwitchUser);
    actions << new SystemEntry(this, SystemEntry::SuspendToRam);
    actions << new SystemEntry(this, SystemEntry::SuspendToDisk);
    actions << new SystemEntry(this, SystemEntry::Reboot);
    actions << new SystemEntry(this, SystemEntry::Shutdown);

    foreach(SystemEntry *entry, actions) {
        if (entry->isValid()) {
            m_entryList << entry;
        } else {
            delete entry;
        }
    }
}

QString SystemModel::description() const
{
    return i18n("System actions");
}

QVariant SystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entryList.count()) {
        return QVariant();
    }

    const SystemEntry *entry = m_entryList.at(index.row());

    if (role == Qt::DisplayRole) {
        return entry->name();
    } else if (role == Qt::DecorationRole) {
        return entry->iconName();
    } else if (role == Kicker::DescriptionRole) {
        return entry->description();
    } else if (role == Kicker::GroupRole) {
        return entry->group();
    } else if (role == Kicker::FavoriteIdRole) {
        return entry->id();
    }

    return QVariant();
}

int SystemModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entryList.count();
}

bool SystemModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    Q_UNUSED(actionId)
    Q_UNUSED(argument)

    if (row >= 0 && row < m_entryList.count()) {
        m_entryList.at(row)->run();

        return true;
    }

    return false;
}

void SystemModel::refresh()
{
    beginResetModel();

    qDeleteAll(m_entryList);
    m_entryList.clear();

    init();

    endResetModel();

    emit countChanged();

    m_favoritesModel->refresh();
}
