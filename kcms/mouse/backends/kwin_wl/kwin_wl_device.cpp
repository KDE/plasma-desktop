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
T valueLoaderPart(QVariant const &reply)
{
    return reply.value<T>();
}

template<>
Qt::MouseButtons valueLoaderPart(QVariant const &reply)
{
    return static_cast<Qt::MouseButtons>(reply.toInt());
}

}

KWinWaylandDevice::KWinWaylandDevice(const QString &dbusName)
    : m_dbusName(dbusName)
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

    auto valueLoader = [properties = reply.value(), this](auto &prop) {
        if (QVariant variant = properties.value(prop.dbus); variant.isValid()) {
            prop.avail = true;
            prop.old = valueLoaderPart<typename std::remove_reference_t<decltype(prop)>::value_type>(variant);
            prop.set(prop.old);
            return true;
        }
        qCCritical(KCM_MOUSE) << "Device" << m_dbusName << "does not have property on d-bus read of" << prop.dbus;
        prop.avail = false;
        return false;
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

    return success;
}

bool KWinWaylandDevice::getDefaultConfig()
{
    m_enabled.set(true);
    m_leftHanded.set(false);

    m_pointerAcceleration.set(m_defaultPointerAcceleration);
    m_pointerAccelerationProfileFlat.set(m_defaultPointerAccelerationProfileFlat);
    m_pointerAccelerationProfileAdaptive.set(m_defaultPointerAccelerationProfileAdaptive);

    m_middleEmulation.set(m_middleEmulationEnabledByDefault);
    m_naturalScroll.set(m_naturalScrollEnabledByDefault);

    m_scrollFactor.set(1.0);

    return true;
}

bool KWinWaylandDevice::applyConfig()
{
    QList<QString> msgs;

    msgs << valueWriter(m_enabled) << valueWriter(m_leftHanded) << valueWriter(m_pointerAcceleration) << valueWriter(m_pointerAccelerationProfileFlat)
         << valueWriter(m_pointerAccelerationProfileAdaptive) << valueWriter(m_middleEmulation) << valueWriter(m_naturalScroll) << valueWriter(m_scrollFactor);
    bool success = true;
    QString error_msg;

    for (QString m : msgs) {
        if (!m.isNull()) {
            qCCritical(KCM_MOUSE) << "in error:" << m;
            if (!success) {
                error_msg.append("\n");
            }
            error_msg.append(m);
            success = false;
        }
    }

    if (!success) {
        qCCritical(KCM_MOUSE) << error_msg;
    }
    return success;
}

bool KWinWaylandDevice::isSaveNeeded() const
{
    return m_enabled.changed() || m_leftHanded.changed() || m_pointerAcceleration.changed() || m_pointerAccelerationProfileFlat.changed()
        || m_pointerAccelerationProfileAdaptive.changed() || m_middleEmulation.changed() || m_scrollFactor.changed() || m_naturalScroll.changed();
}

template<typename T>
QString KWinWaylandDevice::valueWriter(const Prop<T> &prop)
{
    if (!prop.changed()) {
        return QString();
    }
    auto message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                  QStringLiteral("/org/kde/KWin/InputDevice/") + m_dbusName,
                                                  QStringLiteral("org.freedesktop.DBus.Properties"),
                                                  QStringLiteral("Set"));
    message << QStringLiteral("org.kde.KWin.InputDevice") << prop.dbus << QVariant::fromValue(QDBusVariant(prop.val));
    QDBusReply<void> reply = QDBusConnection::sessionBus().call(message);
    if (reply.error().isValid()) {
        qCCritical(KCM_MOUSE) << reply.error().message();
        return reply.error().message();
    }
    return QString();
}

#include "moc_kwin_wl_device.cpp"
