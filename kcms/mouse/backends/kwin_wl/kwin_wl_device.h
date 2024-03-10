/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>

class QDBusInterface;

class KWinWaylandDevice : public QObject
{
    Q_OBJECT

    //
    // general
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(bool supportsDisableEvents READ supportsDisableEvents CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)

    //
    // advanced
    Q_PROPERTY(Qt::MouseButtons supportedButtons READ supportedButtons CONSTANT)

    Q_PROPERTY(bool supportsLeftHanded READ supportsLeftHanded CONSTANT)
    Q_PROPERTY(bool leftHandedEnabledByDefault READ leftHandedEnabledByDefault CONSTANT)
    Q_PROPERTY(bool leftHanded READ isLeftHanded WRITE setLeftHanded NOTIFY leftHandedChanged)

    Q_PROPERTY(bool supportsMiddleEmulation READ supportsMiddleEmulation CONSTANT)
    Q_PROPERTY(bool middleEmulationEnabledByDefault READ middleEmulationEnabledByDefault CONSTANT)
    Q_PROPERTY(bool middleEmulation READ isMiddleEmulation WRITE setMiddleEmulation NOTIFY middleEmulationChanged)

    //
    // acceleration speed and profile
    Q_PROPERTY(bool supportsPointerAcceleration READ supportsPointerAcceleration CONSTANT)
    Q_PROPERTY(qreal pointerAcceleration READ pointerAcceleration WRITE setPointerAcceleration NOTIFY pointerAccelerationChanged)

    Q_PROPERTY(bool supportsPointerAccelerationProfileFlat READ supportsPointerAccelerationProfileFlat CONSTANT)
    Q_PROPERTY(bool defaultPointerAccelerationProfileFlat READ defaultPointerAccelerationProfileFlat CONSTANT)
    Q_PROPERTY(bool pointerAccelerationProfileFlat READ pointerAccelerationProfileFlat WRITE setPointerAccelerationProfileFlat NOTIFY
                   pointerAccelerationProfileFlatChanged)

    Q_PROPERTY(bool supportsPointerAccelerationProfileAdaptive READ supportsPointerAccelerationProfileAdaptive CONSTANT)
    Q_PROPERTY(bool defaultPointerAccelerationProfileAdaptive READ defaultPointerAccelerationProfileAdaptive CONSTANT)
    Q_PROPERTY(bool pointerAccelerationProfileAdaptive READ pointerAccelerationProfileAdaptive WRITE setPointerAccelerationProfileAdaptive NOTIFY
                   pointerAccelerationProfileAdaptiveChanged)

    //
    // scrolling
    Q_PROPERTY(bool supportsNaturalScroll READ supportsNaturalScroll CONSTANT)
    Q_PROPERTY(bool naturalScrollEnabledByDefault READ naturalScrollEnabledByDefault CONSTANT)
    Q_PROPERTY(bool naturalScroll READ isNaturalScroll WRITE setNaturalScroll NOTIFY naturalScrollChanged)
    Q_PROPERTY(qreal scrollFactor READ scrollFactor WRITE setScrollFactor NOTIFY scrollFactorChanged)

public:
    KWinWaylandDevice(const QString &dbusName);
    ~KWinWaylandDevice() override;

    bool init();

    bool getDefaultConfig();
    bool applyConfig();
    bool isSaveNeeded() const;

    //
    // general
    QString name() const
    {
        return m_name.val;
    }
    QString sysName() const
    {
        return m_sysName.val;
    }
    bool supportsDisableEvents() const
    {
        return m_supportsDisableEvents.val;
    }
    void setEnabled(bool enabled)
    {
        m_enabled.set(enabled);
    }
    bool isEnabled() const
    {
        return m_enabled.val;
    }
    Qt::MouseButtons supportedButtons() const
    {
        return m_supportedButtons.val;
    }

    //
    // advanced
    bool supportsLeftHanded() const
    {
        return m_supportsLeftHanded.val;
    }
    bool leftHandedEnabledByDefault() const
    {
        return m_leftHandedEnabledByDefault.val;
    }
    bool isLeftHanded() const
    {
        return m_leftHanded.val;
    }
    void setLeftHanded(bool set)
    {
        m_leftHanded.set(set);
    }

    bool supportsMiddleEmulation() const
    {
        return m_supportsMiddleEmulation.val;
    }
    bool middleEmulationEnabledByDefault() const
    {
        return m_middleEmulationEnabledByDefault.val;
    }
    bool isMiddleEmulation() const
    {
        return m_middleEmulation.val;
    }
    void setMiddleEmulation(bool set)
    {
        m_middleEmulation.set(set);
    }

    //
    // acceleration speed and profile
    bool supportsPointerAcceleration() const
    {
        return m_supportsPointerAcceleration.val;
    }
    qreal pointerAcceleration() const
    {
        return m_pointerAcceleration.val;
    }
    void setPointerAcceleration(qreal acceleration)
    {
        m_pointerAcceleration.set(acceleration);
    }

    bool supportsPointerAccelerationProfileFlat() const
    {
        return m_supportsPointerAccelerationProfileFlat.val;
    }
    bool defaultPointerAccelerationProfileFlat() const
    {
        return m_defaultPointerAccelerationProfileFlat.val;
    }
    bool pointerAccelerationProfileFlat() const
    {
        return m_pointerAccelerationProfileFlat.val;
    }
    void setPointerAccelerationProfileFlat(bool set)
    {
        m_pointerAccelerationProfileFlat.set(set);
    }

