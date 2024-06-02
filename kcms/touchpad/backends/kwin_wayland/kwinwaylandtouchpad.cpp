/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwinwaylandtouchpad.h"

#include <QDBusError>
#include <QDBusInterface>
#include <QList>

#include "logging.h"

KWinWaylandTouchpad::KWinWaylandTouchpad(QString dbusName)
    : LibinputCommon()
{
    m_iface = new QDBusInterface(QStringLiteral("org.kde.KWin"),
                                 QStringLiteral("/org/kde/KWin/InputDevice/") + dbusName,
                                 QStringLiteral("org.kde.KWin.InputDevice"),
                                 QDBusConnection::sessionBus(),
                                 this);
}

KWinWaylandTouchpad::~KWinWaylandTouchpad()
{
    delete m_iface;
}

bool KWinWaylandTouchpad::init()
{
    // need to do it here in order to populate combobox and handle events
    return valueLoader(m_name) && valueLoader(m_sysName);
}

bool KWinWaylandTouchpad::getConfig()
{
    bool success = true;

    // general
    success &= valueLoader(m_supportsDisableEvents);
    success &= valueLoader(m_supportsLeftHanded);
    success &= valueLoader(m_supportedButtons);
    success &= valueLoader(m_leftHandedEnabledByDefault);
    success &= valueLoader(m_enabled);
    success &= valueLoader(m_leftHanded);
    // advanced
    success &= valueLoader(m_supportsPointerAcceleration);
    success &= valueLoader(m_supportsPointerAccelerationProfileFlat);
    success &= valueLoader(m_supportsPointerAccelerationProfileAdaptive);
    success &= valueLoader(m_supportsDisableWhileTyping);
    success &= valueLoader(m_supportsDisableEventsOnExternalMouse);
    success &= valueLoader(m_defaultPointerAcceleration);
    success &= valueLoader(m_defaultPointerAccelerationProfileFlat);
    success &= valueLoader(m_defaultPointerAccelerationProfileAdaptive);
    success &= valueLoader(m_disableWhileTypingEnabledByDefault);
    success &= valueLoader(m_leftHandedEnabledByDefault);
    success &= valueLoader(m_pointerAcceleration);
    success &= valueLoader(m_pointerAccelerationProfileFlat);
    success &= valueLoader(m_pointerAccelerationProfileAdaptive);
    success &= valueLoader(m_disableWhileTyping);
    // tapping
    success &= valueLoader(m_tapFingerCount);
    success &= valueLoader(m_supportsMiddleEmulation);
    success &= valueLoader(m_tapToClickEnabledByDefault);
    success &= valueLoader(m_tapAndDragEnabledByDefault);
    success &= valueLoader(m_tapDragLockEnabledByDefault);
    success &= valueLoader(m_middleEmulationEnabledByDefault);
    success &= valueLoader(m_tapToClick);
    success &= valueLoader(m_tapAndDrag);
    success &= valueLoader(m_tapDragLock);
    success &= valueLoader(m_middleEmulation);
    success &= valueLoader(m_lmrTapButtonMapEnabledByDefault);
    success &= valueLoader(m_lmrTapButtonMap);
    // scrolling modes avail
    success &= valueLoader(m_supportsNaturalScroll);
    success &= valueLoader(m_supportsScrollTwoFinger);
    success &= valueLoader(m_supportsScrollEdge);
    success &= valueLoader(m_supportsScrollOnButtonDown);
    // default scrolling modes
    success &= valueLoader(m_naturalScrollEnabledByDefault);
    success &= valueLoader(m_scrollTwoFingerEnabledByDefault);
    success &= valueLoader(m_scrollEdgeEnabledByDefault);
    success &= valueLoader(m_scrollOnButtonDownEnabledByDefault);
    success &= valueLoader(m_defaultScrollButton);
    // current scrolling mode
    success &= valueLoader(m_naturalScroll);
    success &= valueLoader(m_isScrollTwoFinger);
    success &= valueLoader(m_isScrollEdge);
    success &= valueLoader(m_isScrollOnButtonDown);
    success &= valueLoader(m_scrollButton);
    // scroll speed
    success &= valueLoader(m_scrollFactor);
    // click methods
    success &= valueLoader(m_supportsClickMethodAreas);
    success &= valueLoader(m_supportsClickMethodClickfinger);
    success &= valueLoader(m_defaultClickMethodAreas);
    success &= valueLoader(m_defaultClickMethodClickfinger);
    success &= valueLoader(m_clickMethodAreas);
    success &= valueLoader(m_clickMethodClickfinger);

    return success;
}

