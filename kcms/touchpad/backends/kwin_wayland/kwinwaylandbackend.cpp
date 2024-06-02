/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwinwaylandbackend.h"
#include "kwinwaylandtouchpad.h"

#include <algorithm>

#include <KLocalizedString>

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QStringList>

#include "logging.h"

KWinWaylandBackend::KWinWaylandBackend(QObject *parent)
    : TouchpadBackend(parent)
{
    m_deviceManager = new QDBusInterface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/org/kde/KWin/InputDevice"),
                                         QStringLiteral("org.kde.KWin.InputDeviceManager"),
                                         QDBusConnection::sessionBus(),
                                         this);

    setMode(TouchpadInputBackendMode::WaylandLibinput);

    findTouchpads();

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

void KWinWaylandBackend::findTouchpads()
{
    const QVariant reply = m_deviceManager->property("devicesSysNames");
    if (!reply.isValid()) {
        qCCritical(KCM_TOUCHPAD) << "Error on receiving device list from KWin.";
        m_errorString = i18n("Querying input devices failed. Please reopen this settings module.");
        return;
    }
    const auto devicesSysNames = reply.toStringList();
    for (const QString &sn : devicesSysNames) {
        QDBusInterface deviceIface(QStringLiteral("org.kde.KWin"),
                                   QStringLiteral("/org/kde/KWin/InputDevice/") + sn,
                                   QStringLiteral("org.kde.KWin.InputDevice"),
                                   QDBusConnection::sessionBus(),
                                   this);
        const QVariant reply = deviceIface.property("touchpad");
        if (reply.isValid() && reply.toBool()) {
            KWinWaylandTouchpad *tp = new KWinWaylandTouchpad(sn);
            if (!tp->init() || !tp->getConfig()) {
                qCCritical(KCM_TOUCHPAD) << "Error on creating touchpad object" << sn;
                m_errorString = i18n("Critical error on reading fundamental device infos for touchpad %1.", sn);
                return;
            }
            m_devices.append(tp);
            qCInfo(KCM_TOUCHPAD).nospace() << "Touchpad found: " << tp->name() << " (" << tp->sysName() << ")";
        }
    }
    if (m_devices.isEmpty()) {
        qCInfo(KCM_TOUCHPAD) << "No Devices found.";
    }
}

bool KWinWaylandBackend::applyConfig()
{
    return std::all_of(m_devices.constBegin(), m_devices.constEnd(), [](QObject *t) {
        return static_cast<KWinWaylandTouchpad *>(t)->applyConfig();
    });
}

bool KWinWaylandBackend::getConfig()
{
    return std::all_of(m_devices.constBegin(), m_devices.constEnd(), [](QObject *t) {
        return static_cast<KWinWaylandTouchpad *>(t)->getConfig();
    });
}

bool KWinWaylandBackend::getDefaultConfig()
{
    return std::all_of(m_devices.constBegin(), m_devices.constEnd(), [](QObject *t) {
        return static_cast<KWinWaylandTouchpad *>(t)->getDefaultConfig();
    });
}

bool KWinWaylandBackend::isChangedConfig() const
{
    return std::any_of(m_devices.constBegin(), m_devices.constEnd(), [](QObject *t) {
        return static_cast<KWinWaylandTouchpad *>(t)->isChangedConfig();
    });
}

void KWinWaylandBackend::onDeviceAdded(QString sysName)
{
    if (std::any_of(m_devices.constBegin(), m_devices.constEnd(), [sysName](QObject *t) {
            return static_cast<KWinWaylandTouchpad *>(t)->sysName() == sysName;
        })) {
        return;
    }

    QDBusInterface deviceIface(QStringLiteral("org.kde.KWin"),
                               QStringLiteral("/org/kde/KWin/InputDevice/") + sysName,
                               QStringLiteral("org.kde.KWin.InputDevice"),
                               QDBusConnection::sessionBus(),
                               this);
    QVariant reply = deviceIface.property("touchpad");
    if (reply.isValid() && reply.toBool()) {
        KWinWaylandTouchpad *tp = new KWinWaylandTouchpad(sysName);
        if (!tp->init() || !tp->getConfig()) {
            Q_EMIT touchpadAdded(false);
            return;
        }
        m_devices.append(tp);
        qCDebug(KCM_TOUCHPAD).nospace() << "Touchpad connected: " << tp->name() << " (" << tp->sysName() << ")";
        Q_EMIT touchpadAdded(true);
    }
}

void KWinWaylandBackend::onDeviceRemoved(QString sysName)
{
    QList<QObject *>::const_iterator it = std::find_if(m_devices.constBegin(), m_devices.constEnd(), [sysName](QObject *t) {
        return static_cast<KWinWaylandTouchpad *>(t)->sysName() == sysName;
    });
    if (it == m_devices.cend()) {
        return;
    }

    KWinWaylandTouchpad *tp = static_cast<KWinWaylandTouchpad *>(*it);
    qCDebug(KCM_TOUCHPAD).nospace() << "Touchpad disconnected: " << tp->name() << " (" << tp->sysName() << ")";

    int index = it - m_devices.cbegin();
    m_devices.removeAt(index);
    Q_EMIT touchpadRemoved(index);
}

#include "moc_kwinwaylandbackend.cpp"
