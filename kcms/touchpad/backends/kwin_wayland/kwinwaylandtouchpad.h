/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

class QDBusInterface;

namespace
{
template<typename T>
inline T valueLoaderPart(QVariant const &reply)
{
    Q_UNUSED(reply);
    return T();
}

template<>
inline bool valueLoaderPart(QVariant const &reply)
{
    return reply.toBool();
}

template<>
inline int valueLoaderPart(QVariant const &reply)
{
    return reply.toInt();
}

template<>
inline quint32 valueLoaderPart(QVariant const &reply)
{
    return reply.toInt();
}

template<>
inline qreal valueLoaderPart(QVariant const &reply)
{
    return reply.toReal();
}

template<>
inline QString valueLoaderPart(QVariant const &reply)
{
    return reply.toString();
}

template<>
inline Qt::MouseButtons valueLoaderPart(QVariant const &reply)
{
    return static_cast<Qt::MouseButtons>(reply.toInt());
}
}

class KWinWaylandTouchpad : public QObject
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

    Q_PROPERTY(bool supportsDisableEventsOnExternalMouse READ supportsDisableEventsOnExternalMouse CONSTANT)
    Q_PROPERTY(bool disableEventsOnExternalMouseEnabledByDefault READ disableEventsOnExternalMouseEnabledByDefault CONSTANT)
    Q_PROPERTY(
        bool disableEventsOnExternalMouse READ isDisableEventsOnExternalMouse WRITE setDisableEventsOnExternalMouse NOTIFY disableEventsOnExternalMouseChanged)

    Q_PROPERTY(bool supportsDisableWhileTyping READ supportsDisableWhileTyping CONSTANT)
    Q_PROPERTY(bool disableWhileTypingEnabledByDefault READ disableWhileTypingEnabledByDefault CONSTANT)
    Q_PROPERTY(bool disableWhileTyping READ isDisableWhileTyping WRITE setDisableWhileTyping NOTIFY disableWhileTypingChanged)

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
    // tapping
    Q_PROPERTY(int tapFingerCount READ tapFingerCount CONSTANT)
    Q_PROPERTY(bool tapToClickEnabledByDefault READ tapToClickEnabledByDefault CONSTANT)
    Q_PROPERTY(bool tapToClick READ isTapToClick WRITE setTapToClick NOTIFY tapToClickChanged)

    Q_PROPERTY(bool supportsLmrTapButtonMap READ supportsLmrTapButtonMap CONSTANT)
    Q_PROPERTY(bool lmrTapButtonMapEnabledByDefault READ lmrTapButtonMapEnabledByDefault CONSTANT)
    Q_PROPERTY(bool lmrTapButtonMap READ lmrTapButtonMap WRITE setLmrTapButtonMap NOTIFY lmrTapButtonMapChanged)

    Q_PROPERTY(bool tapAndDragEnabledByDefault READ tapAndDragEnabledByDefault CONSTANT)
    Q_PROPERTY(bool tapAndDrag READ isTapAndDrag WRITE setTapAndDrag NOTIFY tapAndDragChanged)

    Q_PROPERTY(bool tapDragLockEnabledByDefault READ tapDragLockEnabledByDefault CONSTANT)
    Q_PROPERTY(bool tapDragLock READ isTapDragLock WRITE setTapDragLock NOTIFY tapDragLockChanged)

    //
    // scrolling
    Q_PROPERTY(bool supportsNaturalScroll READ supportsNaturalScroll CONSTANT)
    Q_PROPERTY(bool naturalScrollEnabledByDefault READ naturalScrollEnabledByDefault CONSTANT)
    Q_PROPERTY(bool naturalScroll READ isNaturalScroll WRITE setNaturalScroll NOTIFY naturalScrollChanged)

    Q_PROPERTY(bool supportsHorizontalScrolling READ supportsHorizontalScrolling CONSTANT)
    Q_PROPERTY(bool horizontalScrollingByDefault READ horizontalScrollingByDefault CONSTANT)
    Q_PROPERTY(bool horizontalScrolling READ horizontalScrolling WRITE setHorizontalScrolling NOTIFY horizontalScrollingChanged)

    Q_PROPERTY(bool supportsScrollTwoFinger READ supportsScrollTwoFinger CONSTANT)
    Q_PROPERTY(bool scrollTwoFingerEnabledByDefault READ scrollTwoFingerEnabledByDefault CONSTANT)
    Q_PROPERTY(bool scrollTwoFinger READ isScrollTwoFinger WRITE setScrollTwoFinger NOTIFY scrollTwoFingerChanged)

    Q_PROPERTY(bool supportsScrollEdge READ supportsScrollEdge CONSTANT)
    Q_PROPERTY(bool scrollEdgeEnabledByDefault READ scrollEdgeEnabledByDefault CONSTANT)
    Q_PROPERTY(bool scrollEdge READ isScrollEdge WRITE setScrollEdge NOTIFY scrollEdgeChanged)

    Q_PROPERTY(bool supportsScrollOnButtonDown READ supportsScrollOnButtonDown CONSTANT)
    Q_PROPERTY(bool scrollOnButtonDownEnabledByDefault READ scrollOnButtonDownEnabledByDefault CONSTANT)
    Q_PROPERTY(bool scrollOnButtonDown READ isScrollOnButtonDown WRITE setScrollOnButtonDown NOTIFY scrollOnButtonDownChanged)

    Q_PROPERTY(quint32 defaultScrollButton READ defaultScrollButton CONSTANT)
    Q_PROPERTY(quint32 scrollButton READ scrollButton WRITE setScrollButton NOTIFY scrollButtonChanged)

    // Click Methods
    Q_PROPERTY(bool supportsClickMethodAreas READ supportsClickMethodAreas CONSTANT)
    Q_PROPERTY(bool defaultClickMethodAreas READ defaultClickMethodAreas CONSTANT)
    Q_PROPERTY(bool clickMethodAreas READ isClickMethodAreas WRITE setClickMethodAreas NOTIFY clickMethodAreasChanged)

    Q_PROPERTY(bool supportsClickMethodClickfinger READ supportsClickMethodClickfinger CONSTANT)
    Q_PROPERTY(bool defaultClickMethodClickfinger READ defaultClickMethodClickfinger CONSTANT)
    Q_PROPERTY(bool clickMethodClickfinger READ isClickMethodClickfinger WRITE setClickMethodClickfinger NOTIFY clickMethodClickfingerChanged)

    Q_PROPERTY(bool supportsScrollFactor READ supportsScrollFactor CONSTANT)

    Q_PROPERTY(qreal scrollFactor READ scrollFactor WRITE setScrollFactor NOTIFY scrollFactorChanged)

