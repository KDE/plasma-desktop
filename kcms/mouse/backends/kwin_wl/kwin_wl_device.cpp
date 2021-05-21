/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwin_wl_device.h"

#include <QDBusError>
#include <QDBusInterface>
#include <QVector>

#include "logging.h"

namespace
{
template<typename T>
T valueLoaderPart(QVariant const &reply)
{
    Q_UNUSED(reply);
    return T();
}

template<>
bool valueLoaderPart(QVariant const &reply)
{
    return reply.toBool();
}

template<>
int valueLoaderPart(QVariant const &reply)
{
    return reply.toInt();
}

template<>
quint32 valueLoaderPart(QVariant const &reply)
{
    return reply.toInt();
}

template<>
qreal valueLoaderPart(QVariant const &reply)
{
    return reply.toReal();
}

template<>
QString valueLoaderPart(QVariant const &reply)
{
    return reply.toString();
}

template<>
Qt::MouseButtons valueLoaderPart(QVariant const &reply)
{
    return static_cast<Qt::MouseButtons>(reply.toInt());
}
}

KWinWaylandDevice::KWinWaylandDevice(QString dbusName)
{
    m_iface = new QDBusInterface(QStringLiteral("org.kde.KWin"),
                                 QStringLiteral("/org/kde/KWin/InputDevice/") + dbusName,
                                 QStringLiteral("org.kde.KWin.InputDevice"),
                                 QDBusConnection::sessionBus(),
                                 this);
}

KWinWaylandDevice::~KWinWaylandDevice()
{
    delete m_iface;
}

bool KWinWaylandDevice::init()
{
    // need to do it here in order to populate combobox and handle events
    return valueLoader(m_name) && valueLoader(m_sysName);
}

bool KWinWaylandDevice::getConfig()
{
    bool success = true;

    // general
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
    QVector<QString> msgs;

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

bool KWinWaylandDevice::isChangedConfig() const
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
    m_iface->setProperty(prop.dbus, prop.val);
    QDBusError error = m_iface->lastError();
    if (error.isValid()) {
        qCCritical(KCM_MOUSE) << error.message();
        return error.message();
    }
    return QString();
}

template<typename T>
bool KWinWaylandDevice::valueLoader(Prop<T> &prop)
{
    QVariant reply = m_iface->property(prop.dbus);
    if (!reply.isValid()) {
        qCCritical(KCM_MOUSE) << "Error on d-bus read of" << prop.dbus;
        prop.avail = false;
        return false;
    }
    prop.avail = true;

    T replyValue = valueLoaderPart<T>(reply);

    prop.old = replyValue;
    prop.val = replyValue;
    return true;
}
