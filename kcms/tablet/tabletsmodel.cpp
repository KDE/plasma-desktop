/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    Work sponsored by Technische Universität Dresden:
    SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tabletsmodel.h"

#include <utility>

#include <QDBusInterface>

#include "inputdevice.h"
#include "logging.h"

extern "C" {
#include <libwacom/libwacom.h>
}

TabletsModel::TabletsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_deviceManager = new QDBusInterface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/org/kde/KWin/InputDevice"),
                                         QStringLiteral("org.kde.KWin.InputDeviceManager"),
                                         QDBusConnection::sessionBus(),
                                         this);

    resetModel();

    m_deviceManager->connection().connect(QStringLiteral("org.kde.KWin"),
                                          QStringLiteral("/org/kde/KWin/InputDevice"),
                                          QStringLiteral("org.kde.KWin.InputDeviceManager"),
                                          QStringLiteral("deviceAdded"),
                                          this,
                                          SLOT(onDeviceAdded(QString)));
    m_deviceManager->connection().connect(QStringLiteral("org.kde.KWin"),
                                          QStringLiteral("/org/kde/KWin/InputDevice"),
                                          QStringLiteral("org.kde.KWin.InputDeviceManager"),
                                          QStringLiteral("deviceRemoved"),
                                          this,
                                          SLOT(onDeviceRemoved(QString)));
}

void TabletsModel::resetModel()
{
    beginResetModel();
    m_devices.clear();

    QStringList devicesSysNames;
    const QVariant reply = m_deviceManager->property("devicesSysNames");
    if (reply.isValid()) {
        devicesSysNames = reply.toStringList();
    } else {
        qWarning() << "Error on receiving device list from KWin.";
        return;
    }

    for (const QString &sysname : std::as_const(devicesSysNames)) {
        addDevice(sysname, false);
    }
    endResetModel();
}

QVariant TabletsModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid) || index.column() != 0) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        auto db = libwacom_database_new();

        const QString sysPath = QStringLiteral("/dev/input/%1").arg(m_devices.at(index.row()).penDevice->sysName());

        WacomError *error = libwacom_error_new();
        auto device = libwacom_new_from_path(db, sysPath.toLatin1().constData(), WFALLBACK_NONE, error);

        if (device) {
            return libwacom_get_name(device);
        } else {
            return QString(m_devices.at(index.row()).padDevice->name() + QLatin1String(" and ") + m_devices.at(index.row()).penDevice->name());
        }
    }
    return {};
}

int TabletsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_devices.size();
}

void TabletsModel::onDeviceAdded(const QString &sysName)
{
    if (std::any_of(m_devices.cbegin(), m_devices.cend(), [sysName](auto &t) {
            return t.penDevice->sysName() == sysName || t.padDevice->sysName() == sysName;
        })) {
        return;
    }

    addDevice(sysName, true);
}

void TabletsModel::addDevice(const QString &sysName, bool tellModel)
{
    QDBusInterface deviceIface(QStringLiteral("org.kde.KWin"),
                               QStringLiteral("/org/kde/KWin/InputDevice/") + sysName,
                               QStringLiteral("org.kde.KWin.InputDevice"),
                               QDBusConnection::sessionBus(),
                               this);
    QVariant reply = deviceIface.property("tabletTool");
    QVariant reply2 = deviceIface.property("tabletPad");
    if (reply.isValid() && (reply.toBool() || reply2.toBool())) {
        auto dev = std::make_unique<InputDevice>(sysName, this);
        connect(dev.get(), &InputDevice::needsSaveChanged, this, &TabletsModel::needsSaveChanged);

        qDebug().nospace() << "Device connected: " << dev->name() << " (" << dev->sysName() << ")";

        QString deviceGroup = dev->deviceGroup();

        auto it = std::find_if(m_devices.begin(), m_devices.end(), [deviceGroup](const TabletDevice &device) {
            return device.deviceGroup == deviceGroup;
        });

        if (it != m_devices.end()) {
            if (dev->tabletTool()) {
                it->penDevice = std::move(dev);
            } else if (dev->tabletPad()) {
                it->padDevice = std::move(dev);
            }
        } else {
            TabletDevice tablet;
            tablet.deviceGroup = deviceGroup;

            if (dev->tabletTool()) {
                tablet.penDevice = std::move(dev);
            } else if (dev->tabletPad()) {
                tablet.padDevice = std::move(dev);
            }

            if (tellModel) {
                beginInsertRows({}, m_devices.size(), m_devices.size());
            }
            m_devices.push_back(std::move(tablet));
            if (tellModel) {
                endInsertRows();
            }
        }
    }
}

void TabletsModel::onDeviceRemoved(const QString &sysName)
{
    auto it = std::find_if(m_devices.cbegin(), m_devices.cend(), [sysName](auto &t) {
        return t.penDevice->sysName() == sysName || t.padDevice->sysName() == sysName;
    });
    if (it == m_devices.cend()) {
        return;
    }

    int index = std::distance(m_devices.cbegin(), it);

    beginRemoveRows({}, index, index);
    m_devices.erase(it);
    endRemoveRows();
}

QHash<int, QByteArray> TabletsModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"},
    };
}
