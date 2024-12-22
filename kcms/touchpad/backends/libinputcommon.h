/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2019 Atul Bisht <atulbisht26@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QVariant>

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

class LibinputCommon : public QObject
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
                   pointerAccelerationProfileChanged)

    Q_PROPERTY(bool supportsPointerAccelerationProfileAdaptive READ supportsPointerAccelerationProfileAdaptive CONSTANT)
    Q_PROPERTY(bool defaultPointerAccelerationProfileAdaptive READ defaultPointerAccelerationProfileAdaptive CONSTANT)
    Q_PROPERTY(bool pointerAccelerationProfileAdaptive READ pointerAccelerationProfileAdaptive WRITE setPointerAccelerationProfileAdaptive NOTIFY
                   pointerAccelerationProfileChanged)

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
    Q_PROPERTY(bool scrollTwoFinger READ isScrollTwoFinger WRITE setScrollTwoFinger NOTIFY scrollMethodChanged)

    Q_PROPERTY(bool supportsScrollEdge READ supportsScrollEdge CONSTANT)
    Q_PROPERTY(bool scrollEdgeEnabledByDefault READ scrollEdgeEnabledByDefault CONSTANT)
    Q_PROPERTY(bool scrollEdge READ isScrollEdge WRITE setScrollEdge NOTIFY scrollMethodChanged)

    Q_PROPERTY(bool supportsScrollOnButtonDown READ supportsScrollOnButtonDown CONSTANT)
    Q_PROPERTY(bool scrollOnButtonDownEnabledByDefault READ scrollOnButtonDownEnabledByDefault CONSTANT)
    Q_PROPERTY(bool scrollOnButtonDown READ isScrollOnButtonDown WRITE setScrollOnButtonDown NOTIFY scrollMethodChanged)

    Q_PROPERTY(quint32 defaultScrollButton READ defaultScrollButton CONSTANT)
    Q_PROPERTY(quint32 scrollButton READ scrollButton WRITE setScrollButton NOTIFY scrollButtonChanged)

    // Click Methods
    Q_PROPERTY(bool supportsClickMethodAreas READ supportsClickMethodAreas CONSTANT)
    Q_PROPERTY(bool defaultClickMethodAreas READ defaultClickMethodAreas CONSTANT)
    Q_PROPERTY(bool clickMethodAreas READ isClickMethodAreas WRITE setClickMethodAreas NOTIFY clickMethodChanged)

    Q_PROPERTY(bool supportsClickMethodClickfinger READ supportsClickMethodClickfinger CONSTANT)
    Q_PROPERTY(bool defaultClickMethodClickfinger READ defaultClickMethodClickfinger CONSTANT)
    Q_PROPERTY(bool clickMethodClickfinger READ isClickMethodClickfinger WRITE setClickMethodClickfinger NOTIFY clickMethodChanged)

    Q_PROPERTY(bool supportsScrollFactor READ supportsScrollFactor CONSTANT)
public:
    LibinputCommon()
    {
    }
    virtual ~LibinputCommon()
    {
    }

    virtual QString name() const = 0;
    virtual bool supportsDisableEvents() const = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool set) = 0;

    //
    // advanced
    Qt::MouseButtons supportedButtons() const
    {
        return m_supportedButtons.val;
    }

    virtual bool supportsLeftHanded() const = 0;
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

    virtual bool supportsDisableEventsOnExternalMouse() const = 0;
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

    virtual bool supportsDisableWhileTyping() const = 0;
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

    virtual bool supportsMiddleEmulation() const = 0;
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

    virtual bool supportsPointerAcceleration() const = 0;
    qreal pointerAcceleration() const
    {
        return m_pointerAcceleration.val;
    }
    void setPointerAcceleration(qreal acceleration)
    {
        m_pointerAcceleration.set(acceleration);
    }

    virtual bool supportsPointerAccelerationProfileFlat() const = 0;
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

    virtual bool supportsPointerAccelerationProfileAdaptive() const = 0;
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
    virtual bool supportsNaturalScroll() const = 0;
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

    virtual bool supportsHorizontalScrolling() const = 0;
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

    virtual bool supportsScrollTwoFinger() const = 0;
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

    virtual bool supportsScrollEdge() const = 0;
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

    virtual bool supportsScrollOnButtonDown() const = 0;
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
    // tapping
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
    virtual void setLmrTapButtonMap(bool set) = 0;

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
    // click method
    virtual bool supportsClickMethodAreas() const = 0;
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

    virtual bool supportsClickMethodClickfinger() const = 0;
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

    virtual bool supportsScrollFactor() const = 0;

Q_SIGNALS:
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
    void pointerAccelerationProfileChanged();
    // scrolling
    void naturalScrollChanged();
    void horizontalScrollingChanged();
    void scrollMethodChanged();
    void scrollButtonChanged();
    // click methods
    void clickMethodChanged();