    bool supportsPointerAccelerationProfileAdaptive() const
    {
        return m_supportsPointerAccelerationProfileAdaptive.val;
    }
    bool defaultPointerAccelerationProfileAdaptive() const
    {
        return m_defaultPointerAccelerationProfileAdaptive.val;
    }
    bool pointerAccelerationProfileAdaptive() const
    {
        return m_pointerAccelerationProfileAdaptive.val;
    }
    void setPointerAccelerationProfileAdaptive(bool set)
    {
        m_pointerAccelerationProfileAdaptive.set(set);
    }

    //
    // scrolling
    bool supportsNaturalScroll() const
    {
        return m_supportsNaturalScroll.val;
    }
    bool naturalScrollEnabledByDefault() const
    {
        return m_naturalScrollEnabledByDefault.val;
    }
    bool isNaturalScroll() const
    {
        return m_naturalScroll.val;
    }
    void setNaturalScroll(bool set)
    {
        m_naturalScroll.set(set);
    }

    qreal scrollFactor() const
    {
        return m_scrollFactor.val;
    }
    void setScrollFactor(qreal set)
    {
        m_scrollFactor.set(set);
    }

Q_SIGNALS:
    void needsSaveChanged();

    void leftHandedChanged();
    void pointerAccelerationChanged();
    void pointerAccelerationProfileFlatChanged();
    void pointerAccelerationProfileAdaptiveChanged();
    void enabledChanged();
    void middleEmulationChanged();
    void naturalScrollChanged();
    void scrollFactorChanged();

private:
    template<typename T>
    struct Prop {
        using value_type = T;
        using ChangedSignal = void (KWinWaylandDevice::*)();

        explicit Prop(KWinWaylandDevice *device, const char *dbusName, ChangedSignal changedSignal = nullptr)
            : dbus(dbusName)
            , changedSignalFunction(changedSignal)
            , device(device)
        {
            if (changedSignalFunction) {
                QObject::connect(device, changedSignalFunction, device, &KWinWaylandDevice::needsSaveChanged);
            }
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

        QLatin1String dbus;
        bool avail;
        const ChangedSignal changedSignalFunction;
        KWinWaylandDevice *const device;
        T old;
        T val;
    };

    template<typename T>
    bool valueLoader(Prop<T> &prop);

    template<typename T>
    QString valueWriter(const Prop<T> &prop);

    //
    // general
    Prop<QString> m_name = Prop<QString>(this, "name");
    Prop<QString> m_sysName = Prop<QString>(this, "sysName");
    Prop<bool> m_supportsDisableEvents = Prop<bool>(this, "supportsDisableEvents");
    Prop<bool> m_enabled = Prop<bool>(this, "enabled", &KWinWaylandDevice::enabledChanged);

    //
    // advanced
    Prop<Qt::MouseButtons> m_supportedButtons = Prop<Qt::MouseButtons>(this, "supportedButtons");

    Prop<bool> m_supportsLeftHanded = Prop<bool>(this, "supportsLeftHanded");
    Prop<bool> m_leftHandedEnabledByDefault = Prop<bool>(this, "leftHandedEnabledByDefault");
    Prop<bool> m_leftHanded = Prop<bool>(this, "leftHanded", &KWinWaylandDevice::leftHandedChanged);

    Prop<bool> m_supportsMiddleEmulation = Prop<bool>(this, "supportsMiddleEmulation");
    Prop<bool> m_middleEmulationEnabledByDefault = Prop<bool>(this, "middleEmulationEnabledByDefault");
    Prop<bool> m_middleEmulation = Prop<bool>(this, "middleEmulation", &KWinWaylandDevice::middleEmulationChanged);

    //
    // acceleration speed and profile
    Prop<bool> m_supportsPointerAcceleration = Prop<bool>(this, "supportsPointerAcceleration");
    Prop<qreal> m_defaultPointerAcceleration = Prop<qreal>(this, "defaultPointerAcceleration");
    Prop<qreal> m_pointerAcceleration = Prop<qreal>(this, "pointerAcceleration", &KWinWaylandDevice::pointerAccelerationChanged);

    Prop<bool> m_supportsPointerAccelerationProfileFlat = Prop<bool>(this, "supportsPointerAccelerationProfileFlat");
    Prop<bool> m_defaultPointerAccelerationProfileFlat = Prop<bool>(this, "defaultPointerAccelerationProfileFlat");
    Prop<bool> m_pointerAccelerationProfileFlat = Prop<bool>(this, "pointerAccelerationProfileFlat", &KWinWaylandDevice::pointerAccelerationProfileFlatChanged);

    Prop<bool> m_supportsPointerAccelerationProfileAdaptive = Prop<bool>(this, "supportsPointerAccelerationProfileAdaptive");
    Prop<bool> m_defaultPointerAccelerationProfileAdaptive = Prop<bool>(this, "defaultPointerAccelerationProfileAdaptive");
    Prop<bool> m_pointerAccelerationProfileAdaptive =
        Prop<bool>(this, "pointerAccelerationProfileAdaptive", &KWinWaylandDevice::pointerAccelerationProfileAdaptiveChanged);

    //
    // scrolling
    Prop<bool> m_supportsNaturalScroll = Prop<bool>(this, "supportsNaturalScroll");
    Prop<bool> m_naturalScrollEnabledByDefault = Prop<bool>(this, "naturalScrollEnabledByDefault");
    Prop<bool> m_naturalScroll = Prop<bool>(this, "naturalScroll", &KWinWaylandDevice::naturalScrollChanged);
    Prop<qreal> m_scrollFactor = Prop<qreal>(this, "scrollFactor", &KWinWaylandDevice::scrollFactorChanged);

    QString m_dbusName;
};
