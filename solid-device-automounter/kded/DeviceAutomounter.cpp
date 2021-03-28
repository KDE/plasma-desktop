/***************************************************************************
 *   Copyright (C) 2009 by Trever Fischer <wm161@wm161.net>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "DeviceAutomounter.h"

#include <KPluginFactory>

#include <Solid/DeviceNotifier>
#include <Solid/StorageAccess>
#include <Solid/StorageVolume>

#include <QDBusConnection>
#include <QDBusMessage>

#include <QTimer>

K_PLUGIN_CLASS_WITH_JSON(DeviceAutomounter, "device_automounter.json")

DeviceAutomounter::DeviceAutomounter(QObject *parent, const QVariantList &args)
    : KDEDModule(parent)
    , m_settings(new AutomounterSettings(this))
{
    Q_UNUSED(args)
    QTimer::singleShot(0, this, &DeviceAutomounter::init);
}

DeviceAutomounter::~DeviceAutomounter()
{
}

void DeviceAutomounter::init()
{
    if (!m_settings->automountEnabled()) {
        // Automounting is disabled, no point in hanging around.
        QDBusConnection dbus = QDBusConnection::sessionBus();
        QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded5"),
                                                          QStringLiteral("/kded"),
                                                          QStringLiteral("org.kde.kded5"),
                                                          QStringLiteral("setModuleAutoloading"));
        msg.setArguments({QVariant(QStringLiteral("device_automounter")), QVariant(false)});
        dbus.call(msg, QDBus::NoBlock);

        // Unload right away
        msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded5"),
                                             QStringLiteral("/kded"),
                                             QStringLiteral("org.kde.kded5"),
                                             QStringLiteral("unloadModule"));
        msg.setArguments({QVariant(QStringLiteral("device_automounter"))});
        dbus.call(msg, QDBus::NoBlock);
        return;
    }

    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, &DeviceAutomounter::deviceAdded);
    const QList<Solid::Device> volumes = Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume);
    for (Solid::Device volume : volumes) {
        // sa can be 0 (e.g. for the swap partition)
        if (const Solid::StorageAccess *sa = volume.as<Solid::StorageAccess>()) {
            connect(sa, &Solid::StorageAccess::accessibilityChanged, this, &DeviceAutomounter::deviceMountChanged);
        }
        automountDevice(volume, m_settings->Login);
    }
    m_settings->save();
}

void DeviceAutomounter::deviceMountChanged(bool accessible, const QString &udi)
{
    m_settings->setDeviceLastSeenMounted(udi, accessible);
    m_settings->save();
}

void DeviceAutomounter::automountDevice(Solid::Device &dev, AutomounterSettings::AutomountType type)
{
    if (dev.is<Solid::StorageVolume>() && dev.is<Solid::StorageAccess>()) {
        Solid::StorageAccess *sa = dev.as<Solid::StorageAccess>();

        m_settings->setDeviceLastSeenMounted(dev.udi(), sa->isAccessible());
        m_settings->saveDevice(dev);

        if (m_settings->shouldAutomountDevice(dev.udi(), type)) {
            Solid::StorageVolume *sv = dev.as<Solid::StorageVolume>();
            if (!sa->isAccessible() && !sv->isIgnored()) {
                sa->setup();
            }
        }
    }
}

void DeviceAutomounter::deviceAdded(const QString &udi)
{
    m_settings->load();

    Solid::Device dev(udi);
    automountDevice(dev, AutomounterSettings::Attach);
    m_settings->save();

    if (dev.is<Solid::StorageAccess>()) {
        Solid::StorageAccess *sa = dev.as<Solid::StorageAccess>();
        if (sa) {
            connect(sa, &Solid::StorageAccess::accessibilityChanged, this, &DeviceAutomounter::deviceMountChanged);
        }
    }
}

#include "DeviceAutomounter.moc"
