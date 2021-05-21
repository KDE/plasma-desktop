/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWINWAYLANDDEVICE_H
#define KWINWAYLANDDEVICE_H

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
                   pointerAccelerationProfileChanged)

    Q_PROPERTY(bool supportsPointerAccelerationProfileAdaptive READ supportsPointerAccelerationProfileAdaptive CONSTANT)
    Q_PROPERTY(bool defaultPointerAccelerationProfileAdaptive READ defaultPointerAccelerationProfileAdaptive CONSTANT)
    Q_PROPERTY(bool pointerAccelerationProfileAdaptive READ pointerAccelerationProfileAdaptive WRITE setPointerAccelerationProfileAdaptive NOTIFY
                   pointerAccelerationProfileChanged)

    //
    // scrolling
    Q_PROPERTY(bool supportsNaturalScroll READ supportsNaturalScroll CONSTANT)
    Q_PROPERTY(bool naturalScrollEnabledByDefault READ naturalScrollEnabledByDefault CONSTANT)
    Q_PROPERTY(bool naturalScroll READ isNaturalScroll WRITE setNaturalScroll NOTIFY naturalScrollChanged)
    Q_PROPERTY(qreal scrollFactor READ scrollFactor WRITE setScrollFactor NOTIFY scrollFactorChanged)

public:
    KWinWaylandDevice(QString dbusName);
    ~KWinWaylandDevice() override;

    bool init();

    bool getConfig();
    bool getDefaultConfig();
    bool applyConfig();
    bool isChangedConfig() const;

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
    void leftHandedChanged();
    void pointerAccelerationChanged();
    void pointerAccelerationProfileChanged();
    void enabledChanged();
    void middleEmulationChanged();
    void naturalScrollChanged();
    void scrollFactorChanged();

private:
    template<typename T>
    struct Prop {
        explicit Prop(const QByteArray &dbusName)
            : dbus(dbusName)
        {
        }

        void set(T newVal)
        {
            if (avail && val != newVal) {
                val = newVal;
            }
        }
        void set(const Prop<T> &p)
        {
            if (avail && val != p.val) {
                val = p.val;
            }
        }
        bool changed() const
        {
            return avail && (old != val);
        }

        QByteArray dbus;
        bool avail;
        T old;
        T val;
    };

    template<typename T>
    bool valueLoader(Prop<T> &prop);

    template<typename T>
    QString valueWriter(const Prop<T> &prop);

    //
    // general
    Prop<QString> m_name = Prop<QString>("name");
    Prop<QString> m_sysName = Prop<QString>("sysName");
    Prop<bool> m_supportsDisableEvents = Prop<bool>("supportsDisableEvents");
    Prop<bool> m_enabled = Prop<bool>("enabled");

    //
    // advanced
    Prop<Qt::MouseButtons> m_supportedButtons = Prop<Qt::MouseButtons>("supportedButtons");

    Prop<bool> m_supportsLeftHanded = Prop<bool>("supportsLeftHanded");
    Prop<bool> m_leftHandedEnabledByDefault = Prop<bool>("leftHandedEnabledByDefault");
    Prop<bool> m_leftHanded = Prop<bool>("leftHanded");

    Prop<bool> m_supportsMiddleEmulation = Prop<bool>("supportsMiddleEmulation");
    Prop<bool> m_middleEmulationEnabledByDefault = Prop<bool>("middleEmulationEnabledByDefault");
    Prop<bool> m_middleEmulation = Prop<bool>("middleEmulation");

    //
    // acceleration speed and profile
    Prop<bool> m_supportsPointerAcceleration = Prop<bool>("supportsPointerAcceleration");
    Prop<qreal> m_defaultPointerAcceleration = Prop<qreal>("defaultPointerAcceleration");
    Prop<qreal> m_pointerAcceleration = Prop<qreal>("pointerAcceleration");

    Prop<bool> m_supportsPointerAccelerationProfileFlat = Prop<bool>("supportsPointerAccelerationProfileFlat");
    Prop<bool> m_defaultPointerAccelerationProfileFlat = Prop<bool>("defaultPointerAccelerationProfileFlat");
    Prop<bool> m_pointerAccelerationProfileFlat = Prop<bool>("pointerAccelerationProfileFlat");

    Prop<bool> m_supportsPointerAccelerationProfileAdaptive = Prop<bool>("supportsPointerAccelerationProfileAdaptive");
    Prop<bool> m_defaultPointerAccelerationProfileAdaptive = Prop<bool>("defaultPointerAccelerationProfileAdaptive");
    Prop<bool> m_pointerAccelerationProfileAdaptive = Prop<bool>("pointerAccelerationProfileAdaptive");

    //
    // scrolling
    Prop<bool> m_supportsNaturalScroll = Prop<bool>("supportsNaturalScroll");
    Prop<bool> m_naturalScrollEnabledByDefault = Prop<bool>("naturalScrollEnabledByDefault");
    Prop<bool> m_naturalScroll = Prop<bool>("naturalScroll");
    Prop<qreal> m_scrollFactor = Prop<qreal>("scrollFactor");

    QDBusInterface *m_iface;
};

#endif // KWINWAYLANDDEVICE_H
