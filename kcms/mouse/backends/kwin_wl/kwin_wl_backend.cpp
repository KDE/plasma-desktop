/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwin_wl_backend.h"
#include "kwin_wl_device.h"

#include <algorithm>

#include <KLocalizedString>

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QStringList>

#include "logging.h"

KWinWaylandBackend::KWinWaylandBackend(QObject *parent)
    : InputBackend(parent)
{
    m_mode = InputBackendMode::KWinWayland;

    m_deviceManager = new QDBusInterface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/org/kde/KWin/InputDevice"),
                                         QStringLiteral("org.kde.KWin.InputDeviceManager"),
                                         QDBusConnection::sessionBus(),
                                         this);

    findDevices();

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

KWinWaylandBackend::~KWinWaylandBackend()
{
    qDeleteAll(m_devices);
    delete m_deviceManager;
}

void KWinWaylandBackend::findDevices()
{
    QStringList devicesSysNames;
    const QVariant reply = m_deviceManager->property("devicesSysNames");
    if (reply.isValid()) {
        qCDebug(KCM_MOUSE) << "Devices list received successfully from KWin.";
        devicesSysNames = reply.toStringList();
    } else {
        qCCritical(KCM_MOUSE) << "Error on receiving device list from KWin.";
        m_errorString = i18n("Querying input devices failed. Please reopen this settings module.");
        return;
    }

    for (QString sn : devicesSysNames) {
        QDBusInterface deviceIface(QStringLiteral("org.kde.KWin"),
                                   QStringLiteral("/org/kde/KWin/InputDevice/") + sn,
                                   QStringLiteral("org.kde.KWin.InputDevice"),
                                   QDBusConnection::sessionBus(),
                                   this);
        QVariant reply = deviceIface.property("pointer");
        if (reply.isValid() && reply.toBool()) {
            reply = deviceIface.property("touchpad");
            if (reply.isValid() && reply.toBool()) {
                continue;
            }

            KWinWaylandDevice *dev = new KWinWaylandDevice(sn);
            if (!dev->init()) {
                qCCritical(KCM_MOUSE) << "Error on creating device object" << sn;
                m_errorString = i18n("Critical error on reading fundamental device infos of %1.", sn);
                return;
            }
            m_devices.append(dev);
            qCDebug(KCM_MOUSE).nospace() << "Device found: " << dev->name() << " (" << dev->sysName() << ")";
        }
    }
}

bool KWinWaylandBackend::applyConfig()
{
    return std::all_of(m_devices.constBegin(), m_devices.constEnd(), [](QObject *t) {
        return static_cast<KWinWaylandDevice *>(t)->applyConfig();
    });
}

bool KWinWaylandBackend::getConfig()
{
    return std::all_of(m_devices.constBegin(), m_devices.constEnd(), [](QObject *t) {
        return static_cast<KWinWaylandDevice *>(t)->getConfig();
    });
}

bool KWinWaylandBackend::getDefaultConfig()
{
    return std::all_of(m_devices.constBegin(), m_devices.constEnd(), [](QObject *t) {
        return static_cast<KWinWaylandDevice *>(t)->getDefaultConfig();
    });
}

bool KWinWaylandBackend::isChangedConfig() const
{
    return std::any_of(m_devices.constBegin(), m_devices.constEnd(), [](QObject *t) {
        return static_cast<KWinWaylandDevice *>(t)->isChangedConfig();
    });
}

void KWinWaylandBackend::onDeviceAdded(QString sysName)
{
    if (std::any_of(m_devices.constBegin(), m_devices.constEnd(), [sysName](QObject *t) {
            return static_cast<KWinWaylandDevice *>(t)->sysName() == sysName;
        })) {
        return;
    }

    QDBusInterface deviceIface(QStringLiteral("org.kde.KWin"),
                               QStringLiteral("/org/kde/KWin/InputDevice/") + sysName,
                               QStringLiteral("org.kde.KWin.InputDevice"),
                               QDBusConnection::sessionBus(),
                               this);
    QVariant reply = deviceIface.property("pointer");

    if (reply.isValid() && reply.toBool()) {
        reply = deviceIface.property("touchpad");
        if (reply.isValid() && reply.toBool()) {
            return;
        }

        KWinWaylandDevice *dev = new KWinWaylandDevice(sysName);
        if (!dev->init() || !dev->getConfig()) {
            Q_EMIT deviceAdded(false);
            return;
        }

        m_devices.append(dev);
        qCDebug(KCM_MOUSE).nospace() << "Device connected: " << dev->name() << " (" << dev->sysName() << ")";
        Q_EMIT deviceAdded(true);
    }
}

void KWinWaylandBackend::onDeviceRemoved(QString sysName)
{
    QVector<QObject *>::const_iterator it = std::find_if(m_devices.constBegin(), m_devices.constEnd(), [sysName](QObject *t) {
        return static_cast<KWinWaylandDevice *>(t)->sysName() == sysName;
    });
    if (it == m_devices.cend()) {
        return;
    }

    KWinWaylandDevice *dev = static_cast<KWinWaylandDevice *>(*it);
    qCDebug(KCM_MOUSE).nospace() << "Device disconnected: " << dev->name() << " (" << dev->sysName() << ")";

    int index = it - m_devices.cbegin();
    m_devices.removeAt(index);
    Q_EMIT deviceRemoved(index);
}
