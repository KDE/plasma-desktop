/*
 *   Copyright (C) 2012 - 2016 by Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BlacklistedApplicationsModel.h"

#include <QList>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

#include <KService>
#include <KConfig>
#include <KSharedConfig>
#include <KConfigGroup>

#include <utils/d_ptr_implementation.h>

#include "definitions.h"

class BlacklistedApplicationsModel::Private {
public:
    struct ApplicationData {
        QString name;
        QString title;
        QString icon;
        bool blocked;
    };

    QList<ApplicationData> applications;
    QSqlDatabase database;

    KSharedConfig::Ptr pluginConfig;
    bool enabled;
};

BlacklistedApplicationsModel::BlacklistedApplicationsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    d->enabled = false;
    d->pluginConfig = KSharedConfig::openConfig("kactivitymanagerd-pluginsrc");
}

BlacklistedApplicationsModel::~BlacklistedApplicationsModel()
{
}

QHash<int, QByteArray> BlacklistedApplicationsModel::roleNames() const
{
    return {
        { ApplicationIdRole, "name"},
        { Qt::DecorationRole, "icon"},
        { Qt::DisplayRole, "title"},
        { BlockedApplicationRole, "blocked" }
    };
}

void BlacklistedApplicationsModel::load()
{
    // Loading plugin configuration

    const auto config = d->pluginConfig->group(SQLITE_PLUGIN_CONFIG_KEY);

    const auto defaultBlockedValue = config.readEntry("blocked-by-default", false);
    auto blockedApplications = QSet<QString>::fromList(config.readEntry("blocked-applications", QStringList()));
    auto allowedApplications = QSet<QString>::fromList(config.readEntry("allowed-applications", QStringList()));

    // Reading new applications from the database

    const QString path
        = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
          + QStringLiteral("/kactivitymanagerd/resources/database");

    d->database = QSqlDatabase::addDatabase("QSQLITE", "plugins_sqlite_db_resources");
    d->database.setDatabaseName(path);

    if (!d->database.open()) {
        // qDebug() << "Failed to open the database" << path << d->database.lastError();
        return;
    }

    auto query = d->database.exec("SELECT DISTINCT(initiatingAgent) FROM ResourceScoreCache ORDER BY initiatingAgent");

    if (d->applications.length() > 0) {
        beginRemoveRows(QModelIndex(), 0, d->applications.length() - 1);
        d->applications.clear();
        endRemoveRows();
    }

    while (query.next()) {
        const auto name = query.value(0).toString();

        if (defaultBlockedValue) {
            if (!allowedApplications.contains(name)) {
                blockedApplications << name;
            }
        } else {
            if (!blockedApplications.contains(name)) {
                allowedApplications << name;
            }
        }
    }

    auto applications = (blockedApplications + allowedApplications).toList();

    if (applications.length() > 0) {
        qSort(applications);

        beginInsertRows(QModelIndex(), 0, applications.length() - 1);

        foreach(const auto & name, applications)
        {
            const auto service = KService::serviceByDesktopName(name);
            const auto blocked = blockedApplications.contains(name);

            if (service) {
                d->applications << Private::ApplicationData{
                                       name,
                                       service->name(),
                                       service->icon(),
                                       blocked
                                   };
            } else {
                d->applications << Private::ApplicationData{ name, name, QString(), blocked };
            }
        }

        endInsertRows();
    }
}

void BlacklistedApplicationsModel::save()
{
    auto config = d->pluginConfig->group(SQLITE_PLUGIN_CONFIG_KEY);
    QStringList blockedApplications;
    QStringList allowedApplications;

    for (int i = 0; i < rowCount(); i++) {
        (d->applications[i].blocked ? blockedApplications : allowedApplications)
            << d->applications[i].name;
    }

    config.writeEntry("allowed-applications", allowedApplications);
    config.writeEntry("blocked-applications", blockedApplications);
}

void BlacklistedApplicationsModel::defaults()
{
    for (int i = 0; i < rowCount(); i++) {
        d->applications[i].blocked = false;
    }

    dataChanged(QAbstractListModel::index(0),
                QAbstractListModel::index(rowCount() - 1));
}

void BlacklistedApplicationsModel::toggleApplicationBlocked(int index)
{
    if (index > rowCount()) {
        return;
    }

    d->applications[index].blocked = !d->applications[index].blocked;
    dataChanged(QAbstractListModel::index(index),
                QAbstractListModel::index(index));

    emit changed();
}

QVariant BlacklistedApplicationsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant();
}

QVariant BlacklistedApplicationsModel::data(const QModelIndex &modelIndex, int role) const
{
    const auto index = modelIndex.row();

    if (index > rowCount()) {
        return QVariant();
    }

    const auto &application = d->applications[index];

    switch (role) {
    default:
        return QVariant();

    case ApplicationIdRole:
        return application.name;

    case Qt::DisplayRole:
        return application.title;

    case Qt::DecorationRole:
        return application.icon.isEmpty() ? "application-x-executable" : application.icon;

    case BlockedApplicationRole:
        return application.blocked;
    }
}

int BlacklistedApplicationsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->applications.size();
}

bool BlacklistedApplicationsModel::enabled() const
{
    return d->enabled;
}

void BlacklistedApplicationsModel::setEnabled(bool enabled)
{
    d->enabled = enabled;
    emit enabledChanged(enabled);
}

// #include <BlacklistedApplicationsModel.moc>
