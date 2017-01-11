/*
 * Copyright 2017 Roman Gilg <subdiff@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KWINWAYLANDTOUCHPAD_H
#define KWINWAYLANDTOUCHPAD_H

#include <QObject>
#include <QString>

class QDBusInterface;

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
    Q_PROPERTY(bool pointerAccelerationProfileFlat READ pointerAccelerationProfileFlat WRITE setPointerAccelerationProfileFlat NOTIFY pointerAccelerationProfileChanged)

    Q_PROPERTY(bool supportsPointerAccelerationProfileAdaptive READ supportsPointerAccelerationProfileAdaptive CONSTANT)
    Q_PROPERTY(bool defaultPointerAccelerationProfileAdaptive READ defaultPointerAccelerationProfileAdaptive CONSTANT)
    Q_PROPERTY(bool pointerAccelerationProfileAdaptive READ pointerAccelerationProfileAdaptive WRITE setPointerAccelerationProfileAdaptive NOTIFY pointerAccelerationProfileChanged)

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

public:
    KWinWaylandTouchpad(QString dbusName);
    virtual ~KWinWaylandTouchpad();

    bool init();

    bool getConfig();
    bool getDefaultConfig();
    bool applyConfig();
    bool isChangedConfig() const;

    //
    // general
    QString name() const {
        return m_name.val;
    }
    QString sysName() const {
        return m_sysName.val;
    }
    bool supportsDisableEvents() const {
        return m_supportsDisableEvents.val;
    }
    void setEnabled(bool enabled) {
        m_enabled.set(enabled);
    }
    bool isEnabled() const {
        return m_enabled.val;
    }
    Qt::MouseButtons supportedButtons() const {
        return m_supportedButtons.val;
    }

    //
    // advanced
    bool supportsLeftHanded() const {
        return m_supportsLeftHanded.val;
    }
    bool leftHandedEnabledByDefault() const {
        return m_leftHandedEnabledByDefault.val;
    }
    bool isLeftHanded() const {
        return m_leftHanded.val;
    }
    void setLeftHanded(bool set) {
        m_leftHanded.set(set);
    }

    bool supportsDisableEventsOnExternalMouse() const {
        return m_supportsDisableEventsOnExternalMouse.val;
    }

    bool supportsDisableWhileTyping() const {
        return m_supportsDisableWhileTyping.val;
    }
    bool disableWhileTypingEnabledByDefault() const {
        return m_disableWhileTypingEnabledByDefault.val;
    }
    bool isDisableWhileTyping() const {
        return m_disableWhileTyping.val;
    }
    void setDisableWhileTyping(bool set) {
        m_disableWhileTyping.set(set);
    }

    bool supportsMiddleEmulation() const {
        return m_supportsMiddleEmulation.val;
    }
    bool middleEmulationEnabledByDefault() const {
        return m_middleEmulationEnabledByDefault.val;
    }
    bool isMiddleEmulation() const {
        return m_middleEmulation.val;
    }
    void setMiddleEmulation(bool set) {
        m_middleEmulation.set(set);
    }

    //
    // acceleration speed and profile
    bool supportsPointerAcceleration() const {
        return m_supportsPointerAcceleration.val;
    }
    qreal pointerAcceleration() const {
        return m_pointerAcceleration.val;
    }
    void setPointerAcceleration(qreal acceleration) {
        m_pointerAcceleration.set(acceleration);
    }

    bool supportsPointerAccelerationProfileFlat() const {
        return m_supportsPointerAccelerationProfileFlat.val;
    }
    bool defaultPointerAccelerationProfileFlat() const {
        return m_defaultPointerAccelerationProfileFlat.val;
    }
    bool pointerAccelerationProfileFlat() const {
        return m_pointerAccelerationProfileFlat.val;
    }
    void setPointerAccelerationProfileFlat(bool set) {
        m_pointerAccelerationProfileFlat.set(set);
    }

    bool supportsPointerAccelerationProfileAdaptive() const {
        return m_supportsPointerAccelerationProfileAdaptive.val;
    }
    bool defaultPointerAccelerationProfileAdaptive() const {
        return m_defaultPointerAccelerationProfileAdaptive.val;
    }
    bool pointerAccelerationProfileAdaptive() const {
        return m_pointerAccelerationProfileAdaptive.val;
    }
    void setPointerAccelerationProfileAdaptive(bool set) {
        m_pointerAccelerationProfileAdaptive.set(set);
    }

    //
    // tapping
    int tapFingerCount() const {
        return m_tapFingerCount.val;
    }
    bool tapToClickEnabledByDefault() const {
        return m_tapToClickEnabledByDefault.val;
    }
    bool isTapToClick() const {
        return m_tapToClick.val;
    }
    void setTapToClick(bool set) {
        m_tapToClick.set(set);
    }

    bool supportsLmrTapButtonMap() const {
        return m_tapFingerCount.val > 1;
    }
    bool lmrTapButtonMapEnabledByDefault() const {
        return m_lmrTapButtonMapEnabledByDefault.val;
    }
    bool lmrTapButtonMap() const {
        return m_lmrTapButtonMap.val;
    }
    void setLmrTapButtonMap(bool set) {
        m_lmrTapButtonMap.set(set);
    }

    bool tapAndDragEnabledByDefault() const {
        return m_tapAndDragEnabledByDefault.val;
    }
    bool isTapAndDrag() const {
        return m_tapAndDrag.val;
    }
    void setTapAndDrag(bool set) {
        m_tapAndDrag.set(set);
    }

    bool tapDragLockEnabledByDefault() const {
        return m_tapDragLockEnabledByDefault.val;
    }
    bool isTapDragLock() const {
        return m_tapDragLock.val;
    }
    void setTapDragLock(bool set) {
        m_tapDragLock.set(set);
    }

    //
    // scrolling
    bool supportsNaturalScroll() const {
        return m_supportsNaturalScroll.val;
    }
    bool naturalScrollEnabledByDefault() const {
        return m_naturalScrollEnabledByDefault.val;
    }
    bool isNaturalScroll() const {
        return m_naturalScroll.val;
    }
    void setNaturalScroll(bool set) {
        m_naturalScroll.set(set);
    }

    bool supportsScrollTwoFinger() const {
        return m_supportsScrollTwoFinger.val;
    }
    bool scrollTwoFingerEnabledByDefault() const {
        return m_scrollTwoFingerEnabledByDefault.val;
    }
    bool isScrollTwoFinger() const {
        return m_isScrollTwoFinger.val;
    }
    void setScrollTwoFinger(bool set) {
        m_isScrollTwoFinger.set(set);
    }

    bool supportsScrollEdge() const {
        return m_supportsScrollEdge.val;
    }
    bool scrollEdgeEnabledByDefault() const {
        return m_scrollEdgeEnabledByDefault.val;
    }
    bool isScrollEdge() const {
        return m_isScrollEdge.val;
    }
    void setScrollEdge(bool set) {
        m_isScrollEdge.set(set);
    }

    bool supportsScrollOnButtonDown() const {
        return m_supportsScrollOnButtonDown.val;
    }
    bool scrollOnButtonDownEnabledByDefault() const {
        return m_scrollOnButtonDownEnabledByDefault.val;
    }
    bool isScrollOnButtonDown() const {
        return m_isScrollOnButtonDown.val;
    }
    void setScrollOnButtonDown(bool set) {
        m_isScrollOnButtonDown.set(set);
    }

    quint32 defaultScrollButton() const {
        return m_defaultScrollButton.val;
    }
    quint32 scrollButton() const {
        return m_scrollButton.val;
    }
    void setScrollButton(quint32 button) {
        m_scrollButton.set(button);
    }

Q_SIGNALS:
    void leftHandedChanged();
    void pointerAccelerationChanged();
    void pointerAccelerationProfileChanged();
    void enabledChanged();
    void tapToClickChanged();
    void tapAndDragChanged();
    void tapDragLockChanged();
    void lmrTapButtonMapChanged();
    void disableWhileTypingChanged();
    void middleEmulationChanged();
    void naturalScrollChanged();
    void scrollMethodChanged();
    void scrollButtonChanged();

private:
    template <typename T>
    struct Prop {
        explicit Prop(const QByteArray &dbusName)
            : dbus(dbusName)
        {}

        void set(T newVal) {
            if (avail && val != newVal) {
                val = newVal;
            }
        }
        void set(const Prop<T> &p) {
            if (avail && val != p.val) {
                val = p.val;
            }
        }
        bool changed() const {
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
    Prop<Qt::MouseButtons> m_supportedButtons  = Prop<Qt::MouseButtons>("supportedButtons");

    Prop<bool> m_supportsLeftHanded = Prop<bool>("supportsLeftHanded");
    Prop<bool> m_leftHandedEnabledByDefault = Prop<bool>("leftHandedEnabledByDefault");
    Prop<bool> m_leftHanded = Prop<bool>("leftHanded");

    Prop<bool> m_supportsDisableEventsOnExternalMouse = Prop<bool>("supportsDisableEventsOnExternalMouse");

    Prop<bool> m_supportsDisableWhileTyping = Prop<bool>("supportsDisableWhileTyping");
    Prop<bool> m_disableWhileTypingEnabledByDefault = Prop<bool>("disableWhileTypingEnabledByDefault");
    Prop<bool> m_disableWhileTyping = Prop<bool>("disableWhileTyping");

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
    // tapping
    Prop<int> m_tapFingerCount = Prop<int>("tapFingerCount");
    Prop<bool> m_tapToClickEnabledByDefault = Prop<bool>("tapToClickEnabledByDefault");
    Prop<bool> m_tapToClick = Prop<bool>("tapToClick");

    Prop<bool> m_lmrTapButtonMapEnabledByDefault = Prop<bool>("lmrTapButtonMapEnabledByDefault");
    Prop<bool> m_lmrTapButtonMap = Prop<bool>("lmrTapButtonMap");

    Prop<bool> m_tapAndDragEnabledByDefault = Prop<bool>("tapAndDragEnabledByDefault");
    Prop<bool> m_tapAndDrag = Prop<bool>("tapAndDrag");
    Prop<bool> m_tapDragLockEnabledByDefault = Prop<bool>("tapDragLockEnabledByDefault");
    Prop<bool> m_tapDragLock = Prop<bool>("tapDragLock");

    //
    // scrolling
    Prop<bool> m_supportsNaturalScroll = Prop<bool>("supportsNaturalScroll");
    Prop<bool> m_naturalScrollEnabledByDefault = Prop<bool>("naturalScrollEnabledByDefault");
    Prop<bool> m_naturalScroll = Prop<bool>("naturalScroll");

    Prop<bool> m_supportsScrollTwoFinger = Prop<bool>("supportsScrollTwoFinger");
    Prop<bool> m_scrollTwoFingerEnabledByDefault = Prop<bool>("scrollTwoFingerEnabledByDefault");
    Prop<bool> m_isScrollTwoFinger = Prop<bool>("scrollTwoFinger");

    Prop<bool> m_supportsScrollEdge = Prop<bool>("supportsScrollEdge");
    Prop<bool> m_scrollEdgeEnabledByDefault = Prop<bool>("scrollEdgeEnabledByDefault");
    Prop<bool> m_isScrollEdge = Prop<bool>("scrollEdge");

    Prop<bool> m_supportsScrollOnButtonDown = Prop<bool>("supportsScrollOnButtonDown");
    Prop<bool> m_scrollOnButtonDownEnabledByDefault = Prop<bool>("scrollOnButtonDownEnabledByDefault");
    Prop<bool> m_isScrollOnButtonDown = Prop<bool>("scrollOnButtonDown");

    Prop<quint32> m_defaultScrollButton = Prop<quint32>("defaultScrollButton");
    Prop<quint32> m_scrollButton = Prop<quint32>("scrollButton");


    QDBusInterface *m_iface;
};

#endif
