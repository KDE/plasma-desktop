/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "excludedapplicationsmodel.h"

#include <QScopeGuard>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <KService>

using namespace Qt::StringLiterals;

ExcludedApplicationsModel::ExcludedApplicationsModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_settings(new KActivityManagerdPluginsSettings(this))
{
}

ExcludedApplicationsModel::~ExcludedApplicationsModel()
{
}

QHash<int, QByteArray> ExcludedApplicationsModel::roleNames() const
{
    return {{ApplicationIdRole, "name"_ba}, {Qt::DecorationRole, "iconSource"_ba}, {Qt::DisplayRole, "title"_ba}, {BlockedApplicationRole, "blocked"_ba}};
}

void ExcludedApplicationsModel::load()
{
    QSqlDatabase database;

    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kactivitymanagerd/resources/database"_L1;
    database = QSqlDatabase::addDatabase("QSQLITE"_L1, "plugins_sqlite_db_resources"_L1);
    database.setDatabaseName(path);
    auto databaseRemove = qScopeGuard([&] {
        QSqlDatabase::removeDatabase(database.connectionName());
    });

    // Loading plugin configuration
    m_settings->load();
    const auto defaultBlockedValue = m_settings->defaultBlockedByDefaultValue();
    const auto &blockedAppList = m_settings->blockedApplications();
    auto blockedApplications = QSet<QString>(blockedAppList.cbegin(), blockedAppList.cend());
    const auto &allowedAppList = m_settings->allowedApplications();
    auto allowedApplications = QSet<QString>(allowedAppList.cbegin(), allowedAppList.cend());

    // Reading new applications from the database
    if (!database.open()) {
        // qDebug() << "Failed to open the database" << path << d->database.lastError();
        return;
    }

    QSqlQuery query(database);
    query.exec("SELECT DISTINCT(initiatingAgent) FROM ResourceScoreCache ORDER BY initiatingAgent"_L1);

    if (m_applications.length() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_applications.length() - 1);
        m_applications.clear();
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

    QStringList applications = (blockedApplications + allowedApplications).values();

    if (applications.length() > 0) {
        std::sort(applications.begin(), applications.end());

        beginInsertRows(QModelIndex(), 0, applications.length() - 1);

        for (const auto &name : std::as_const(applications)) {
            const auto service = KService::serviceByDesktopName(name);
            const auto blocked = blockedApplications.contains(name);

            if (service) {
                m_applications << ApplicationData{name, service->name(), service->icon(), blocked};
            } else {
                m_applications << ApplicationData{name, name, QString(), blocked};
            }
        }

        endInsertRows();
    }
}

void ExcludedApplicationsModel::save()
{
    m_settings->save();
    Q_EMIT changed(false);
}

void ExcludedApplicationsModel::defaults()
{
    for (int i = 0; i < rowCount(); i++) {
        m_applications[i].blocked = false;
    }

    Q_EMIT dataChanged(QAbstractListModel::index(0), QAbstractListModel::index(rowCount() - 1));

    Q_EMIT defaulted(true);
}

void ExcludedApplicationsModel::toggleApplicationBlocked(int index)
{
    if (index > rowCount()) {
        return;
    }

    m_applications[index].blocked = !m_applications[index].blocked;
    Q_EMIT dataChanged(QAbstractListModel::index(index), QAbstractListModel::index(index));

    QStringList blockedApplications;
    QStringList allowedApplications;

    for (int i = 0; i < rowCount(); i++) {
        const auto name = m_applications[i].name;
        if (m_applications[i].blocked) {
            blockedApplications << name;
        } else {
            allowedApplications << name;
        }
    }

    m_settings->setBlockedApplications(blockedApplications);
    m_settings->setAllowedApplications(allowedApplications);

    const auto allowedApplicationsItem = m_settings->findItem("allowedApplications");
    Q_ASSERT(allowedApplicationsItem);
    const auto blockedApplicationsItem = m_settings->findItem("allowedApplications");
    Q_ASSERT(blockedApplicationsItem);

    Q_EMIT changed(blockedApplicationsItem->isSaveNeeded() && allowedApplicationsItem->isSaveNeeded());
    Q_EMIT defaulted(blockedApplicationsItem->isDefault() && allowedApplicationsItem->isDefault());
}

QVariant ExcludedApplicationsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

QVariant ExcludedApplicationsModel::data(const QModelIndex &modelIndex, int role) const
{
    const auto index = modelIndex.row();

    if (index > rowCount()) {
        return QVariant();
    }

    const auto &application = m_applications[index];

    switch (role) {
    default:
        return QVariant();

    case ApplicationIdRole:
        return application.name;

    case Qt::DisplayRole:
        return application.title;

    case Qt::DecorationRole:
        return application.icon.isEmpty() ? "application-x-executable"_L1 : application.icon;

    case BlockedApplicationRole:
        return application.blocked;
    }
}

int ExcludedApplicationsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_applications.size();
}

#include "moc_excludedapplicationsmodel.cpp"
