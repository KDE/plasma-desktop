/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputdevice.h"

#include <QObject>
#include <QString>

using namespace Qt::StringLiterals;

class QDBusInterface;

class KWinWaylandDevice : public InputDevice
{
    Q_OBJECT

public:
    KWinWaylandDevice(const QString &dbusName);
    ~KWinWaylandDevice() override;

    bool init();

    bool defaults();
    bool save();
    bool isSaveNeeded() const;

    //
    // general
    QString name() const override
    {
        return m_name.val;
    }

    QString sysName() const
    {
        return m_sysName.val;
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
        return m_scrollFactor.val;
    }

    void setScrollFactor(qreal set) override
    {
        m_scrollFactor.set(set);
    }

    bool supportsScrollOnButtonDown() const override
    {
        return m_supportsScrollOnButtonDown.val;
    }

    bool scrollOnButtonDownEnabledByDefault() const override
    {
        return m_scrollOnButtonDownEnabledByDefault.val;
    }

    bool isScrollOnButtonDown() const override
    {
        return m_scrollOnButtonDown.val;
    }

    void setScrollOnButtonDown(bool set) override
    {
        m_scrollOnButtonDown.set(set);
    }

private:
    template<typename T>
    struct Prop {
        using ChangedSignal = void (KWinWaylandDevice::*)();

        explicit Prop(KWinWaylandDevice *device, const QString &dbusName, ChangedSignal changedSignal = nullptr)
            : dbus(dbusName)
            , avail(false)
            , changedSignalFunction(changedSignal)
            , device(device)
            , old(T())
            , val(T())
        {
        }

        void set(const T &newVal)
        {
            if (avail && val != newVal) {
                val = newVal;
                if (changedSignalFunction) {
                    // clang-format off
                    Q_EMIT (device->*changedSignalFunction)();
                    // clang-format on
                    Q_EMIT device->needsSaveChanged();
                }
            }
        }
        void set(const Prop<T> &other)
        {
            if (avail && val != other.val) {
                val = other.val;
                if (changedSignalFunction) {
                    // clang-format off
                    Q_EMIT (device->*changedSignalFunction)();
                    // clang-format on
                    Q_EMIT device->needsSaveChanged();
                }
            }
        }
        bool isSaveNeeded() const
        {
            return avail && (old != val);
        }

        QString dbus;
        bool avail;
        const ChangedSignal changedSignalFunction;
        KWinWaylandDevice *const device;
        T old;
        T val;
    };

    template<typename T>
    bool valueLoader(const QVariantMap &properties, Prop<T> &prop);

    template<typename T>
    bool valueWriter(const Prop<T> &prop);

    //
    // general
    Prop<QString> m_name{this, u"name"_s};
    Prop<QString> m_sysName{this, u"sysName"_s};
    Prop<bool> m_supportsDisableEvents{this, u"supportsDisableEvents"_s};
    Prop<bool> m_enabled{this, u"enabled"_s, &KWinWaylandDevice::enabledChanged};

    //
    // advanced
    Prop<Qt::MouseButtons> m_supportedButtons{this, u"supportedButtons"_s};

    Prop<bool> m_supportsLeftHanded{this, u"supportsLeftHanded"_s};
    Prop<bool> m_leftHandedEnabledByDefault{this, u"leftHandedEnabledByDefault"_s};
    Prop<bool> m_leftHanded{this, u"leftHanded"_s, &KWinWaylandDevice::leftHandedChanged};

    Prop<bool> m_supportsMiddleEmulation{this, u"supportsMiddleEmulation"_s};
    Prop<bool> m_middleEmulationEnabledByDefault{this, u"middleEmulationEnabledByDefault"_s};
    Prop<bool> m_middleEmulation{this, u"middleEmulation"_s, &KWinWaylandDevice::middleEmulationChanged};

    //
    // acceleration speed and profile
    Prop<bool> m_supportsPointerAcceleration{this, u"supportsPointerAcceleration"_s};
    Prop<qreal> m_defaultPointerAcceleration{this, u"defaultPointerAcceleration"_s};
    Prop<qreal> m_pointerAcceleration{this, u"pointerAcceleration"_s, &KWinWaylandDevice::pointerAccelerationChanged};

    Prop<bool> m_supportsPointerAccelerationProfileFlat{this, u"supportsPointerAccelerationProfileFlat"_s};
    Prop<bool> m_defaultPointerAccelerationProfileFlat{this, u"defaultPointerAccelerationProfileFlat"_s};
    Prop<bool> m_pointerAccelerationProfileFlat{this, u"pointerAccelerationProfileFlat"_s, &KWinWaylandDevice::pointerAccelerationProfileFlatChanged};

    Prop<bool> m_supportsPointerAccelerationProfileAdaptive{this, u"supportsPointerAccelerationProfileAdaptive"_s};
    Prop<bool> m_defaultPointerAccelerationProfileAdaptive{this, u"defaultPointerAccelerationProfileAdaptive"_s};
    Prop<bool> m_pointerAccelerationProfileAdaptive{this,
                                                    u"pointerAccelerationProfileAdaptive"_s,
                                                    &KWinWaylandDevice::pointerAccelerationProfileAdaptiveChanged};

    //
    // scrolling
    Prop<bool> m_supportsNaturalScroll{this, u"supportsNaturalScroll"_s};
    Prop<bool> m_naturalScrollEnabledByDefault{this, u"naturalScrollEnabledByDefault"_s};
    Prop<bool> m_naturalScroll{this, u"naturalScroll"_s, &KWinWaylandDevice::naturalScrollChanged};
    Prop<qreal> m_scrollFactor{this, u"scrollFactor"_s, &KWinWaylandDevice::scrollFactorChanged};
    Prop<bool> m_supportsScrollOnButtonDown{this, u"supportsScrollOnButtonDown"_s};
    Prop<bool> m_scrollOnButtonDownEnabledByDefault{this, u"scrollOnButtonDownEnabledByDefault"_s};
    Prop<bool> m_scrollOnButtonDown{this, u"scrollOnButtonDown"_s, &KWinWaylandDevice::scrollOnButtonDownChanged};

    QString m_dbusName;
};