bool KWinWaylandTouchpad::getDefaultConfig()
{
    m_enabled.set(true);
    m_leftHanded.set(false);

    m_pointerAcceleration.set(m_defaultPointerAcceleration);
    m_pointerAccelerationProfileFlat.set(m_defaultPointerAccelerationProfileFlat);
    m_pointerAccelerationProfileAdaptive.set(m_defaultPointerAccelerationProfileAdaptive);

    m_disableWhileTyping.set(m_disableWhileTypingEnabledByDefault);

    m_tapToClick.set(m_tapToClickEnabledByDefault);
    m_tapAndDrag.set(m_tapAndDragEnabledByDefault);
    m_tapDragLock.set(m_tapDragLockEnabledByDefault);
    m_middleEmulation.set(m_middleEmulationEnabledByDefault);

    m_naturalScroll.set(m_naturalScrollEnabledByDefault);
    m_isScrollTwoFinger.set(m_scrollTwoFingerEnabledByDefault);
    m_isScrollEdge.set(m_scrollEdgeEnabledByDefault);
    m_isScrollOnButtonDown.set(m_scrollOnButtonDownEnabledByDefault);

    m_clickMethodAreas.set(m_defaultClickMethodAreas);
    m_clickMethodClickfinger.set(m_defaultClickMethodClickfinger);
    m_scrollFactor.set(1.0);

    return true;
}

bool KWinWaylandTouchpad::applyConfig()
{
    QList<QString> msgs;

    msgs << valueWriter(m_enabled) << valueWriter(m_leftHanded) << valueWriter(m_pointerAcceleration) << valueWriter(m_pointerAccelerationProfileFlat)
         << valueWriter(m_pointerAccelerationProfileAdaptive)

         << valueWriter(m_disableWhileTyping) << valueWriter(m_middleEmulation)

         << valueWriter(m_tapToClick) << valueWriter(m_tapAndDrag) << valueWriter(m_tapDragLock) << valueWriter(m_lmrTapButtonMap)

         << valueWriter(m_naturalScroll) << valueWriter(m_isScrollTwoFinger) << valueWriter(m_isScrollEdge) << valueWriter(m_isScrollOnButtonDown)
         << valueWriter(m_scrollButton) << valueWriter(m_scrollFactor)

         << valueWriter(m_clickMethodAreas) << valueWriter(m_clickMethodClickfinger);

    bool success = true;
    QString error_msg;

    for (const QString &m : std::as_const(msgs)) {
        if (!m.isNull()) {
            qCCritical(KCM_TOUCHPAD) << "in error:" << m;
            if (!success) {
                error_msg.append("\n");
            }
            error_msg.append(m);
            success = false;
        }
    }

    if (!success) {
        qCCritical(KCM_TOUCHPAD) << error_msg;
    }
    return success;
}

bool KWinWaylandTouchpad::isChangedConfig() const
{
    // clang-format off
    return m_enabled.changed() ||
            m_leftHanded.changed() ||
            m_pointerAcceleration.changed() ||
            m_pointerAccelerationProfileFlat.changed() ||
            m_pointerAccelerationProfileAdaptive.changed() ||
            m_disableWhileTyping.changed() ||
            m_middleEmulation.changed() ||
            m_tapToClick.changed() ||
            m_tapAndDrag.changed() ||
            m_tapDragLock.changed() ||
            m_lmrTapButtonMap.changed() ||
            m_naturalScroll.changed() ||
            m_isScrollTwoFinger.changed() ||
            m_isScrollEdge.changed() ||
            m_isScrollOnButtonDown.changed() ||
            m_scrollFactor.changed() ||
            m_scrollButton.changed() ||
            m_clickMethodAreas.changed() ||
            m_clickMethodClickfinger.changed();
    // clang-format on
}

template<typename T>
QString KWinWaylandTouchpad::valueWriter(const Prop<T> &prop)
{
    if (!prop.changed()) {
        return QString();
    }
    m_iface->setProperty(prop.name, prop.val);
    QDBusError error = m_iface->lastError();
    if (error.isValid()) {
        qCCritical(KCM_TOUCHPAD) << error.message();
        return error.message();
    }
    return QString();
}

template<typename T>
bool KWinWaylandTouchpad::valueLoader(Prop<T> &prop)
{
    QVariant reply = m_iface->property(prop.name);
    if (!reply.isValid()) {
        qCCritical(KCM_TOUCHPAD) << "Error on d-bus read of" << prop.name;
        prop.avail = false;
        return false;
    }
    prop.avail = true;

    T replyValue = valueLoaderPart<T>(reply);

    prop.old = replyValue;
    prop.val = replyValue;
    return true;
}

#include "moc_kwinwaylandtouchpad.cpp"
