/*
    SPDX-FileCopyrightText: 2020-2025 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "virtualkeyboardmodel.h"
#include <KApplicationTrader>
#include <KLocalizedString>
#include <QStandardPaths>

VirtualKeyboardsModel::VirtualKeyboardsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_services = KApplicationTrader::query([](const KService::Ptr &service) {
        return service->property<bool>("X-KDE-Wayland-VirtualKeyboard");
    });

    m_services.prepend({});
}

QHash<int, QByteArray> VirtualKeyboardsModel::roleNames() const
{
    QHash<int, QByteArray> ret = QAbstractListModel::roleNames();
    ret.insert(DesktopFileNameRole, "desktopFileName");
    return ret;
}

QVariant VirtualKeyboardsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.parent().isValid() || index.row() > m_services.count()) {
        return {};
    }

    const KService::Ptr service = m_services[index.row()];
    switch (role) {
    case Qt::DisplayRole:
        return service ? service->name() : i18n("None");
    case Qt::DecorationRole:
        return service ? service->icon() : QStringLiteral("edit-none");
    case Qt::ToolTipRole:
        return service ? service->comment() : i18n("Do not use any virtual keyboard");
    case DesktopFileNameRole:
        return service ? QStandardPaths::locate(QStandardPaths::ApplicationsLocation, service->desktopEntryName() + QLatin1String(".desktop")) : QString();
    }
    return {};
}

int VirtualKeyboardsModel::inputMethodIndex(const QString &desktopFile) const
{
    if (desktopFile.isEmpty()) {
        return 0;
    }

    int i = 0;
    for (const auto &service : m_services) {
        if (service && desktopFile.endsWith(service->desktopEntryName() + QLatin1String(".desktop"))) {
            return i;
        }
        ++i;
    }
    return -1;
}

int VirtualKeyboardsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_services.count();
}
