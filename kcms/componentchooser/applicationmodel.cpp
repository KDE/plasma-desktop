/*
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "applicationmodel.h"

#include <KApplicationTrader>
#include <KLocalizedString>
#include <KService>

#include <optional>

#include "componentchooser.h"

ApplicationModel::ApplicationModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

void ApplicationModel::addApplication(const QString &name, const QString &iconName, const QString &storageId, bool isSelected, const QString &execLine)
{
    QVariantMap application;
    application[QStringLiteral("name")] = name;
    application[QStringLiteral("icon")] = iconName;
    application[QStringLiteral("storageId")] = storageId;
    application[QStringLiteral("isSelected")] = isSelected;
    application[QStringLiteral("execLine")] = execLine;
    m_applications += application;
}

void ApplicationModel::load(const QString &mimeType,
                            const QString &applicationCategory,
                            const QString &defaultApplication,
                            const KService::Ptr preferredService)
{
    beginResetModel();

    m_applications.clear();

    if (preferredService) {
        addApplication(preferredService->name(), preferredService->icon(), preferredService->storageId(), true, preferredService->exec());
        if (preferredService->storageId() == defaultApplication) {
            m_defaultIndex = 0;
        }
    }

    KApplicationTrader::query([preferredService, applicationCategory, mimeType, defaultApplication, this](const KService::Ptr &service) {
        if (service->exec().isEmpty() || (!applicationCategory.isEmpty() && !service->categories().contains(applicationCategory))
            || (!mimeType.isEmpty() && !ComponentChooser::serviceSupportsMimeType(service, mimeType))
            || (preferredService && preferredService->storageId() == service->storageId())) {
            return false;
        }

        const auto icon = service->icon().isEmpty() ? QStringLiteral("application-x-shellscript") : service->icon();
        bool isDefault = service->storageId() == defaultApplication;

        addApplication(service->name(), icon, service->storageId(), false, service->exec());

        if (isDefault) {
            m_defaultIndex = m_applications.length() - 1;
        }
        return false;
    });

    addApplication(i18n("Other…"), QStringLiteral("application-x-shellscript"), QString(), false, QString());

    endResetModel();
}

int ApplicationModel::currentIndex() const
{
    int index = 0;
    for (const auto &application : std::as_const(m_applications)) {
        if (application["isSelected"].toBool()) {
            return index;
        }
        ++index;
    }

    if (m_defaultIndex != -1) {
        return m_defaultIndex;
    }

    return 0;
}

std::optional<int> ApplicationModel::defaultIndex() const
{
    if (m_defaultIndex == -1)
        return {};
    else {
        return m_defaultIndex;
    }
}

int ApplicationModel::addApplicationBeforeLast(const KServicePtr service)
{
    // the application will grow by 1, we want to add the new application at the before
    // last position taking into account the future new size of m_applications
    int newRowIndex = rowCount() - 1;
    beginInsertRows(QModelIndex(), newRowIndex, newRowIndex);

    addApplication(service->name(), service->icon(), service->storageId(), false, service->exec());
    // addApplication adds at the end of of the list
    // swaps the last and before last applications
    m_applications.swapItemsAt(rowCount() - 2, rowCount() - 1);

    endInsertRows();

    return newRowIndex;
}

QVariant ApplicationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)

    return QVariant();
}

QModelIndex ApplicationModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    if (row >= 0 && row < m_applications.length()) {
        return createIndex(row, column);
    }
    return QModelIndex();
}

QModelIndex ApplicationModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int ApplicationModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_applications.length();
}

int ApplicationModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    return 1;
}

QVariant ApplicationModel::data(const QModelIndex &index, int role) const
{
    if (!isValid(index))
        return QVariant();

    int row = index.row();
    const auto map = m_applications.at(row);
    switch (role) {
    case Qt::DisplayRole:
        return map["name"];
    case Icon:
        return map["icon"];
    case StorageId:
        return map["storageId"];
    case Selected:
        return map["isSelected"];
    case ExecLine:
        return map["execLine"];
    }

    return QVariant();
}

QVariant ApplicationModel::data(const int &row, int role) const
{
    return data(index(row, 0), role);
}

bool ApplicationModel::isValid(const QModelIndex &index) const
{
    return index.column() == 0 && index.row() >= 0 && index.row() < m_applications.length();
}

bool ApplicationModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Selected) {
        return false;
    }
    if (!isValid(index)) {
        return false;
    }
    if (!value.canConvert(QMetaType(QMetaType::Type::Bool)) && !value.toBool()) {
        return false;
    }

    for (auto &application : m_applications) {
        application["isSelected"] = false;
    }

    const auto row = index.row();

    // auto map = m_applications[row].toMap();
    m_applications[row]["isSelected"] = true;
    // m_applications[row] = map;

    Q_EMIT dataChanged(index, index, {role});

    return true;
}

QModelIndex ApplicationModel::findByStorageId(const QString &storageId) const
{
    int i = 0;
    for (const auto &application : std::as_const(m_applications)) {
        if (application["storageId"] == storageId) {
            return index(i, 0);
        }
        ++i;
    }
    return QModelIndex();
}

QHash<int, QByteArray> ApplicationModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "name"},
        {Icon, "icon"},
        {StorageId, "storageId"},
        {Selected, "isSelected"},
        {ExecLine, "execLine"},
    };
}
