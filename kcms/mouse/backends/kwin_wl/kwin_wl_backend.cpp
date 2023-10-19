/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwin_wl_backend.h"
#include "kwin_wl_device.h"

#include <algorithm>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QKeySequence>
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
    const QVariant replyDevicesSysNames = m_deviceManager->property("devicesSysNames");
    if (replyDevicesSysNames.isValid()) {
        qCDebug(KCM_MOUSE) << "Devices list received successfully from KWin.";
        devicesSysNames = replyDevicesSysNames.toStringList();
    } else {
        qCCritical(KCM_MOUSE) << "Error on receiving device list from KWin.";
        m_errorString = i18n("Querying input devices failed. Please reopen this settings module.");
        return;
    }

    for (const QString &sn : std::as_const(devicesSysNames)) {
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
    KConfigGroup buttonGroup = KSharedConfig::openConfig("kcminputrc")->group("ButtonRebinds").group("Mouse");
    for (auto it = m_buttonMapping.cbegin(); it != m_buttonMapping.cend(); ++it) {
        if (auto keys = it.value().value<QKeySequence>(); !keys.isEmpty()) {
            buttonGroup.writeEntry(it.key(), QStringList{"Key", keys.toString(QKeySequence::PortableText)}, KConfig::Notify);
        } else {
            buttonGroup.deleteEntry(it.key(), KConfig::Notify);
        }
    }

    return std::all_of(m_devices.constBegin(), m_devices.constEnd(), [](QObject *t) {
        return static_cast<KWinWaylandDevice *>(t)->applyConfig();
    });
}

bool KWinWaylandBackend::getConfig()
{
    m_loadedButtonMapping.clear();
    const KConfigGroup buttonGroup = KSharedConfig::openConfig("kcminputrc")->group("ButtonRebinds").group("Mouse");
    for (int i = 1; i <= 24; ++i) {
        const QString buttonName = QLatin1String("ExtraButton%1").arg(QString::number(i));
        auto entry = buttonGroup.readEntry(buttonName, QStringList());
        if (entry.size() == 2 && entry.first() == QLatin1String("Key")) {
            auto keys = QKeySequence::fromString(entry.at(1), QKeySequence::PortableText);
            if (!keys.isEmpty()) {
                m_loadedButtonMapping.insert(buttonName, keys);
            }
        }
    }
    m_buttonMapping = m_loadedButtonMapping;

    return std::all_of(m_devices.constBegin(), m_devices.constEnd(), [](QObject *t) {
        return static_cast<KWinWaylandDevice *>(t)->init();
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
    return m_buttonMapping != m_loadedButtonMapping || std::any_of(m_devices.constBegin(), m_devices.constEnd(), [](QObject *t) {
        return static_cast<KWinWaylandDevice *>(t)->isChangedConfig();
    });
}

QVariantMap KWinWaylandBackend::buttonMapping()
{
    return m_buttonMapping;
}

void KWinWaylandBackend::setButtonMapping(const QVariantMap &mapping)
{
    if (m_buttonMapping != mapping) {
        m_buttonMapping = mapping;
        Q_EMIT buttonMappingChanged();
    }
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
        if (!dev->init()) {
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
    QList<QObject *>::const_iterator it = std::find_if(m_devices.constBegin(), m_devices.constEnd(), [sysName](QObject *t) {
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
