/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWINWAYLANDDEVICE_H
#define KWINWAYLANDDEVICE_H

#include "InputDevice_interface.h"
#include <QObject>
#include <QString>
#include <optional>

class InputDevice : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sysName READ sysName CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString outputName READ outputName WRITE setOutputName NOTIFY outputNameChanged)

public:
    InputDevice(const QString &dbusName, QObject *parent);
    ~InputDevice() override;

    void load();
    void save();
    void defaults();
    bool isSaveNeeded() const;
    bool isDefaults() const;

    QString name() const
    {
        return m_name.value();
    }

    QString sysName() const
    {
        return m_sysName.value();
    }

    QString outputName() const
    {
        return m_outputName.value();
    }
    void setOutputName(const QString &outputName);

    bool isEnabled() const
    {
        return m_enabled.value();
    }

    void setEnabled(bool enabled);

Q_SIGNALS:
    void needsSaveChanged();

    void outputNameChanged();
    void enabledChanged();

private:
    template<typename T>
    struct Prop {
        typedef T (OrgKdeKWinInputDeviceInterface::*ValueFunction)() const;
        typedef bool (OrgKdeKWinInputDeviceInterface::*SupportedFunction)() const;
        typedef bool (OrgKdeKWinInputDeviceInterface::*SetterFunction)(const T &value);
        typedef void (InputDevice::*ChangedSignal)();

        explicit Prop(InputDevice *device, const char *propName, ValueFunction defaultValueFunction, SupportedFunction supported, ChangedSignal changedSignal)
            : m_defaultValueFunction(defaultValueFunction)
            , m_supportedFunction(supported)
            , m_changedSignalFunction(changedSignal)
            , m_device(device)
        {
            int idx = OrgKdeKWinInputDeviceInterface::staticMetaObject.indexOfProperty(propName);
            if (idx < 0) {
                qDebug() << "there is no" << propName;
            }
            Q_ASSERT(idx >= 0);
            m_prop = OrgKdeKWinInputDeviceInterface::staticMetaObject.property(idx);
        }

        explicit Prop(InputDevice *device, const char *propName)
            : m_defaultValueFunction(nullptr)
            , m_supportedFunction(nullptr)
            , m_changedSignalFunction(nullptr)
            , m_device(device)
        {
            Q_ASSERT(m_device);
            int idx = OrgKdeKWinInputDeviceInterface::staticMetaObject.indexOfProperty(propName);
            Q_ASSERT(idx >= 0);
            m_prop = OrgKdeKWinInputDeviceInterface::staticMetaObject.property(idx);
        }

        T value() const
        {
            if (!m_value.has_value()) {
                auto iface = m_device->m_iface.get();
                if (isSupported()) {
                    m_value = m_prop.read(iface).value<T>();
                }
            }
            return m_value ? m_value.value() : T();
        }
        void resetFromDefaults()
        {
            if (isSupported()) {
                set(defaultValue());
            }
        }
        void resetFromSaved()
        {
            m_value = {};
            value();
            m_configValue = m_value;
        }

        void set(T newVal);
        T defaultValue() const
        {
            return m_defaultValueFunction ? (m_device->m_iface.get()->*m_defaultValueFunction)() : T();
        }
        bool changed() const;
        void set(const Prop<T> &p)
        {
            set(p.value);
        }

        bool isSupported() const
        {
            auto iface = m_device->m_iface.get();
            return !m_supportedFunction || (iface->*m_supportedFunction)();
        }

        bool save();
        bool isDefaults() const
        {
            return m_value == defaultValue();
        }

    private:
        QMetaProperty m_prop;
        const ValueFunction m_defaultValueFunction;
        const SupportedFunction m_supportedFunction;
        const ChangedSignal m_changedSignalFunction;
        InputDevice *const m_device;
        mutable std::optional<T> m_configValue;
        mutable std::optional<T> m_value;
    };

    //
    // general
    Prop<QString> m_name = Prop<QString>(this, "name");
    Prop<QSizeF> m_size = Prop<QSizeF>(this, "size");
    Prop<QString> m_sysName = Prop<QString>(this, "sysName");

    Prop<bool> m_enabled = Prop<bool>(this, "enabled", &OrgKdeKWinInputDeviceInterface::enabledByDefault, nullptr, &InputDevice::enabledChanged);

    Prop<QString> m_outputName = Prop<QString>(this, "outputName", nullptr, nullptr, &InputDevice::outputNameChanged);

    std::unique_ptr<OrgKdeKWinInputDeviceInterface> m_iface;
};

#endif // KWINWAYLANDDEVICE_H
