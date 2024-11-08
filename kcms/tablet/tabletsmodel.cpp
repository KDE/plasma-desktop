/*
    SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tabletsmodel.h"

#include <utility>

#include "inputdevice.h"
#include "logging.h"

extern "C" {
#include <libwacom/libwacom.h>
}

TabletsModel::TabletsModel(WacomDeviceDatabase *db, QObject *parent)
    : QAbstractListModel(parent)
{
    m_db = db;

    resetModel();

    QDBusConnection::sessionBus().connect(QStringLiteral("org.kde.KWin"),
                                          QStringLiteral("/org/kde/KWin/InputDevice"),
                                          QStringLiteral("org.kde.KWin.InputDeviceManager"),
                                          QStringLiteral("deviceAdded"),
                                          this,
                                          SLOT(onDeviceAdded(QString)));
    QDBusConnection::sessionBus().connect(QStringLiteral("org.kde.KWin"),
                                          QStringLiteral("/org/kde/KWin/InputDevice"),
                                          QStringLiteral("org.kde.KWin.InputDeviceManager"),
                                          QStringLiteral("deviceRemoved"),
                                          this,
                                          SLOT(onDeviceRemoved(QString)));
}

void TabletsModel::resetModel()
{
    auto message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                  QStringLiteral("/org/kde/KWin/InputDevice"),
                                                  QStringLiteral("org.freedesktop.DBus.Properties"),
                                                  QStringLiteral("Get"));
    message << "org.kde.KWin.InputDeviceManager"
            << "devicesSysNames";
    QDBusConnection::sessionBus().callWithCallback(message, this, SLOT(loadReply(QDBusMessage)));
}

QVariant TabletsModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid) || index.column() != 0) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        const auto &tablet = m_devices.at(index.row());

        const bool hasPenDevice = tablet.penDevice != nullptr;
        const bool hasPadDevice = tablet.padDevice != nullptr;

        QString sysPath;
        // Prefer grabbing the sysPath from the pen, not the pad (if available)
        if (hasPenDevice) {
            sysPath = QStringLiteral("/dev/input/%1").arg(tablet.penDevice->sysName());
        } else if (hasPadDevice) {
            sysPath = QStringLiteral("/dev/input/%1").arg(tablet.padDevice->sysName());
        } else {
            Q_UNREACHABLE();
        }

        WacomError *error = libwacom_error_new();
        auto device = libwacom_new_from_path(m_db, sysPath.toLatin1().constData(), WFALLBACK_NONE, error);
        if (device == nullptr) {
            qCWarning(KCM_TABLET()) << "Failed to find device in libwacom:" << libwacom_error_get_message(error);
            libwacom_error_free(&error);
        } else {
            libwacom_error_free(&error);
            return libwacom_get_name(device);
        }

        if (hasPenDevice) {
            return m_devices.at(index.row()).penDevice->name();
        }
        if (hasPadDevice) {
            return m_devices.at(index.row()).padDevice->name();
        }

        Q_UNREACHABLE();
    }
    return {};
}

int TabletsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_devices.size();
}

void TabletsModel::load()
{
    for (auto &device : m_devices) {
        if (device.penDevice) {
            device.penDevice->load();
        }
        if (device.padDevice) {
            device.padDevice->load();
        }
    }
}

void TabletsModel::save()
{
    for (auto &device : m_devices) {
        if (device.penDevice) {
            device.penDevice->save();
        }
        if (device.padDevice) {
            device.padDevice->save();
        }
    }
}

void TabletsModel::defaults()
{
    for (auto &device : m_devices) {
        if (device.penDevice) {
            device.penDevice->defaults();
        }
        if (device.padDevice) {
            device.padDevice->defaults();
        }
    }
}

bool TabletsModel::isSaveNeeded() const
{
    return std::any_of(m_devices.cbegin(), m_devices.cend(), [](auto &dev) {
        bool isSaveNeeded = false;
        if (dev.penDevice) {
            isSaveNeeded |= dev.penDevice->isSaveNeeded();
        }
        if (dev.padDevice) {
            isSaveNeeded |= dev.padDevice->isSaveNeeded();
        }
        return isSaveNeeded;
    });
}

bool TabletsModel::isDefaults() const
{
    return std::any_of(m_devices.cbegin(), m_devices.cend(), [](auto &dev) {
        bool isDefaults = false;
        if (dev.penDevice) {
            isDefaults |= dev.penDevice->isDefaults();
        }
        if (dev.padDevice) {
            isDefaults |= dev.padDevice->isDefaults();
        }
        return isDefaults;
    });
}

void TabletsModel::onDeviceAdded(const QString &sysName)
{
    if (std::any_of(m_devices.cbegin(), m_devices.cend(), [sysName](auto &t) {
            if (t.penDevice && t.penDevice->sysName() == sysName) {
                return true;
            }
            if (t.padDevice && t.padDevice->sysName() == sysName) {
                return true;
            }
            return false;
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
        auto dev = new InputDevice(sysName, this);
        connect(dev, &InputDevice::needsSaveChanged, this, &TabletsModel::needsSaveChanged);

        QString deviceGroup = dev->deviceGroup();

        auto it = std::find_if(m_devices.begin(), m_devices.end(), [deviceGroup](const TabletDevice &device) {
            return device.deviceGroup == deviceGroup;
        });

        if (it != m_devices.end()) {
            if (dev->tabletTool()) {
                qCInfo(KCM_TABLET) << "Adding a tablet pen to an existing thing.";
                it->penDevice = std::move(dev);
            } else if (dev->tabletPad()) {
                qCInfo(KCM_TABLET) << "Adding a tablet pad to an existing thing.";
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
        if (t.penDevice && t.penDevice->sysName() == sysName) {
            return true;
        }
        if (t.padDevice && t.padDevice->sysName() == sysName) {
            return true;
        }
        return false;
    });
    if (it == m_devices.cend()) {
        return;
    }

    int index = std::distance(m_devices.cbegin(), it);

    beginRemoveRows({}, index, index);
    if (it->penDevice) {
        it->penDevice->deleteLater();
    }
    if (it->padDevice) {
        it->padDevice->deleteLater();
    }
    m_devices.erase(it);
    endRemoveRows();

    Q_EMIT deviceRemoved(sysName);
}

void TabletsModel::loadReply(QDBusMessage reply)
{
    beginResetModel();
    m_devices.clear();

    QStringList devicesSysNames;
    if (reply.type() == QDBusMessage::ReplyMessage) {
        devicesSysNames = reply.arguments().first().value<QDBusVariant>().variant().toStringList();
    } else {
        qCWarning(KCM_TABLET) << "Error on receiving device list from KWin:" << reply.errorMessage();
        return;
    }

    for (const QString &sysname : std::as_const(devicesSysNames)) {
        addDevice(sysname, false);
    }
    endResetModel();
}

InputDevice *TabletsModel::penAt(int row) const
{
    if (row == -1) {
        return nullptr;
    }

    return m_devices[row].penDevice;
}

InputDevice *TabletsModel::padAt(int row) const
{
    if (row == -1) {
        return nullptr;
    }

    return m_devices[row].padDevice;
}

QHash<int, QByteArray> TabletsModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"},
    };
}
