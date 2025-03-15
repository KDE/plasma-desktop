/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwin_wl_backend.h"

#include <algorithm>

#include <KLocalizedString>
#include <KSharedConfig>

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QKeySequence>
#include <QStringList>

#include "logging.h"

KWinWaylandBackend::KWinWaylandBackend()
    : InputBackend()
    , m_deviceManager(new QDBusInterface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/org/kde/KWin/InputDevice"),
                                         QStringLiteral("org.kde.KWin.InputDeviceManager"),
                                         QDBusConnection::sessionBus(),
                                         this))
{
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

            auto dev = std::make_unique<KWinWaylandDevice>(sn);
            if (!dev->initDBus()) {
                qCCritical(KCM_MOUSE) << "Error on creating device object" << sn;
                m_errorString = i18n("Critical error on reading fundamental device infos of %1.", sn);
                return;
            }
            loadButtonRebinds(dev.get(), mouseButtonRebindsConfigGroup());
            connect(dev.get(), &KWinWaylandDevice::needsSaveChanged, this, &InputBackend::needsSaveChanged);
            qCDebug(KCM_MOUSE).nospace() << "Device found: " << dev->name() << " (" << dev->sysName() << ")";
            m_devices.push_back(std::move(dev));
        }
    }
}

bool KWinWaylandBackend::forAllDevices(bool (KWinWaylandDevice::*f)()) const
{
    bool ok = true;
    for (const auto &device : std::as_const(m_devices)) {
        ok &= (device.get()->*f)();
    }
    return ok;
}

KConfigGroup KWinWaylandBackend::mouseButtonRebindsConfigGroup()
{
    return KSharedConfig::openConfig(u"kcminputrc"_s)->group(u"ButtonRebinds"_s).group(u"Mouse"_s);
}

bool KWinWaylandBackend::save()
{
    KConfigGroup buttonGroup = mouseButtonRebindsConfigGroup();

    for (const auto &device : std::as_const(m_devices)) {
        KConfigGroup deviceGroup = buttonGroup.group(device.get()->name());
        const auto mapping = device.get()->buttonMapping();

        for (const auto &[buttonName, variant] : mapping.asKeyValueRange()) {
            if (const auto keySequence = variant.value<QKeySequence>(); !keySequence.isEmpty()) {
                const auto value = QStringList{u"Key"_s, keySequence.toString(QKeySequence::PortableText)};
                deviceGroup.writeEntry(buttonName, value, KConfig::Notify);
            } else {
                deviceGroup.deleteEntry(buttonName, KConfig::Notify);
            }
        }
    }
    // We copied these global keys to each device on load, so we can clear out
    // any global keys now.
    for (const QString &key : buttonGroup.keyList()) {
        buttonGroup.deleteEntry(key);
    }

    return forAllDevices(&KWinWaylandDevice::saveDBus);
}

void KWinWaylandBackend::loadButtonRebinds(KWinWaylandDevice *device, const KConfigGroup &buttonGroup)
{
    const auto loadButtonMapping = [device](const KConfigGroup &deviceGroup) {
        QVariantMap buttonMapping;
        const auto buttonNames = deviceGroup.keyList();

        for (const QString &buttonName : buttonNames) {
            const auto action = deviceGroup.readEntry(buttonName, QStringList());
            if (action.size() == 2 && action.first() == QLatin1String("Key")) {
                const auto keySequence = QKeySequence::fromString(action.at(1), QKeySequence::PortableText);
                if (!keySequence.isEmpty()) {
                    buttonMapping.insert(buttonName, keySequence);
                }
            }
        }
        device->initButtonMapping(buttonMapping);
    };

    const KConfigGroup deviceGroup = buttonGroup.group(device->name());
    if (deviceGroup.exists()) {
        loadButtonMapping(deviceGroup);
    } else {
        // Bindings config should be per-device now, so if global bindings exist,
        // migrate them from the parent group to each device we know of.
        loadButtonMapping(buttonGroup);
    }
}

bool KWinWaylandBackend::load()
{
    if (!forAllDevices(&KWinWaylandDevice::initDBus)) {
        return false;
    }

    const KConfigGroup buttonGroup = mouseButtonRebindsConfigGroup();
    for (const auto &device : std::as_const(m_devices)) {
        loadButtonRebinds(device.get(), buttonGroup);
    }

    return true;
}

bool KWinWaylandBackend::defaults()
{
    return forAllDevices(&KWinWaylandDevice::defaults);
}

bool KWinWaylandBackend::isSaveNeeded() const
{
    return std::ranges::any_of(std::as_const(m_devices), [](const std::unique_ptr<KWinWaylandDevice> &device) {
        return device->isSaveNeeded();
    });
}

void KWinWaylandBackend::onDeviceAdded(QString sysName)
{
    if (std::ranges::any_of(std::as_const(m_devices), [sysName](const std::unique_ptr<KWinWaylandDevice> &device) {
            return device->sysName() == sysName;
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

        auto dev = std::make_unique<KWinWaylandDevice>(sysName);
        if (!dev->initDBus()) {
            Q_EMIT deviceAdded(false);
            return;
        }

        loadButtonRebinds(dev.get(), mouseButtonRebindsConfigGroup());
        connect(dev.get(), &KWinWaylandDevice::needsSaveChanged, this, &InputBackend::needsSaveChanged);
        qCDebug(KCM_MOUSE).nospace() << "Device connected: " << dev->name() << " (" << dev->sysName() << ")";
        m_devices.push_back(std::move(dev));
        Q_EMIT deviceAdded(true);
        Q_EMIT inputDevicesChanged();
    }
}

void KWinWaylandBackend::onDeviceRemoved(QString sysName)
{
    const auto it = std::ranges::find_if(std::as_const(m_devices), [sysName](const std::unique_ptr<KWinWaylandDevice> &device) {
        return device->sysName() == sysName;
    });
    if (it == m_devices.cend()) {
        return;
    }
    int index = std::distance(m_devices.cbegin(), it);

    // delete at the end of the function, after change signals have been fired
    std::unique_ptr<KWinWaylandDevice> dev = std::move(m_devices[index]);
    m_devices.erase(m_devices.cbegin() + index);

    bool deviceNeededSave = dev->isSaveNeeded();
    disconnect(dev.get(), nullptr, this, nullptr);

    qCDebug(KCM_MOUSE).nospace() << "Device disconnected: " << dev->name() << " (" << dev->sysName() << ")";

    Q_EMIT deviceRemoved(index);
    Q_EMIT inputDevicesChanged();

    if (deviceNeededSave) {
        Q_EMIT needsSaveChanged();
    }
}

QList<InputDevice *> KWinWaylandBackend::inputDevices() const
{
    QList<InputDevice *> devices;
    devices.reserve(m_devices.size());
    for (const auto &device : m_devices) {
        devices.append(device.get());
    }
    return devices;
}

int KWinWaylandBackend::deviceCount() const
{
    return m_devices.size();
}

QString KWinWaylandBackend::errorString() const
{
    return m_errorString;
}

#include "moc_kwin_wl_backend.cpp"