protected:
    template<typename T>
    struct Prop {
        explicit Prop(const QByteArray &name, T initialValue)
            : name(name)
            , avail(false)
            , old(initialValue)
            , val(initialValue)
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

        // In wayland, name will be dbus name
        QByteArray name;
        bool avail;
        T old;
        T val;
    };

    struct PropInt : public Prop<int> {
        explicit PropInt(const QByteArray &name)
            : Prop<int>(name, 0)
        {
        }
    };

    struct PropReal : public Prop<qreal> {
        explicit PropReal(const QByteArray &name)
            : Prop<qreal>(name, false)
        {
        }
    };

    struct PropBool : public Prop<bool> {
        explicit PropBool(const QByteArray &name)
            : Prop<bool>(name, 0)
        {
        }
    };

    //
    // general
    PropBool m_supportsDisableEvents = PropBool("supportsDisableEvents");
    PropBool m_enabledDefault = PropBool("enabledDefault");
    PropBool m_enabled = PropBool("enabled");

    //
    // advanced
    Prop<Qt::MouseButtons> m_supportedButtons = Prop<Qt::MouseButtons>("supportedButtons", Qt::MouseButton::NoButton);

    PropBool m_leftHandedEnabledByDefault = PropBool("leftHandedEnabledByDefault");
    PropBool m_leftHanded = PropBool("leftHanded");

    PropBool m_supportsDisableEventsOnExternalMouse = PropBool("supportsDisableEventsOnExternalMouse");
    PropBool m_disableEventsOnExternalMouseEnabledByDefault = PropBool("disableEventsOnExternalMouseEnabledByDefault");
    PropBool m_disableEventsOnExternalMouse = PropBool("disableEventsOnExternalMouse");

    PropBool m_disableWhileTypingEnabledByDefault = PropBool("disableWhileTypingEnabledByDefault");
    PropBool m_disableWhileTyping = PropBool("disableWhileTyping");

    PropBool m_middleEmulationEnabledByDefault = PropBool("middleEmulationEnabledByDefault");
    PropBool m_middleEmulation = PropBool("middleEmulation");

    //
    // acceleration speed and profile
    Prop<qreal> m_defaultPointerAcceleration = PropReal("defaultPointerAcceleration");
    Prop<qreal> m_pointerAcceleration = PropReal("pointerAcceleration");

    PropBool m_supportsPointerAccelerationProfileFlat = PropBool("supportsPointerAccelerationProfileFlat");
    PropBool m_defaultPointerAccelerationProfileFlat = PropBool("defaultPointerAccelerationProfileFlat");
    PropBool m_pointerAccelerationProfileFlat = PropBool("pointerAccelerationProfileFlat");

    PropBool m_supportsPointerAccelerationProfileAdaptive = PropBool("supportsPointerAccelerationProfileAdaptive");
    PropBool m_defaultPointerAccelerationProfileAdaptive = PropBool("defaultPointerAccelerationProfileAdaptive");
    PropBool m_pointerAccelerationProfileAdaptive = PropBool("pointerAccelerationProfileAdaptive");

    //
    // tapping
    Prop<int> m_tapFingerCount = PropInt("tapFingerCount");
    PropBool m_tapToClickEnabledByDefault = PropBool("tapToClickEnabledByDefault");
    PropBool m_tapToClick = PropBool("tapToClick");

    PropBool m_lmrTapButtonMapEnabledByDefault = PropBool("lmrTapButtonMapEnabledByDefault");
    PropBool m_lmrTapButtonMap = PropBool("lmrTapButtonMap");

    PropBool m_tapAndDragEnabledByDefault = PropBool("tapAndDragEnabledByDefault");
    PropBool m_tapAndDrag = PropBool("tapAndDrag");
    PropBool m_tapDragLockEnabledByDefault = PropBool("tapDragLockEnabledByDefault");
    PropBool m_tapDragLock = PropBool("tapDragLock");

    //
    // scrolling
    PropBool m_naturalScrollEnabledByDefault = PropBool("naturalScrollEnabledByDefault");
    PropBool m_naturalScroll = PropBool("naturalScroll");

    PropBool m_horizontalScrolling = PropBool("horizontalScrolling");

    PropBool m_supportsScrollTwoFinger = PropBool("supportsScrollTwoFinger");
    PropBool m_scrollTwoFingerEnabledByDefault = PropBool("scrollTwoFingerEnabledByDefault");
    PropBool m_isScrollTwoFinger = PropBool("scrollTwoFinger");

    PropBool m_supportsScrollEdge = PropBool("supportsScrollEdge");
    PropBool m_scrollEdgeEnabledByDefault = PropBool("scrollEdgeEnabledByDefault");
    PropBool m_isScrollEdge = PropBool("scrollEdge");

    PropBool m_supportsScrollOnButtonDown = PropBool("supportsScrollOnButtonDown");
    PropBool m_scrollOnButtonDownEnabledByDefault = PropBool("scrollOnButtonDownEnabledByDefault");
    PropBool m_isScrollOnButtonDown = PropBool("scrollOnButtonDown");

    Prop<quint32> m_defaultScrollButton = Prop<quint32>("defaultScrollButton", 0);
    Prop<quint32> m_scrollButton = Prop<quint32>("scrollButton", 0);

    // Click Method
    PropBool m_supportsClickMethodAreas = PropBool("supportsClickMethodAreas");
    PropBool m_defaultClickMethodAreas = PropBool("defaultClickMethodAreas");
    PropBool m_clickMethodAreas = PropBool("clickMethodAreas");

    PropBool m_supportsClickMethodClickfinger = PropBool("supportsClickMethodClickfinger");
    PropBool m_defaultClickMethodClickfinger = PropBool("defaultClickMethodClickfinger");
    PropBool m_clickMethodClickfinger = PropBool("clickMethodClickfinger");
};
