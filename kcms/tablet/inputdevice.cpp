/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "inputdevice.h"

#include <QDBusError>
#include <QDBusInterface>
#include <QVector>

#include "logging.h"

template<typename T>
bool InputDevice::Prop<T>::save()
{
    if (!isSupported() || !m_value || m_prop.isConstant()) {
        qCDebug(KCM_TABLET) << "skipping" << this << m_value.has_value() << isSupported() << m_prop.name();
        return false;
    }

    auto iface = m_device->m_iface.data();
    const bool ret = m_prop.write(iface, *m_value);
    if (ret) {
        m_configValue = *m_value;
    }
    return ret;
}

template<typename T>
void InputDevice::Prop<T>::set(T newVal)
{
    if (!m_value) {
        value();
    }

    Q_ASSERT(isSupported());
    if (m_value != newVal) {
        m_value = newVal;
        if (m_changedSignalFunction) {
            (m_device->*m_changedSignalFunction)();
        }
    }
}

template<typename T>
bool InputDevice::Prop<T>::changed() const
{
    return m_value.has_value() && m_value.value() != m_configValue;
}

InputDevice::InputDevice(QString dbusName)
{
    m_iface.reset(new OrgKdeKWinInputDeviceInterface(QStringLiteral("org.kde.KWin"),
                                                     QStringLiteral("/org/kde/KWin/InputDevice/") + dbusName,
                                                     QDBusConnection::sessionBus(),
                                                     this));
    connect(this, &InputDevice::leftHandedChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::orientationChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::outputNameChanged, this, &InputDevice::needsSaveChanged);
}

InputDevice::~InputDevice() = default;

void InputDevice::save()
{
    m_orientation.save();
    m_outputName.save();
    m_leftHanded.save();
}

bool InputDevice::isSaveNeeded() const
{
    return m_leftHanded.changed() || m_orientation.changed() || m_outputName.changed();
}

void InputDevice::defaults()
{
    m_leftHanded.resetFromDefaults();
    m_orientation.resetFromDefaults();
    m_outputName.resetFromDefaults();
}

bool InputDevice::isDefaults() const
{
    return m_leftHanded.isDefaults() && m_orientation.isDefaults() && m_outputName.isDefaults();
}

void InputDevice::load()
{
    m_orientation.resetFromSaved();
    m_leftHanded.resetFromSaved();
    m_outputName.resetFromSaved();
}

void InputDevice::setOrientation(int ori)
{
    m_orientation.set(ori);
}

void InputDevice::setOutputName(const QString &outputName)
{
    m_outputName.set(outputName);
}

void InputDevice::setLeftHanded(bool set)
{
    m_leftHanded.set(set);
}
