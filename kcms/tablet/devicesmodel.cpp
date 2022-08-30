/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicesmodel.h"
#include "inputdevice.h"
#include <QDBusInterface>

#include "logging.h"

DevicesModel::DevicesModel(const QByteArray &kind, QObject *parent)
    : QAbstractListModel(parent)
    , m_kind(kind)
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

void DevicesModel::resetModel()
{
    beginResetModel();
    qDeleteAll(m_devices);
    m_devices.clear();

    QStringList devicesSysNames;
    const QVariant reply = m_deviceManager->property("devicesSysNames");
    if (reply.isValid()) {
        qCDebug(KCM_TABLET) << "Devices list received successfully from KWin.";
        devicesSysNames = reply.toStringList();
    } else {
        qCCritical(KCM_TABLET) << "Error on receiving device list from KWin.";
        return;
    }

    for (const QString &sysname : devicesSysNames) {
        addDevice(sysname, false);
    }
    endResetModel();
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid) || index.column() != 0)
        return {};

    switch (role) {
    case Qt::DisplayRole:
        return m_devices.at(index.row())->name();
    case Qt::UserRole:
        return QVariant::fromValue<QObject *>(m_devices.at(index.row()));
    }
    return {};
}

int DevicesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_devices.count();
}

void DevicesModel::onDeviceAdded(const QString &sysName)
{
    if (std::any_of(m_devices.constBegin(), m_devices.constEnd(), [sysName](InputDevice *t) {
            return t->sysName() == sysName;
        })) {
        return;
    }

    addDevice(sysName, true);
}

void DevicesModel::addDevice(const QString &sysName, bool tellModel)
{
    QDBusInterface deviceIface(QStringLiteral("org.kde.KWin"),
                               QStringLiteral("/org/kde/KWin/InputDevice/") + sysName,
                               QStringLiteral("org.kde.KWin.InputDevice"),
                               QDBusConnection::sessionBus(),
                               this);
    QVariant reply = deviceIface.property(m_kind);
    if (reply.isValid() && reply.toBool()) {
        InputDevice *dev = new InputDevice(sysName, this);
        connect(dev, &InputDevice::needsSaveChanged, this, &DevicesModel::needsSaveChanged);

        if (tellModel) {
            beginInsertRows({}, m_devices.count(), m_devices.count());
        }
        m_devices.append(dev);
        qCDebug(KCM_TABLET).nospace() << "Device connected: " << dev->name() << " (" << dev->sysName() << ")";
        if (tellModel) {
            endInsertRows();
        }
    }
}

void DevicesModel::onDeviceRemoved(const QString &sysName)
{
    QVector<InputDevice *>::const_iterator it = std::find_if(m_devices.constBegin(), m_devices.constEnd(), [sysName](InputDevice *t) {
        return t->sysName() == sysName;
    });
    if (it == m_devices.cend()) {
        return;
    }

    InputDevice *dev = static_cast<InputDevice *>(*it);
    qCDebug(KCM_TABLET).nospace() << "Device disconnected: " << dev->name() << " (" << dev->sysName() << ")";

    int index = m_devices.indexOf(*it);

    beginRemoveRows({}, index, index);
    m_devices.removeAt(index);
    endInsertRows();
}

InputDevice *DevicesModel::deviceAt(int row) const
{
    return m_devices.value(row, nullptr);
}

QHash<int, QByteArray> DevicesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"},
    };
}

void DevicesModel::defaults()
{
    for (auto device : qAsConst(m_devices)) {
        device->defaults();
    }
}

void DevicesModel::load()
{
    for (auto device : qAsConst(m_devices)) {
        device->load();
    }
}

void DevicesModel::save()
{
    for (auto device : qAsConst(m_devices)) {
        device->save();
    }
}

bool DevicesModel::isDefaults() const
{
    return std::all_of(m_devices.constBegin(), m_devices.constEnd(), [](InputDevice *dev) {
        return dev->isDefaults();
    });
}

bool DevicesModel::isSaveNeeded() const
{
    return std::any_of(m_devices.constBegin(), m_devices.constEnd(), [](InputDevice *dev) {
        return dev->isSaveNeeded();
    });
}
