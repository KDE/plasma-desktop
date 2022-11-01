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
        qCDebug(KCM_TOUCHSCREEN) << "skipping" << this << m_value.has_value() << isSupported() << m_prop.name();
        return false;
    }

    auto iface = m_device->m_iface.get();
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

InputDevice::InputDevice(const QString &dbusName, QObject *parent)
    : QObject(parent)
{
    m_iface.reset(new OrgKdeKWinInputDeviceInterface(QStringLiteral("org.kde.KWin"),
                                                     QStringLiteral("/org/kde/KWin/InputDevice/") + dbusName,
                                                     QDBusConnection::sessionBus(),
                                                     this));
    connect(this, &InputDevice::outputNameChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::enabledChanged, this, &InputDevice::needsSaveChanged);
}

InputDevice::~InputDevice()
{
}

void InputDevice::save()
{
    m_outputName.save();
    m_enabled.save();
}

bool InputDevice::isSaveNeeded() const
{
    return m_outputName.changed() || m_enabled.changed();
}

void InputDevice::defaults()
{
    m_outputName.resetFromDefaults();
    m_enabled.resetFromDefaults();
}

bool InputDevice::isDefaults() const
{
    return m_outputName.isDefaults() && m_enabled.isDefaults();
}

void InputDevice::load()
{
    m_outputName.resetFromSaved();
    m_enabled.resetFromSaved();
}

void InputDevice::setOutputName(const QString &outputName)
{
    m_outputName.set(outputName);
}

void InputDevice::setEnabled(bool enabled)
{
    m_enabled.set(enabled);
}
