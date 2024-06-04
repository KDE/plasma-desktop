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

KWinWaylandBackend::KWinWaylandBackend(QObject *parent)
    : InputBackend(parent)
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

    connect(this, &InputBackend::buttonMappingChanged, this, &InputBackend::needsSaveChanged);
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
            if (!dev->init()) {
                qCCritical(KCM_MOUSE) << "Error on creating device object" << sn;
                m_errorString = i18n("Critical error on reading fundamental device infos of %1.", sn);
                return;
            }
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

    for (const auto &[buttonName, variant] : std::as_const(m_buttonMapping).asKeyValueRange()) {
        if (const auto keySequence = variant.value<QKeySequence>(); !keySequence.isEmpty()) {
            const auto value = QStringList{u"Key"_s, keySequence.toString(QKeySequence::PortableText)};
            buttonGroup.writeEntry(buttonName, value, KConfig::Notify);
        } else {
            buttonGroup.deleteEntry(buttonName, KConfig::Notify);
        }
    }

    return forAllDevices(&KWinWaylandDevice::save);
}

bool KWinWaylandBackend::load()
{
    m_loadedButtonMapping.clear();
    const KConfigGroup buttonGroup = mouseButtonRebindsConfigGroup();

    for (int i = 1; i <= 24; ++i) {
        const auto buttonName = QLatin1String("ExtraButton%1").arg(QString::number(i));
        const auto entry = buttonGroup.readEntry(buttonName, QStringList());
        if (entry.size() == 2 && entry.first() == QLatin1String("Key")) {
            if (const auto keySequence = QKeySequence::fromString(entry.at(1), QKeySequence::PortableText); !keySequence.isEmpty()) {
                m_loadedButtonMapping.insert(buttonName, keySequence);
            }
        }
    }
    setButtonMapping(m_loadedButtonMapping);

    return forAllDevices(&KWinWaylandDevice::init);
}

bool KWinWaylandBackend::defaults()
{
    return forAllDevices(&KWinWaylandDevice::defaults);
}

bool KWinWaylandBackend::isSaveNeeded() const
{
    return m_buttonMapping != m_loadedButtonMapping || std::ranges::any_of(std::as_const(m_devices), [](const std::unique_ptr<KWinWaylandDevice> &device) {
               return device->isSaveNeeded();
           });
}

QVariantMap KWinWaylandBackend::buttonMapping() const
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
        if (!dev->init()) {
            Q_EMIT deviceAdded(false);
            return;
        }

        connect(dev.get(), &KWinWaylandDevice::needsSaveChanged, this, &InputBackend::needsSaveChanged);
        qCDebug(KCM_MOUSE).nospace() << "Device connected: " << dev->name() << " (" << dev->sysName() << ")";
        m_devices.push_back(std::move(dev));
        Q_EMIT deviceAdded(true);
        Q_EMIT inputDevicesChanged();

        if (dev->isSaveNeeded()) {
            Q_EMIT needsSaveChanged();
        }
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
