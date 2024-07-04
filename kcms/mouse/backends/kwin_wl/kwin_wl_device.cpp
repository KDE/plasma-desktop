/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwin_wl_device.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusReply>
#include <QList>

#include "logging.h"

namespace
{
template<typename T>
T valueLoaderPart(const QVariant &reply)
{
    return reply.value<T>();
}

template<>
Qt::MouseButtons valueLoaderPart(const QVariant &reply)
{
    return static_cast<Qt::MouseButtons>(reply.toInt());
}

}

KWinWaylandDevice::KWinWaylandDevice(const QString &dbusName)
    : InputDevice()
    , m_dbusName(dbusName)
{
}

KWinWaylandDevice::~KWinWaylandDevice()
{
}

bool KWinWaylandDevice::init()
{
    bool success = true;

    auto message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                  QStringLiteral("/org/kde/KWin/InputDevice/") + m_dbusName,
                                                  QStringLiteral("org.freedesktop.DBus.Properties"),
                                                  QStringLiteral("GetAll"));
    message << QStringLiteral("org.kde.KWin.InputDevice");
    QDBusReply<QVariantMap> reply = QDBusConnection::sessionBus().call(message);

    if (!reply.isValid()) {
        return false;
    }

    const auto properties = reply.value();

    const auto valueLoader = [&]<typename T>(Prop<T> &prop) {
        return this->valueLoader(properties, prop);
    };

    // general
    success &= valueLoader(m_name);
    success &= valueLoader(m_sysName);
    success &= valueLoader(m_supportsDisableEvents);
    success &= valueLoader(m_enabled);
    // advanced
    success &= valueLoader(m_supportedButtons);
    success &= valueLoader(m_supportsLeftHanded);
    success &= valueLoader(m_leftHandedEnabledByDefault);
    success &= valueLoader(m_leftHanded);
    success &= valueLoader(m_supportsMiddleEmulation);
    success &= valueLoader(m_middleEmulationEnabledByDefault);
    success &= valueLoader(m_middleEmulation);
    // acceleration
    success &= valueLoader(m_supportsPointerAcceleration);
    success &= valueLoader(m_supportsPointerAccelerationProfileFlat);
    success &= valueLoader(m_supportsPointerAccelerationProfileAdaptive);
    success &= valueLoader(m_defaultPointerAcceleration);
    success &= valueLoader(m_defaultPointerAccelerationProfileFlat);
    success &= valueLoader(m_defaultPointerAccelerationProfileAdaptive);
    success &= valueLoader(m_pointerAcceleration);
    success &= valueLoader(m_pointerAccelerationProfileFlat);
    success &= valueLoader(m_pointerAccelerationProfileAdaptive);
    // natural scroll
    success &= valueLoader(m_supportsNaturalScroll);
    success &= valueLoader(m_naturalScrollEnabledByDefault);
    success &= valueLoader(m_naturalScroll);

    success &= valueLoader(m_scrollFactor);
    success &= valueLoader(m_supportsScrollOnButtonDown);
    success &= valueLoader(m_scrollOnButtonDownEnabledByDefault);
    success &= valueLoader(m_scrollOnButtonDown);

    return success;
}

bool KWinWaylandDevice::defaults()
{
    // general & advanced
    m_enabled.set(true);
    m_leftHanded.set(false);
    m_middleEmulation.set(m_middleEmulationEnabledByDefault);
    // acceleration
    m_pointerAcceleration.set(m_defaultPointerAcceleration);
    m_pointerAccelerationProfileFlat.set(m_defaultPointerAccelerationProfileFlat);
    m_pointerAccelerationProfileAdaptive.set(m_defaultPointerAccelerationProfileAdaptive);
    // scrolling
    m_naturalScroll.set(m_naturalScrollEnabledByDefault);
    m_scrollFactor.set(1.0);
    m_scrollOnButtonDown.set(m_scrollOnButtonDownEnabledByDefault);

    return true;
}

bool KWinWaylandDevice::save()
{
    bool success = true;

    // general & advanced
    success &= valueWriter(m_enabled);
    success &= valueWriter(m_leftHanded);
    success &= valueWriter(m_middleEmulation);
    // acceleration
    success &= valueWriter(m_pointerAcceleration);
    success &= valueWriter(m_pointerAccelerationProfileFlat);
    success &= valueWriter(m_pointerAccelerationProfileAdaptive);
    // scrolling
    success &= valueWriter(m_naturalScroll);
    success &= valueWriter(m_scrollFactor);
    success &= valueWriter(m_scrollOnButtonDown);

    return success;
}

bool KWinWaylandDevice::isSaveNeeded() const
{
    //     general & advanced
    return m_enabled.isSaveNeeded() //
        || m_leftHanded.isSaveNeeded() //
        || m_middleEmulation.isSaveNeeded() //
        // acceleration
        || m_pointerAcceleration.isSaveNeeded() //
        || m_pointerAccelerationProfileFlat.isSaveNeeded() //
        || m_pointerAccelerationProfileAdaptive.isSaveNeeded() //
        // scrolling
        || m_naturalScroll.isSaveNeeded() //
        || m_scrollFactor.isSaveNeeded() //
        || m_scrollOnButtonDown.isSaveNeeded();
}

template<typename T>
bool KWinWaylandDevice::valueLoader(const QVariantMap &properties, Prop<T> &prop)
{
    if (const auto variant = properties.value(prop.dbus); variant.isValid()) {
        prop.avail = true;
        prop.old = valueLoaderPart<T>(variant);
        prop.set(prop.old);
        return true;
    } else {
        qCCritical(KCM_MOUSE) << "Device" << m_dbusName << "does not have property on d-bus read of" << prop.dbus;
        prop.avail = false;
        return false;
    }
};

template<typename T>
bool KWinWaylandDevice::valueWriter(const Prop<T> &prop)
{
    if (!prop.isSaveNeeded()) {
        return true;
    }
    auto message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                  QStringLiteral("/org/kde/KWin/InputDevice/") + m_dbusName,
                                                  QStringLiteral("org.freedesktop.DBus.Properties"),
                                                  QStringLiteral("Set"));
    message << QStringLiteral("org.kde.KWin.InputDevice") << prop.dbus << QVariant::fromValue(QDBusVariant(prop.val));
    QDBusReply<void> reply = QDBusConnection::sessionBus().call(message);
    if (reply.error().isValid()) {
        qCCritical(KCM_MOUSE) << reply.error().message();
        return false;
    }
    return true;
}

#include "moc_kwin_wl_device.cpp"
