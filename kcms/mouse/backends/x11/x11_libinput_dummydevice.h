/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputdevice.h"

#include <QObject>
#include <QString>
#include <QtGui/private/qtx11extras_p.h>

#include <X11/X.h>
#include <X11/Xdefs.h>

#include <memory>

using namespace Qt::StringLiterals;

struct LibinputSettings;

class X11LibinputDummyDevice : public InputDevice
{
    Q_OBJECT

public:
    X11LibinputDummyDevice(QObject *parent, Display *dpy);
    ~X11LibinputDummyDevice() override;

    bool load();
    bool defaults();
    bool save();
    bool isSaveNeeded() const;

    void defaultsFromX();

    //
    // general
    QString name() const override
    {
        return QString();
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

        explicit Prop(X11LibinputDummyDevice *device, ChangedSignal changedSignal = nullptr, const QString &cfgName = u""_s)
            : cfgName(cfgName)
            , changedSignalFunction(changedSignal)
            , device(device)
            , old(T())
            , val(T())
            , atom(None)
        {
        }

        void set(const T &newVal)
        {
            if (val != newVal) {
                val = newVal;
                if (changedSignalFunction) {
                    (device->*changedSignalFunction)();
                }
            }
        }
        void set(const Prop<T> &other)
        {
            if (val != other.val) {
                val = other.val;
                if (changedSignalFunction) {
                    (device->*changedSignalFunction)();
                }
            }
        }
        bool isSaveNeeded() const
        {
            return old != val;
        }

        void reset(const T &newVal)
        {
            old = newVal;
            set(newVal);
        }

        QString cfgName;

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
    Prop<bool> m_supportsDisableEvents{this};
    Prop<bool> m_enabled{this, &X11LibinputDummyDevice::enabledChanged};

    //
    // advanced
    Prop<Qt::MouseButtons> m_supportedButtons{this};

    Prop<bool> m_supportsLeftHanded{this};
    Prop<bool> m_leftHandedEnabledByDefault{this};
    Prop<bool> m_leftHanded{this, &X11LibinputDummyDevice::leftHandedChanged, u"XLbInptLeftHanded"_s};

    Prop<bool> m_supportsMiddleEmulation{this};
    Prop<bool> m_middleEmulationEnabledByDefault{this};
    Prop<bool> m_middleEmulation{this, &X11LibinputDummyDevice::middleEmulationChanged, u"XLbInptMiddleEmulation"_s};

    //
    // acceleration speed and profile
    Prop<bool> m_supportsPointerAcceleration{this};
    Prop<qreal> m_defaultPointerAcceleration{this};
    Prop<qreal> m_pointerAcceleration{this, &X11LibinputDummyDevice::pointerAccelerationChanged, u"XLbInptPointerAcceleration"_s};

    Prop<bool> m_supportsPointerAccelerationProfileFlat{this};
    Prop<bool> m_defaultPointerAccelerationProfileFlat{this};
    Prop<bool> m_pointerAccelerationProfileFlat{this, &X11LibinputDummyDevice::pointerAccelerationProfileFlatChanged, u"XLbInptAccelProfileFlat"_s};

    Prop<bool> m_supportsPointerAccelerationProfileAdaptive{this};
    Prop<bool> m_defaultPointerAccelerationProfileAdaptive{this};
    Prop<bool> m_pointerAccelerationProfileAdaptive{this, &X11LibinputDummyDevice::pointerAccelerationProfileAdaptiveChanged};

    //
    // scrolling
    Prop<bool> m_supportsNaturalScroll{this};
    Prop<bool> m_naturalScrollEnabledByDefault{this};
    Prop<bool> m_naturalScroll{this, &X11LibinputDummyDevice::naturalScrollChanged, u"XLbInptNaturalScroll"_s};

    std::unique_ptr<LibinputSettings> m_settings;
    Display *m_dpy = nullptr;
};