public:
    explicit KWinWaylandTouchpad(const QString &dbusName);
    ~KWinWaylandTouchpad() override;

    bool load();
    bool defaults();
    bool save();
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
    //
    // advanced
    Qt::MouseButtons supportedButtons() const
    {
        return m_supportedButtons.val;
    }
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
    bool supportsDisableEventsOnExternalMouse() const
    {
        return m_supportsDisableEventsOnExternalMouse.val;
    }
    bool disableEventsOnExternalMouseEnabledByDefault() const
    {
        return m_disableEventsOnExternalMouseEnabledByDefault.val;
    }
    bool isDisableEventsOnExternalMouse() const
    {
        return m_disableEventsOnExternalMouse.val;
    }
    void setDisableEventsOnExternalMouse(bool set)
    {
        m_disableEventsOnExternalMouse.set(set);
    }
    bool supportsDisableWhileTyping() const
    {
        return m_supportsDisableWhileTyping.val;
    }
    bool disableWhileTypingEnabledByDefault() const
    {
        return m_disableWhileTypingEnabledByDefault.val;
    }
    bool isDisableWhileTyping() const
    {
        return m_disableWhileTyping.val;
    }
    void setDisableWhileTyping(bool set)
    {
        m_disableWhileTyping.set(set);
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
    // tapping
    void setLmrTapButtonMap(bool set)
    {
        m_lmrTapButtonMap.set(set);
    }
    int tapFingerCount() const
    {
        return m_tapFingerCount.val;
    }
    bool tapToClickEnabledByDefault() const
    {
        return m_tapToClickEnabledByDefault.val;
    }
    bool isTapToClick() const
    {
        return m_tapToClick.val;
    }
    void setTapToClick(bool set)
    {
        m_tapToClick.set(set);
    }

    bool supportsLmrTapButtonMap() const
    {
        return m_tapFingerCount.val > 1;
    }
    bool lmrTapButtonMapEnabledByDefault() const
    {
        return m_lmrTapButtonMapEnabledByDefault.val;
    }
    bool lmrTapButtonMap() const
    {
        return m_lmrTapButtonMap.val;
    }
    bool tapAndDragEnabledByDefault() const
    {
        return m_tapAndDragEnabledByDefault.val;
    }
    bool isTapAndDrag() const
    {
        return m_tapAndDrag.val;
    }
    void setTapAndDrag(bool set)
    {
        m_tapAndDrag.set(set);
    }

    bool tapDragLockEnabledByDefault() const
    {
        return m_tapDragLockEnabledByDefault.val;
    }
    bool isTapDragLock() const
    {
        return m_tapDragLock.val;
    }
    void setTapDragLock(bool set)
    {
        m_tapDragLock.set(set);
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
    bool supportsHorizontalScrolling() const
    {
        return false;
    }
    bool horizontalScrollingByDefault() const
    {
        return true;
    }
    bool horizontalScrolling() const
    {
        return m_horizontalScrolling.val;
    }
    void setHorizontalScrolling(bool set)
    {
        m_horizontalScrolling.set(set);
    }
    bool supportsScrollTwoFinger() const
    {
        return m_supportsScrollTwoFinger.val;
    }
    bool scrollTwoFingerEnabledByDefault() const
    {
        return m_scrollTwoFingerEnabledByDefault.val;
    }
    bool isScrollTwoFinger() const
    {
        return m_isScrollTwoFinger.val;
    }
    void setScrollTwoFinger(bool set)
    {
        m_isScrollTwoFinger.set(set);
    }
    bool supportsScrollEdge() const
    {
        return m_supportsScrollEdge.val;
    }
    bool scrollEdgeEnabledByDefault() const
    {
        return m_scrollEdgeEnabledByDefault.val;
    }
    bool isScrollEdge() const
    {
        return m_isScrollEdge.val;
    }
    void setScrollEdge(bool set)
    {
        m_isScrollEdge.set(set);
    }
    bool supportsScrollOnButtonDown() const
    {
        return m_supportsScrollOnButtonDown.val;
    }
    bool scrollOnButtonDownEnabledByDefault() const
    {
        return m_scrollOnButtonDownEnabledByDefault.val;
    }
    bool isScrollOnButtonDown() const
    {
        return m_isScrollOnButtonDown.val;
    }
    void setScrollOnButtonDown(bool set)
    {
        m_isScrollOnButtonDown.set(set);
    }
    quint32 defaultScrollButton() const
    {
        return m_defaultScrollButton.val;
    }
    quint32 scrollButton() const
    {
        return m_scrollButton.val;
    }
    void setScrollButton(quint32 button)
    {
        m_scrollButton.set(button);
    }

    //
    // Scroll Factor
    bool supportsScrollFactor() const
    {
        return true;
    }
    qreal scrollFactor() const
    {
        return m_scrollFactor.val;
    }
    void setScrollFactor(qreal factor)
    {
        return m_scrollFactor.set(factor);
    }

    //
    // Click method
    bool supportsClickMethodAreas() const
    {
        return m_supportsClickMethodAreas.val;
    }
    bool defaultClickMethodAreas() const
    {
        return m_defaultClickMethodAreas.val;
    }
    bool isClickMethodAreas() const
    {
        return m_clickMethodAreas.val;
    }
    void setClickMethodAreas(bool set)
    {
        m_clickMethodAreas.set(set);
    }
    bool supportsClickMethodClickfinger() const
    {
        return m_supportsClickMethodClickfinger.val;
    }
    bool defaultClickMethodClickfinger() const
    {
        return m_defaultClickMethodClickfinger.val;
    }
    bool isClickMethodClickfinger() const
    {
        return m_clickMethodClickfinger.val;
    }
    void setClickMethodClickfinger(bool set)
    {
        m_clickMethodClickfinger.set(set);
    }

Q_SIGNALS:
    void needsSaveChanged();

    void enabledChanged();
    // Tapping
    void tapToClickChanged();
    void lmrTapButtonMapChanged();
    void tapAndDragChanged();
    void tapDragLockChanged();
    // Advanced
    void leftHandedChanged();
    void disableEventsOnExternalMouseChanged();
    void disableWhileTypingChanged();
    void middleEmulationChanged();
    // acceleration speed and profile
    void pointerAccelerationChanged();
    void pointerAccelerationProfileFlatChanged();
    void pointerAccelerationProfileAdaptiveChanged();
    // scrolling
    void naturalScrollChanged();
    void horizontalScrollingChanged();
    void scrollTwoFingerChanged();
    void scrollEdgeChanged();
    void scrollOnButtonDownChanged();
    void scrollButtonChanged();
    // the only unique property to the Wayland backend
    void scrollFactorChanged();
    // click methods
    void clickMethodAreasChanged();
    void clickMethodClickfingerChanged();

private:
    template<typename T>
    struct Prop {
        using ChangedSignal = void (KWinWaylandTouchpad::*)();

        explicit Prop(KWinWaylandTouchpad *device, const QString &name, T initialValue, ChangedSignal changedSignal = nullptr)
            : name(name)
            , avail(false)
            , changedSignalFunction(changedSignal)
            , device(device)
            , old(initialValue)
            , val(initialValue)
        {
        }

        void set(T newVal)
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
        void set(const Prop<T> &p)
        {
            set(p.val);
        }
        bool changed() const
        {
            return avail && (old != val);
        }

        // In wayland, name will be dbus name
        QString name;
        bool avail;
        const ChangedSignal changedSignalFunction;
        KWinWaylandTouchpad *const device;
        T old;
        T val;
    };

    struct PropInt : public Prop<int> {
        explicit PropInt(KWinWaylandTouchpad *device, const QString &name, ChangedSignal changedSignal = nullptr)
            : Prop<int>(device, name, 0, changedSignal)
        {
        }
    };

    struct PropReal : public Prop<qreal> {
        explicit PropReal(KWinWaylandTouchpad *device, const QString &name, ChangedSignal changedSignal = nullptr)
            : Prop<qreal>(device, name, 0, changedSignal)
        {
        }
    };

    struct PropBool : public Prop<bool> {
        explicit PropBool(KWinWaylandTouchpad *device, const QString &name, ChangedSignal changedSignal = nullptr)
            : Prop<bool>(device, name, false, changedSignal)
        {
        }
    };

    template<typename T>
    bool valueLoader(const QVariantMap &properties, Prop<T> &prop);

    template<typename T>
    bool valueWriter(const Prop<T> &prop);

    //
    // general
    PropBool m_supportsDisableEvents = PropBool(this, "supportsDisableEvents");
    PropBool m_enabledDefault = PropBool(this, "enabledDefault");
    PropBool m_enabled = PropBool(this, "enabled", &KWinWaylandTouchpad::enabledChanged);

    //
    // advanced
    Prop<Qt::MouseButtons> m_supportedButtons = Prop<Qt::MouseButtons>(this, "supportedButtons", Qt::MouseButton::NoButton);

    PropBool m_leftHandedEnabledByDefault = PropBool(this, "leftHandedEnabledByDefault");
    PropBool m_leftHanded = PropBool(this, "leftHanded", &KWinWaylandTouchpad::leftHandedChanged);

    PropBool m_supportsDisableEventsOnExternalMouse = PropBool(this, "supportsDisableEventsOnExternalMouse");
    PropBool m_disableEventsOnExternalMouseEnabledByDefault = PropBool(this, "disableEventsOnExternalMouseEnabledByDefault");
    PropBool m_disableEventsOnExternalMouse = PropBool(this, "disableEventsOnExternalMouse", &KWinWaylandTouchpad::disableEventsOnExternalMouseChanged);

    PropBool m_disableWhileTypingEnabledByDefault = PropBool(this, "disableWhileTypingEnabledByDefault");
    PropBool m_disableWhileTyping = PropBool(this, "disableWhileTyping", &KWinWaylandTouchpad::disableWhileTypingChanged);

    PropBool m_middleEmulationEnabledByDefault = PropBool(this, "middleEmulationEnabledByDefault");
    PropBool m_middleEmulation = PropBool(this, "middleEmulation", &KWinWaylandTouchpad::middleEmulationChanged);

    //
    // acceleration speed and profile
    Prop<qreal> m_defaultPointerAcceleration = PropReal(this, "defaultPointerAcceleration");
    Prop<qreal> m_pointerAcceleration = PropReal(this, "pointerAcceleration", &KWinWaylandTouchpad::pointerAccelerationChanged);

    PropBool m_supportsPointerAccelerationProfileFlat = PropBool(this, "supportsPointerAccelerationProfileFlat");
    PropBool m_defaultPointerAccelerationProfileFlat = PropBool(this, "defaultPointerAccelerationProfileFlat");
    PropBool m_pointerAccelerationProfileFlat = PropBool(this, "pointerAccelerationProfileFlat", &KWinWaylandTouchpad::pointerAccelerationProfileFlatChanged);

    PropBool m_supportsPointerAccelerationProfileAdaptive = PropBool(this, "supportsPointerAccelerationProfileAdaptive");
    PropBool m_defaultPointerAccelerationProfileAdaptive = PropBool(this, "defaultPointerAccelerationProfileAdaptive");
    PropBool m_pointerAccelerationProfileAdaptive =
        PropBool(this, "pointerAccelerationProfileAdaptive", &KWinWaylandTouchpad::pointerAccelerationProfileAdaptiveChanged);

    //
    // tapping
    Prop<int> m_tapFingerCount = PropInt(this, "tapFingerCount");
    PropBool m_tapToClickEnabledByDefault = PropBool(this, "tapToClickEnabledByDefault");
    PropBool m_tapToClick = PropBool(this, "tapToClick", &KWinWaylandTouchpad::tapToClickChanged);

    PropBool m_lmrTapButtonMapEnabledByDefault = PropBool(this, "lmrTapButtonMapEnabledByDefault");
    PropBool m_lmrTapButtonMap = PropBool(this, "lmrTapButtonMap", &KWinWaylandTouchpad::lmrTapButtonMapChanged);

    PropBool m_tapAndDragEnabledByDefault = PropBool(this, "tapAndDragEnabledByDefault");
    PropBool m_tapAndDrag = PropBool(this, "tapAndDrag", &KWinWaylandTouchpad::tapAndDragChanged);
    PropBool m_tapDragLockEnabledByDefault = PropBool(this, "tapDragLockEnabledByDefault");
    PropBool m_tapDragLock = PropBool(this, "tapDragLock", &KWinWaylandTouchpad::tapDragLockChanged);

    //
    // scrolling
    PropBool m_naturalScrollEnabledByDefault = PropBool(this, "naturalScrollEnabledByDefault");
    PropBool m_naturalScroll = PropBool(this, "naturalScroll", &KWinWaylandTouchpad::naturalScrollChanged);

    PropBool m_horizontalScrolling = PropBool(this, "horizontalScrolling", &KWinWaylandTouchpad::horizontalScrollingChanged);

    PropBool m_supportsScrollTwoFinger = PropBool(this, "supportsScrollTwoFinger");
    PropBool m_scrollTwoFingerEnabledByDefault = PropBool(this, "scrollTwoFingerEnabledByDefault");
    PropBool m_isScrollTwoFinger = PropBool(this, "scrollTwoFinger", &KWinWaylandTouchpad::scrollTwoFingerChanged);

    PropBool m_supportsScrollEdge = PropBool(this, "supportsScrollEdge");
    PropBool m_scrollEdgeEnabledByDefault = PropBool(this, "scrollEdgeEnabledByDefault");
    PropBool m_isScrollEdge = PropBool(this, "scrollEdge", &KWinWaylandTouchpad::scrollEdgeChanged);

    PropBool m_supportsScrollOnButtonDown = PropBool(this, "supportsScrollOnButtonDown");
    PropBool m_scrollOnButtonDownEnabledByDefault = PropBool(this, "scrollOnButtonDownEnabledByDefault");
    PropBool m_isScrollOnButtonDown = PropBool(this, "scrollOnButtonDown", &KWinWaylandTouchpad::scrollOnButtonDownChanged);

    Prop<quint32> m_defaultScrollButton = Prop<quint32>(this, "defaultScrollButton", 0);
    Prop<quint32> m_scrollButton = Prop<quint32>(this, "scrollButton", 0, &KWinWaylandTouchpad::scrollButtonChanged);

    // Click Method
    PropBool m_supportsClickMethodAreas = PropBool(this, "supportsClickMethodAreas");
    PropBool m_defaultClickMethodAreas = PropBool(this, "defaultClickMethodAreas");
    PropBool m_clickMethodAreas = PropBool(this, "clickMethodAreas", &KWinWaylandTouchpad::clickMethodAreasChanged);

    PropBool m_supportsClickMethodClickfinger = PropBool(this, "supportsClickMethodClickfinger");
    PropBool m_defaultClickMethodClickfinger = PropBool(this, "defaultClickMethodClickfinger");
    PropBool m_clickMethodClickfinger = PropBool(this, "clickMethodClickfinger", &KWinWaylandTouchpad::clickMethodClickfingerChanged);

    //
    // general
    Prop<QString> m_name = Prop<QString>(this, "name", QString());
    Prop<QString> m_sysName = Prop<QString>(this, "sysName", QString());

    //
    // advanced
    Prop<bool> m_supportsLeftHanded = PropBool(this, "supportsLeftHanded");
    Prop<bool> m_supportsDisableWhileTyping = PropBool(this, "supportsDisableWhileTyping");
    Prop<bool> m_supportsMiddleEmulation = PropBool(this, "supportsMiddleEmulation");

    //
    // acceleration speed and profile
    Prop<bool> m_supportsPointerAcceleration = PropBool(this, "supportsPointerAcceleration");

    //
    // scrolling
    Prop<bool> m_supportsNaturalScroll = PropBool(this, "supportsNaturalScroll");
    Prop<qreal> m_scrollFactor = Prop<qreal>(this, "scrollFactor", 0, &KWinWaylandTouchpad::scrollFactorChanged);

    QString m_dbusName;
};
