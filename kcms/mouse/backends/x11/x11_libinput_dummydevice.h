/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputdevice.h"

#include <QObject>
#include <QString>
#include <QtGui/private/qtx11extras_p.h>

#include <X11/Xdefs.h>

struct LibinputSettings;

class X11LibinputDummyDevice : public InputDevice
{
    Q_OBJECT

public:
    X11LibinputDummyDevice(QObject *parent, Display *dpy);
    ~X11LibinputDummyDevice() override;

    bool getConfig();
    bool getDefaultConfig();
    bool applyConfig();
    bool isChangedConfig() const;

    void getDefaultConfigFromX();

    //
    // general
    QString name() const override
    {
        return m_name.val;
    }

    bool supportsDisableEvents() const override
    {
        return m_supportsDisableEvents.val;
    }

    void setEnabled(bool enabled) override
    {
        m_enabled.set(enabled);
    }

    bool isEnabled() const override
    {
        return m_enabled.val;
    }

    Qt::MouseButtons supportedButtons() const override
    {
        return m_supportedButtons.val;
    }

    //
    // advanced
    bool supportsLeftHanded() const override
    {
        return m_supportsLeftHanded.val;
    }

    bool leftHandedEnabledByDefault() const override
    {
        return m_leftHandedEnabledByDefault.val;
    }

    bool isLeftHanded() const override
    {
        return m_leftHanded.val;
    }

    void setLeftHanded(bool set) override
    {
        m_leftHanded.set(set);
    }

    bool supportsMiddleEmulation() const override
    {
        return m_supportsMiddleEmulation.val;
    }

    bool middleEmulationEnabledByDefault() const override
    {
        return m_middleEmulationEnabledByDefault.val;
    }

    bool isMiddleEmulation() const override
    {
        return m_middleEmulation.val;
    }

    void setMiddleEmulation(bool set) override
    {
        m_middleEmulation.set(set);
    }

    //
    // acceleration speed and profile
    bool supportsPointerAcceleration() const override
    {
        return m_supportsPointerAcceleration.val;
    }

    qreal pointerAcceleration() const override
    {
        return m_pointerAcceleration.val;
    }

    void setPointerAcceleration(qreal acceleration) override
    {
        m_pointerAcceleration.set(acceleration);
    }

    bool supportsPointerAccelerationProfileFlat() const override
    {
        return m_supportsPointerAccelerationProfileFlat.val;
    }

    bool defaultPointerAccelerationProfileFlat() const override
    {
        return m_defaultPointerAccelerationProfileFlat.val;
    }

    bool pointerAccelerationProfileFlat() const override
    {
        return m_pointerAccelerationProfileFlat.val;
    }

    void setPointerAccelerationProfileFlat(bool set) override
    {
        m_pointerAccelerationProfileFlat.set(set);
    }

    bool supportsPointerAccelerationProfileAdaptive() const override
    {
        return m_supportsPointerAccelerationProfileAdaptive.val;
    }

    bool defaultPointerAccelerationProfileAdaptive() const override
    {
        return m_defaultPointerAccelerationProfileAdaptive.val;
    }

    bool pointerAccelerationProfileAdaptive() const override
    {
        return m_pointerAccelerationProfileAdaptive.val;
    }

    void setPointerAccelerationProfileAdaptive(bool set) override
    {
        m_pointerAccelerationProfileAdaptive.set(set);
    }

    //
    // scrolling
    bool supportsNaturalScroll() const override
    {
        return m_supportsNaturalScroll.val;
    }

    bool naturalScrollEnabledByDefault() const override
    {
        return m_naturalScrollEnabledByDefault.val;
    }

    bool isNaturalScroll() const override
    {
        return m_naturalScroll.val;
    }

    void setNaturalScroll(bool set) override
    {
        m_naturalScroll.set(set);
    }

    qreal scrollFactor() const override
    {
        return 1;
    }

    void setScrollFactor([[maybe_unused]] qreal set) override
    {
    }

private:
    template<typename T>
    struct Prop {
        using ChangedSignal = void (X11LibinputDummyDevice::*)();

