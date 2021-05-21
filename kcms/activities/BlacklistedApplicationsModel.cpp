/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "BlacklistedApplicationsModel.h"

#include <QDebug>
#include <QList>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <KService>

#include "kactivitymanagerd_plugins_settings.h"

#include <utils/d_ptr_implementation.h>

#include "definitions.h"

class BlacklistedApplicationsModel::Private
{
public:
    struct ApplicationData {
        QString name;
        QString title;
        QString icon;
        bool blocked;
    };

    QList<ApplicationData> applications;
    QSqlDatabase database;

    KActivityManagerdPluginsSettings *pluginConfig;
    bool enabled;
};

BlacklistedApplicationsModel::BlacklistedApplicationsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    d->enabled = false;
    d->pluginConfig = new KActivityManagerdPluginsSettings;

    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kactivitymanagerd/resources/database");
    d->database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("plugins_sqlite_db_resources"));
    d->database.setDatabaseName(path);
}

BlacklistedApplicationsModel::~BlacklistedApplicationsModel()
{
}

QHash<int, QByteArray> BlacklistedApplicationsModel::roleNames() const
{
    return {{ApplicationIdRole, "name"}, {Qt::DecorationRole, "icon"}, {Qt::DisplayRole, "title"}, {BlockedApplicationRole, "blocked"}};
}

void BlacklistedApplicationsModel::load()
{
    // Loading plugin configuration
    d->pluginConfig->load();
    const auto defaultBlockedValue = d->pluginConfig->defaultBlockedByDefaultValue();
    auto blockedApplications = QSet<QString>::fromList(d->pluginConfig->blockedApplications());
    auto allowedApplications = QSet<QString>::fromList(d->pluginConfig->allowedApplications());

    // Reading new applications from the database

    if (!d->database.open()) {
        // qDebug() << "Failed to open the database" << path << d->database.lastError();
        return;
    }

    auto query = d->database.exec(QStringLiteral("SELECT DISTINCT(initiatingAgent) FROM ResourceScoreCache ORDER BY initiatingAgent"));

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

    auto applications = (blockedApplications + allowedApplications).values();

    if (applications.length() > 0) {
        std::sort(applications.begin(), applications.end());

        beginInsertRows(QModelIndex(), 0, applications.length() - 1);

        foreach (const auto &name, applications) {
            const auto service = KService::serviceByDesktopName(name);
            const auto blocked = blockedApplications.contains(name);

            if (service) {
                d->applications << Private::ApplicationData{name, service->name(), service->icon(), blocked};
            } else {
                d->applications << Private::ApplicationData{name, name, QString(), blocked};
            }
        }

        endInsertRows();
    }
}

void BlacklistedApplicationsModel::save()
{
    d->pluginConfig->save();
    Q_EMIT changed(false);
}

void BlacklistedApplicationsModel::defaults()
{
    for (int i = 0; i < rowCount(); i++) {
        d->applications[i].blocked = false;
    }

    Q_EMIT dataChanged(QAbstractListModel::index(0), QAbstractListModel::index(rowCount() - 1));

    Q_EMIT defaulted(true);
}

void BlacklistedApplicationsModel::toggleApplicationBlocked(int index)
{
    if (index > rowCount()) {
        return;
    }

    d->applications[index].blocked = !d->applications[index].blocked;
    Q_EMIT dataChanged(QAbstractListModel::index(index), QAbstractListModel::index(index));

    QStringList blockedApplications;
    QStringList allowedApplications;

    for (int i = 0; i < rowCount(); i++) {
        const auto name = d->applications[i].name;
        if (d->applications[i].blocked) {
            blockedApplications << name;
        } else {
            allowedApplications << name;
        }
    }

    d->pluginConfig->setBlockedApplications(blockedApplications);
    d->pluginConfig->setAllowedApplications(allowedApplications);

    const auto allowedApplicationsItem = d->pluginConfig->findItem("allowedApplications");
    Q_ASSERT(allowedApplicationsItem);
    const auto blockedApplicationsItem = d->pluginConfig->findItem("allowedApplications");
    Q_ASSERT(blockedApplicationsItem);

    Q_EMIT changed(blockedApplicationsItem->isSaveNeeded() && allowedApplicationsItem->isSaveNeeded());
    Q_EMIT defaulted(blockedApplicationsItem->isDefault() && allowedApplicationsItem->isDefault());
}

QVariant BlacklistedApplicationsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
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
        return application.icon.isEmpty() ? QStringLiteral("application-x-executable") : application.icon;

    case BlockedApplicationRole:
        return application.blocked;
    }
}

int BlacklistedApplicationsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->applications.size();
}

bool BlacklistedApplicationsModel::enabled() const
{
    return d->enabled;
}

void BlacklistedApplicationsModel::setEnabled(bool enabled)
{
    d->enabled = enabled;
    Q_EMIT enabledChanged(enabled);
}

// #include <BlacklistedApplicationsModel.moc>
