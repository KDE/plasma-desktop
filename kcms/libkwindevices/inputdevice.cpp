/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "inputdevice.h"

#include <QList>

#include "logging.h"

template<typename T>
bool InputDevice::Prop<T>::save()
{
    if (!isSupported() || !m_value || m_prop.isConstant()) {
        qCDebug(LIBKWINDEVICES) << "skipping" << this << m_value.has_value() << isSupported() << m_prop.name();
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
    m_iface = std::make_unique<OrgKdeKWinInputDeviceInterface>(QStringLiteral("org.kde.KWin"),
                                                               QStringLiteral("/org/kde/KWin/InputDevice/") + dbusName,
                                                               QDBusConnection::sessionBus(),
                                                               this);
    connect(this, &InputDevice::leftHandedChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::orientationChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::outputNameChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::outputAreaChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::enabledChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::mapToWorkspaceChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::pressureCurveChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::inputAreaChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::pressureRangeMinChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::pressureRangeMaxChanged, this, &InputDevice::needsSaveChanged);
    connect(this, &InputDevice::calibrationMatrixChanged, this, &InputDevice::needsSaveChanged);
}

void InputDevice::save()
{
    m_orientation.save();
    m_outputName.save();
    m_leftHanded.save();
    m_outputArea.save();
    m_enabled.save();
    m_mapToWorkspace.save();
    m_pressureCurve.save();
    m_inputArea.save();
    m_pressureRangeMin.save();
    m_pressureRangeMax.save();
    m_calibrationMatrix.save();
}

bool InputDevice::isSaveNeeded() const
{
    return m_leftHanded.changed() || m_orientation.changed() || m_outputName.changed() || m_outputArea.changed() || m_enabled.changed()
        || m_mapToWorkspace.changed() || m_pressureCurve.changed() || m_inputArea.changed() || m_pressureRangeMin.changed() || m_pressureRangeMax.changed() || m_calibrationMatrix.changed();
}

void InputDevice::defaults()
{
    m_leftHanded.resetFromDefaults();
    m_orientation.resetFromDefaults();
    m_outputName.resetFromDefaults();
    m_outputArea.resetFromDefaults();
    m_enabled.resetFromDefaults();
    m_mapToWorkspace.resetFromDefaults();
    m_pressureCurve.resetFromDefaults();
    m_inputArea.resetFromDefaults();
    m_pressureRangeMin.resetFromDefaults();
    m_pressureRangeMax.resetFromDefaults();
    m_calibrationMatrix.resetFromDefaults();
}

bool InputDevice::isDefaults() const
{
    return m_leftHanded.isDefaults() && m_orientation.isDefaults() && m_outputName.isDefaults() && m_outputArea.isDefaults() && m_enabled.isDefaults()
        && m_mapToWorkspace.isDefaults() && m_pressureCurve.isDefaults() && m_inputArea.isDefaults() && m_pressureRangeMin.isDefaults() && m_pressureRangeMax.isDefaults() && m_calibrationMatrix.isDefaults();
}

void InputDevice::load()
{
    m_orientation.resetFromSaved();
    m_leftHanded.resetFromSaved();
    m_outputName.resetFromSaved();
    m_outputArea.resetFromSaved();
    m_enabled.resetFromSaved();
    m_mapToWorkspace.resetFromSaved();
    m_pressureCurve.resetFromSaved();
    m_inputArea.resetFromSaved();
    m_pressureRangeMin.resetFromSaved();
    m_pressureRangeMax.resetFromSaved();
    m_calibrationMatrix.resetFromSaved();
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

void InputDevice::setOutputArea(const QRectF &outputArea)
{
    m_outputArea.set(outputArea);
}

void InputDevice::setInputArea(const QRectF &inputArea)
{
    m_inputArea.set(inputArea);
}

void InputDevice::setEnabled(bool enabled)
{
    m_enabled.set(enabled);
}

void InputDevice::setMapToWorkspace(bool mapToWorkspace)
{
    m_mapToWorkspace.set(mapToWorkspace);
}

void InputDevice::setPressureCurve(const QString &curve)
{
    m_pressureCurve.set(curve);
}

bool InputDevice::pressureCurveIsDefault() const
{
    return m_pressureCurve.isDefaults();
}

QString InputDevice::serializeMatrix(const QMatrix4x4 &matrix)
{
    QString result;
    for (int i = 0; i < 16; i++) {
        result.append(QString::number(matrix.constData()[i]));
        if (i != 15) {
            result.append(QLatin1Char(','));
        }
    }
    return result;
}

QMatrix4x4 InputDevice::deserializeMatrix(const QString &matrix)
{
    const auto items = QStringView(matrix).split(QLatin1Char(','));
    if (items.size() == 16) {
        QList<float> data;
        data.reserve(16);
        std::ranges::transform(std::as_const(items), std::back_inserter(data), [](const QStringView &item) {
            return item.toFloat();
        });

        return QMatrix4x4{data.constData()};
    }

    return QMatrix4x4{};
}

#include "moc_inputdevice.cpp"