        explicit Prop(X11LibinputDummyDevice *device, const QString &name, ChangedSignal changedSignal = nullptr, const QString &cfgName = "")
            : name(name)
            , cfgName(cfgName)
            , changedSignalFunction(changedSignal)
            , device(device)
        {
        }

        void set(T newVal)
        {
            if (avail && val != newVal) {
                val = newVal;
                if (changedSignalFunction) {
                    (device->*changedSignalFunction)();
                }
            }
        }
        void set(const Prop<T> &p)
        {
            if (avail && val != p.val) {
                val = p.val;
                if (changedSignalFunction) {
                    (device->*changedSignalFunction)();
                }
            }
        }
        bool changed() const
        {
            return avail && (old != val);
        }

        void reset(T newVal)
        {
            old = newVal;
            set(std::move(newVal));
        }

        QString name;
        QString cfgName;

        bool avail = true;
        const ChangedSignal changedSignalFunction;
        X11LibinputDummyDevice *const device;
        T old;
        T val;

        Atom atom;
    };

    template<typename T>
    bool valueWriter(Prop<T> &prop);

    //
    // general
    Prop<QString> m_name{this, "name"};
    Prop<QString> m_sysName{this, "sysName"};
    Prop<bool> m_supportsDisableEvents{this, "supportsDisableEvents"};
    Prop<bool> m_enabled{this, "enabled", &X11LibinputDummyDevice::enabledChanged};

    //
    // advanced
    Prop<Qt::MouseButtons> m_supportedButtons{this, "supportedButtons"};

    Prop<bool> m_supportsLeftHanded{this, "supportsLeftHanded"};
    Prop<bool> m_leftHandedEnabledByDefault{this, "leftHandedEnabledByDefault"};
    Prop<bool> m_leftHanded{this, "leftHanded", &X11LibinputDummyDevice::leftHandedChanged, "XLbInptLeftHanded"};

    Prop<bool> m_supportsMiddleEmulation{this, "supportsMiddleEmulation"};
    Prop<bool> m_middleEmulationEnabledByDefault{this, "middleEmulationEnabledByDefault"};
    Prop<bool> m_middleEmulation{this, "middleEmulation", &X11LibinputDummyDevice::middleEmulationChanged, "XLbInptMiddleEmulation"};

    //
    // acceleration speed and profile
    Prop<bool> m_supportsPointerAcceleration{this, "supportsPointerAcceleration"};
    Prop<qreal> m_defaultPointerAcceleration{this, "defaultPointerAcceleration"};
    Prop<qreal> m_pointerAcceleration{this, "pointerAcceleration", &X11LibinputDummyDevice::pointerAccelerationChanged, "XLbInptPointerAcceleration"};

    Prop<bool> m_supportsPointerAccelerationProfileFlat{this, "supportsPointerAccelerationProfileFlat"};
    Prop<bool> m_defaultPointerAccelerationProfileFlat{this, "defaultPointerAccelerationProfileFlat"};
    Prop<bool> m_pointerAccelerationProfileFlat{this,
                                                "pointerAccelerationProfileFlat",
                                                &X11LibinputDummyDevice::pointerAccelerationProfileFlatChanged,
                                                "XLbInptAccelProfileFlat"};

    Prop<bool> m_supportsPointerAccelerationProfileAdaptive{this, "supportsPointerAccelerationProfileAdaptive"};
    Prop<bool> m_defaultPointerAccelerationProfileAdaptive{this, "defaultPointerAccelerationProfileAdaptive"};
    Prop<bool> m_pointerAccelerationProfileAdaptive{this,
                                                    "pointerAccelerationProfileAdaptive",
                                                    &X11LibinputDummyDevice::pointerAccelerationProfileAdaptiveChanged};

    //
    // scrolling
    Prop<bool> m_supportsNaturalScroll{this, "supportsNaturalScroll"};
    Prop<bool> m_naturalScrollEnabledByDefault{this, "naturalScrollEnabledByDefault"};
    Prop<bool> m_naturalScroll{this, "naturalScroll", &X11LibinputDummyDevice::naturalScrollChanged, "XLbInptNaturalScroll"};

    LibinputSettings *m_settings = nullptr;
    Display *m_dpy = nullptr;
};
