/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2026 Kai Uwe Broulik <kde@broulik.de>

    Work sponsored by Technische Universität Dresden:
    SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicesmodel.h"

#include <utility>

#include <QDBusMessage>

#include "InputDevice_interface.h"
#include "inputdevice.h"
#include "kwindevices-logging.h"

namespace KWinDevices
{

static constexpr QLatin1String g_kwinService{"org.kde.KWin"};
static constexpr QLatin1String g_kwinInputDevicePath{"/org/kde/KWin/InputDevice"};
static constexpr QLatin1String g_kwinInputDeviceManagerInterface{"org.kde.KWin.InputDeviceManager"};

using namespace Qt::StringLiterals;

DevicesModel::DevicesModel(DevicesModel::Kind kind, QObject *parent)
    : DevicesModel(kind, {}, parent)
{
}

DevicesModel::DevicesModel(DevicesModel::Kind kind, const QVariantMap &filters, QObject *parent)
    : QAbstractListModel(parent)
    , m_kind(kind)
    , m_filters(filters)
{
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

    QString listMethod;
    switch (kind) {
    case Kind::Pointers:
        listMethod = u"ListPointers"_s;
        break;
    case Kind::Keyboards:
        listMethod = u"ListKeyboards"_s;
        break;
    case Kind::Touch:
        listMethod = u"ListTouch"_s;
        break;
    }

    auto msg = QDBusMessage::createMethodCall(g_kwinService, g_kwinInputDevicePath, g_kwinInputDeviceManagerInterface, listMethod);
    auto reply = QDBusConnection::sessionBus().call(msg);
    if (reply.type() == QDBusMessage::ErrorMessage || reply.arguments().isEmpty()) {
        qCWarning(LIBKWINDEVICES) << "Error getting device list from KWin" << reply.errorMessage();
        m_errorString = reply.errorMessage();
        return;
    }

    const QStringList deviceSysNames = reply.arguments().at(0).toStringList();
    for (const QString &sysName : deviceSysNames) {
        bool relevantDevice = true;
        if (!m_filters.isEmpty()) {
            org::kde::KWin::InputDevice deviceIface{g_kwinService, "/org/kde/KWin/InputDevice/"_L1 + sysName, QDBusConnection::sessionBus()};
            for (const auto &[key, value] : std::as_const(m_filters).asKeyValueRange()) {
                if (deviceIface.property(qUtf8Printable(key)) != value) {
                    relevantDevice = false;
                    break;
                }
            }
        }

        if (relevantDevice) {
            addDevice(sysName);
        }
    }
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid) || index.column() != 0) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        return m_devices.at(index.row())->name();
    case DeviceRole:
        return QVariant::fromValue<QObject *>(m_devices.at(index.row()).get());
    case SysNameRole:
        return m_deviceSysNames.at(index.row());
    }
    return {};
}

int DevicesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_devices.size();
}

void DevicesModel::onDeviceAdded(const QString &sysName)
{
    const int index = m_deviceSysNames.indexOf(sysName);
    if (index > -1) {
        return;
    }

    org::kde::KWin::InputDevice deviceIface{g_kwinService, "/org/kde/KWin/InputDevice/"_L1 + sysName, QDBusConnection::sessionBus()};

    switch (m_kind) {
    case Kind::Keyboards:
        if (!deviceIface.keyboard()) {
            return;
        }
        break;
    case Kind::Pointers:
        if (!deviceIface.pointer()) {
            return;
        }
        break;
    case Kind::Touch:
        if (!deviceIface.touch()) {
            return;
        }
        break;
    }

    for (const auto &[key, value] : std::as_const(m_filters).asKeyValueRange()) {
        if (deviceIface.property(qUtf8Printable(key)) != value) {
            return;
        }
    }

    beginInsertRows({}, m_devices.size(), m_devices.size());
    addDevice(sysName);
    endInsertRows();
}

void DevicesModel::addDevice(const QString &sysName)
{
    auto dev = std::make_unique<InputDevice>(sysName, this);
    connect(dev.get(), &InputDevice::needsSaveChanged, this, &DevicesModel::needsSaveChanged);
    qCDebug(LIBKWINDEVICES).nospace() << "Device connected:" << sysName;
    m_devices.push_back(std::move(dev));
    m_deviceSysNames.append(sysName);
}

void DevicesModel::onDeviceRemoved(const QString &sysName)
{
    const int index = m_deviceSysNames.indexOf(sysName);
    if (index == -1) {
        return;
    }

    qCDebug(LIBKWINDEVICES).nospace() << "Device disconnected:" << sysName;

    beginRemoveRows({}, index, index);
    m_devices.erase(m_devices.begin() + index);
    m_deviceSysNames.remove(index);
    endRemoveRows();

    Q_EMIT deviceRemoved(sysName);
}

InputDevice *DevicesModel::deviceAt(int row) const
{
    if (row == -1) {
        return nullptr;
    }

    return m_devices.at(row).get();
}

QHash<int, QByteArray> DevicesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {DeviceRole, QByteArrayLiteral("device")},
        {SysNameRole, QByteArrayLiteral("sysName")},
    };
}

void DevicesModel::defaults()
{
    for (auto &device : m_devices) {
        device->defaults();
    }
}

void DevicesModel::load()
{
    for (auto &device : m_devices) {
        device->load();
    }
}

void DevicesModel::save()
{
    for (auto &device : m_devices) {
        device->save();
    }
}

bool DevicesModel::isDefaults() const
{
    return std::all_of(m_devices.cbegin(), m_devices.cend(), [](auto &dev) {
        return dev->isDefaults();
    });
}

bool DevicesModel::isSaveNeeded() const
{
    return std::any_of(m_devices.cbegin(), m_devices.cend(), [](auto &dev) {
        return dev->isSaveNeeded();
    });
}

QString DevicesModel::errorString() const
{
    return m_errorString;
}

} // namespace KWinDevices

#include "moc_devicesmodel.cpp"
