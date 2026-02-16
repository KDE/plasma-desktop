/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwinwaylandtouchpad.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusReply>
#include <QList>

#include "logging.h"

KWinWaylandTouchpad::KWinWaylandTouchpad(const QString &dbusName)
    : LibinputCommon()
    , m_dbusName(dbusName)
{
}

KWinWaylandTouchpad::~KWinWaylandTouchpad()
{
}

bool KWinWaylandTouchpad::load()
{
    auto message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                  QStringLiteral("/org/kde/KWin/InputDevice/") + m_dbusName,
                                                  QStringLiteral("org.freedesktop.DBus.Properties"),
                                                  QStringLiteral("GetAll"));
    message << QStringLiteral("org.kde.KWin.InputDevice");
    QDBusReply<QVariantMap> reply = QDBusConnection::sessionBus().call(message);

    if (!reply.isValid()) {
        return false;
    }

    bool success = true;

    const auto properties = reply.value();

    const auto valueLoader = [this, &properties]<typename T>(Prop<T> &prop) {
        return this->valueLoader(properties, prop);
    };

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
    success &= valueLoader(m_disableEventsOnExternalMouseEnabledByDefault);
    success &= valueLoader(m_disableWhileTypingEnabledByDefault);
    success &= valueLoader(m_leftHandedEnabledByDefault);
    success &= valueLoader(m_pointerAcceleration);
    success &= valueLoader(m_pointerAccelerationProfileFlat);
    success &= valueLoader(m_pointerAccelerationProfileAdaptive);
    success &= valueLoader(m_disableEventsOnExternalMouse);
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

bool KWinWaylandTouchpad::defaults()
{
    m_enabled.set(true);
    m_leftHanded.set(false);

    m_pointerAcceleration.set(m_defaultPointerAcceleration);
    m_pointerAccelerationProfileFlat.set(m_defaultPointerAccelerationProfileFlat);
    m_pointerAccelerationProfileAdaptive.set(m_defaultPointerAccelerationProfileAdaptive);

    m_disableEventsOnExternalMouse.set(m_disableEventsOnExternalMouseEnabledByDefault);
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

bool KWinWaylandTouchpad::save()
{
    bool success = true;

    success &= valueWriter(m_enabled);
    success &= valueWriter(m_leftHanded);
    success &= valueWriter(m_pointerAcceleration);
    success &= valueWriter(m_pointerAccelerationProfileFlat);
    success &= valueWriter(m_pointerAccelerationProfileAdaptive);
    success &= valueWriter(m_disableEventsOnExternalMouse);
    success &= valueWriter(m_disableWhileTyping);
    success &= valueWriter(m_middleEmulation);

    success &= valueWriter(m_tapToClick);
    success &= valueWriter(m_tapAndDrag);
    success &= valueWriter(m_tapDragLock);
    success &= valueWriter(m_lmrTapButtonMap);
    success &= valueWriter(m_naturalScroll);
    success &= valueWriter(m_isScrollTwoFinger);
    success &= valueWriter(m_isScrollEdge);
    success &= valueWriter(m_isScrollOnButtonDown);
    success &= valueWriter(m_scrollButton);
    success &= valueWriter(m_scrollFactor);
    success &= valueWriter(m_clickMethodAreas);
    success &= valueWriter(m_clickMethodClickfinger);

    return success;
}

bool KWinWaylandTouchpad::isSaveNeeded() const
{
    // clang-format off
    return m_enabled.changed() ||
            m_leftHanded.changed() ||
            m_pointerAcceleration.changed() ||
            m_pointerAccelerationProfileFlat.changed() ||
            m_pointerAccelerationProfileAdaptive.changed() ||
            m_disableEventsOnExternalMouse.changed() ||
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
bool KWinWaylandTouchpad::valueWriter(const Prop<T> &prop)
{
    if (!prop.changed()) {
        return true;
    }
    auto message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                  QStringLiteral("/org/kde/KWin/InputDevice/") + m_dbusName,
                                                  QStringLiteral("org.freedesktop.DBus.Properties"),
                                                  QStringLiteral("Set"));
    message << QStringLiteral("org.kde.KWin.InputDevice") << prop.name << QVariant::fromValue(QDBusVariant(prop.val));
    QDBusReply<void> reply = QDBusConnection::sessionBus().call(message);
    if (reply.error().isValid()) {
        qCCritical(KCM_TOUCHPAD) << reply.error().message();
        return false;
    }
    return true;
}

template<typename T>
bool KWinWaylandTouchpad::valueLoader(const QVariantMap &properties, Prop<T> &prop)
{
    if (const auto variant = properties.value(prop.name); variant.isValid()) {
        prop.avail = true;
        prop.old = valueLoaderPart<T>(variant);
        prop.set(prop.old);
        return true;
    } else {
        qCCritical(KCM_TOUCHPAD) << "Device" << m_dbusName << "does not have property on d-bus read of" << prop.name;
        prop.avail = false;
        return false;
    }
}

#include "moc_kwinwaylandtouchpad.cpp"
