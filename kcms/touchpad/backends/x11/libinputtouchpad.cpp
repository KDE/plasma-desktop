/*
    SPDX-FileCopyrightText: 2019 Atul Bisht <atulbisht26@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "libinputtouchpad.h"
#include "logging.h"

#include <QSet>

#include <limits.h>
#include <stddef.h>

#include <libinput-properties.h>
#include <xserver-properties.h>

#include <X11/extensions/XInput2.h>

const Parameter libinputProperties[] = {

    /* libinput disable supports property */
    {"supportsDisableEvents", PT_INT, 0, 1, LIBINPUT_PROP_SENDEVENTS_AVAILABLE, 8, 0},
    {"enabled", PT_INT, 0, 1, LIBINPUT_PROP_SENDEVENTS_ENABLED, 8, 0},
    {"enabledDefault", PT_INT, 0, 1, LIBINPUT_PROP_SENDEVENTS_ENABLED_DEFAULT, 8, 0},

    /* LeftHandSupport */
    {"leftHandedEnabledByDefault", PT_INT, 0, 1, LIBINPUT_PROP_LEFT_HANDED_DEFAULT, 8, 0},
    {"leftHanded", PT_INT, 0, 1, LIBINPUT_PROP_LEFT_HANDED, 8, 0},

    /* Disable on external mouse */
    {"supportsDisableEventsOnExternalMouse", PT_INT, 0, 1, LIBINPUT_PROP_SENDEVENTS_AVAILABLE, 8, 1},
    {"disableEventsOnExternalMouse", PT_INT, 0, 1, LIBINPUT_PROP_SENDEVENTS_ENABLED, 8, 1},
    {"disableEventsOnExternalMouseDefault", PT_INT, 0, 1, LIBINPUT_PROP_SENDEVENTS_ENABLED_DEFAULT, 8, 1},

    /* Disable while typing */
    {"disableWhileTypingEnabledByDefault", PT_INT, 0, 1, LIBINPUT_PROP_DISABLE_WHILE_TYPING_DEFAULT, 8, 0},
    {"disableWhileTyping", PT_INT, 0, 1, LIBINPUT_PROP_DISABLE_WHILE_TYPING, 8, 0},

    /* Middle Emulation */
    {"middleEmulationEnabledByDefault", PT_INT, 0, 1, LIBINPUT_PROP_MIDDLE_EMULATION_ENABLED_DEFAULT, 8, 0},
    {"middleEmulation", PT_INT, 0, 1, LIBINPUT_PROP_MIDDLE_EMULATION_ENABLED, 8, 0},

    /* This is a boolean for all three fingers, no per-finger config */
    {"tapToClick", PT_INT, 0, 1, LIBINPUT_PROP_TAP, 8, 0},
    {"tapToClickEnabledByDefault", PT_INT, 0, 1, LIBINPUT_PROP_TAP_DEFAULT, 8, 0},

    /* LMR 1/2/3-finger tapping mapping to Left/right/middle or left/middle/right */
    {"lrmTapButtonMapEnabledByDefault", PT_INT, 0, 1, LIBINPUT_PROP_TAP_BUTTONMAP_DEFAULT, 8, 0},
    {"lrmTapButtonMap", PT_INT, 0, 1, LIBINPUT_PROP_TAP_BUTTONMAP, 8, 0},
    {"lmrTapButtonMapEnabledByDefault", PT_INT, 0, 1, LIBINPUT_PROP_TAP_BUTTONMAP_DEFAULT, 8, 1},
    {"lmrTapButtonMap", PT_INT, 0, 1, LIBINPUT_PROP_TAP_BUTTONMAP, 8, 1},

    /* Tap and Drag Enabled */
    {"tapAndDragEnabledByDefault", PT_INT, 0, 1, LIBINPUT_PROP_TAP_DRAG_DEFAULT, 8, 0},
    {"tapAndDrag", PT_INT, 0, 1, LIBINPUT_PROP_TAP_DRAG, 8, 0},

    /* Tap and Drag Lock Enabled */
    {"tapDragLockEnabledByDefault", PT_INT, 0, 1, LIBINPUT_PROP_TAP_DRAG_LOCK_DEFAULT, 8, 0},
    {"tapDragLock", PT_INT, 0, 1, LIBINPUT_PROP_TAP_DRAG_LOCK, 8, 0},

    /* libinput normalizes the accel to -1/1 */
    {"defaultPointerAcceleration", PT_DOUBLE, -1.0, 1.0, LIBINPUT_PROP_ACCEL_DEFAULT, 0 /*float */, 0},
    {"pointerAcceleration", PT_DOUBLE, -1.0, 1.0, LIBINPUT_PROP_ACCEL, 0 /*float */, 0},

    /* Libinput Accel Profile */
    {"supportsPointerAccelerationProfileAdaptive", PT_BOOL, 0, 1, LIBINPUT_PROP_ACCEL_PROFILES_AVAILABLE, 8, 0},
    {"defaultPointerAccelerationProfileAdaptive", PT_BOOL, 0, 1, LIBINPUT_PROP_ACCEL_PROFILE_ENABLED_DEFAULT, 8, 0},
    {"pointerAccelerationProfileAdaptive", PT_BOOL, 0, 1, LIBINPUT_PROP_ACCEL_PROFILE_ENABLED, 8, 0},
    {"supportsPointerAccelerationProfileFlat", PT_BOOL, 0, 1, LIBINPUT_PROP_ACCEL_PROFILES_AVAILABLE, 8, 1},
    {"defaultPointerAccelerationProfileFlat", PT_BOOL, 0, 1, LIBINPUT_PROP_ACCEL_PROFILE_ENABLED_DEFAULT, 8, 1},
    {"pointerAccelerationProfileFlat", PT_BOOL, 0, 1, LIBINPUT_PROP_ACCEL_PROFILE_ENABLED, 8, 1},

    /* Natural Scrolling */
    {"naturalScrollEnabledByDefault", PT_INT, 0, 1, LIBINPUT_PROP_NATURAL_SCROLL_DEFAULT, 8, 0},
    {"naturalScroll", PT_INT, 0, 1, LIBINPUT_PROP_NATURAL_SCROLL, 8, 0},

    /* Horizontal scrolling */
    {"horizontalScrolling", PT_INT, 0, 1, LIBINPUT_PROP_HORIZ_SCROLL_ENABLED, 8, 0},

    /* Two-Finger Scrolling */
    {"supportsScrollTwoFinger", PT_INT, 0, 1, LIBINPUT_PROP_SCROLL_METHODS_AVAILABLE, 8, 0},
    {"scrollTwoFingerEnabledByDefault", PT_INT, 0, 1, LIBINPUT_PROP_SCROLL_METHOD_ENABLED_DEFAULT, 8, 0},
    {"scrollTwoFinger", PT_INT, 0, 1, LIBINPUT_PROP_SCROLL_METHOD_ENABLED, 8, 0},

    /* Edge Scrolling */
    {"supportsScrollEdge", PT_INT, 0, 1, LIBINPUT_PROP_SCROLL_METHODS_AVAILABLE, 8, 1},
    {"scrollEdgeEnabledByDefault", PT_INT, 0, 1, LIBINPUT_PROP_SCROLL_METHOD_ENABLED_DEFAULT, 8, 1},
    {"scrollEdge", PT_INT, 0, 1, LIBINPUT_PROP_SCROLL_METHOD_ENABLED, 8, 1},

    /* scroll on button */
    {"supportsScrollOnButtonDown", PT_INT, 0, 1, LIBINPUT_PROP_SCROLL_METHODS_AVAILABLE, 8, 2},
    {"scrollOnButtonDownEnabledByDefault", PT_INT, 0, 1, LIBINPUT_PROP_SCROLL_METHOD_ENABLED_DEFAULT, 8, 2},
    {"scrollOnButtonDown", PT_INT, 0, 1, LIBINPUT_PROP_SCROLL_METHOD_ENABLED, 8, 2},

    /* Scroll Button for scroll on button Down */
    {"defaultScrollButton", PT_INT, 0, INT_MAX, LIBINPUT_PROP_SCROLL_BUTTON_DEFAULT, 32, 0},
    {"scrollButton", PT_INT, 0, INT_MAX, LIBINPUT_PROP_SCROLL_BUTTON, 32, 0},

    /* Click Methods */
    {"supportsClickMethodAreas", PT_INT, 0, 1, LIBINPUT_PROP_CLICK_METHODS_AVAILABLE, 8, 0},
    {"defaultClickMethodAreas", PT_INT, 0, 1, LIBINPUT_PROP_CLICK_METHOD_ENABLED_DEFAULT, 8, 0},
    {"clickMethodAreas", PT_INT, 0, 1, LIBINPUT_PROP_CLICK_METHOD_ENABLED, 8, 0},

    {"supportsClickMethodClickfinger", PT_INT, 0, 1, LIBINPUT_PROP_CLICK_METHODS_AVAILABLE, 8, 1},
    {"defaultClickMethodClickfinger", PT_INT, 0, 1, LIBINPUT_PROP_CLICK_METHOD_ENABLED_DEFAULT, 8, 1},
    {"clickMethodClickfinger", PT_INT, 0, 1, LIBINPUT_PROP_CLICK_METHOD_ENABLED, 8, 1},

    /* libinput doesn't have a separate toggle for horiz scrolling */
    {nullptr, PT_INT, 0, 0, nullptr, 0, 0}};

Qt::MouseButtons maskBtns(Display *display, XIButtonClassInfo *buttonInfo)
{
    Qt::MouseButtons buttons = Qt::NoButton;
    for (int i = 0; i < buttonInfo->num_buttons; ++i) {
        QByteArray reply = XGetAtomName(display, buttonInfo->labels[i]);

        if (reply == BTN_LABEL_PROP_BTN_LEFT) {
            buttons |= Qt::LeftButton;
        }
        if (reply == BTN_LABEL_PROP_BTN_RIGHT) {
            buttons |= Qt::RightButton;
        }
        if (reply == BTN_LABEL_PROP_BTN_MIDDLE) {
            buttons |= Qt::MiddleButton;
        }
        if (reply == BTN_LABEL_PROP_BTN_SIDE) {
            buttons |= Qt::ExtraButton1;
        }
        if (reply == BTN_LABEL_PROP_BTN_EXTRA) {
            buttons |= Qt::ExtraButton2;
        }
        if (reply == BTN_LABEL_PROP_BTN_FORWARD) {
            buttons |= Qt::ForwardButton;
        }
        if (reply == BTN_LABEL_PROP_BTN_BACK) {
            buttons |= Qt::BackButton;
        }
        if (reply == BTN_LABEL_PROP_BTN_TASK) {
            buttons |= Qt::TaskButton;
        }
    }
    return buttons;
}

LibinputTouchpad::LibinputTouchpad(Display *display, int deviceId)
    : LibinputCommon()
    , XlibTouchpad(display, deviceId)
{
    loadSupportedProperties(libinputProperties);

    int nDevices = 0;
    XIDeviceInfo *deviceInfo = XIQueryDevice(m_display, m_deviceId, &nDevices);
    m_name = deviceInfo->name;

    for (int i = 0; i < deviceInfo->num_classes; ++i) {
        XIAnyClassInfo *classInfo = deviceInfo->classes[i];

        if (classInfo->type == XIButtonClass) {
            XIButtonClassInfo *btnInfo = (XIButtonClassInfo *)classInfo;
            m_supportedButtons.avail = true;
            m_supportedButtons.set(maskBtns(m_display, btnInfo));
        }
        if (classInfo->type == XITouchClass) {
            XITouchClassInfo *touchInfo = (XITouchClassInfo *)classInfo;
            m_tapFingerCount.avail = true;
            m_tapFingerCount.set(touchInfo->num_touches);
        }
    }
    XIFreeDeviceInfo(deviceInfo);

    /* FingerCount cannot be zero */
    if (!m_tapFingerCount.val) {
        m_tapFingerCount.avail = true;

        // when lmr or rml are enabled by default, fingercount must be at least 3
        if ((getParameter(findParameter(m_lmrTapButtonMapEnabledByDefault.name)).toBool()
             || getParameter(findParameter(m_lrmTapButtonMapEnabledByDefault.name)).toBool())) {
            m_tapFingerCount.set(3);
        } else {
            m_tapFingerCount.set(1);
        }
    }
    m_config = KSharedConfig::openConfig(QStringLiteral("touchpadxlibinputrc"));
}

bool LibinputTouchpad::getConfig()
{
    bool success = true;

    success &= valueLoader(m_supportsDisableEvents);
    success &= valueLoader(m_enabled);
    success &= valueLoader(m_enabledDefault);

    success &= valueLoader(m_tapToClickEnabledByDefault);
    success &= valueLoader(m_tapToClick);
    success &= valueLoader(m_lrmTapButtonMapEnabledByDefault);
    success &= valueLoader(m_lrmTapButtonMap);
    success &= valueLoader(m_lmrTapButtonMapEnabledByDefault);
    success &= valueLoader(m_lmrTapButtonMap);
    success &= valueLoader(m_tapAndDragEnabledByDefault);
    success &= valueLoader(m_tapAndDrag);
    success &= valueLoader(m_tapDragLockEnabledByDefault);
    success &= valueLoader(m_tapDragLock);

    success &= valueLoader(m_leftHandedEnabledByDefault);
    success &= valueLoader(m_leftHanded);

    success &= valueLoader(m_supportsDisableEventsOnExternalMouse);
    success &= valueLoader(m_disableEventsOnExternalMouse);
    success &= valueLoader(m_disableEventsOnExternalMouseDefault);

    success &= valueLoader(m_disableWhileTypingEnabledByDefault);
    success &= valueLoader(m_disableWhileTyping);

    success &= valueLoader(m_middleEmulationEnabledByDefault);
    success &= valueLoader(m_middleEmulation);

    success &= valueLoader(m_defaultPointerAcceleration);
    success &= valueLoader(m_pointerAcceleration);

    success &= valueLoader(m_supportsPointerAccelerationProfileFlat);
    success &= valueLoader(m_defaultPointerAccelerationProfileFlat);
    success &= valueLoader(m_pointerAccelerationProfileFlat);
    success &= valueLoader(m_supportsPointerAccelerationProfileAdaptive);
    success &= valueLoader(m_defaultPointerAccelerationProfileAdaptive);
    success &= valueLoader(m_pointerAccelerationProfileAdaptive);

    success &= valueLoader(m_naturalScrollEnabledByDefault);
    success &= valueLoader(m_naturalScroll);

    success &= valueLoader(m_horizontalScrolling);

    success &= valueLoader(m_supportsScrollTwoFinger);
    success &= valueLoader(m_scrollTwoFingerEnabledByDefault);
    success &= valueLoader(m_isScrollTwoFinger);

    success &= valueLoader(m_supportsScrollEdge);
    success &= valueLoader(m_scrollEdgeEnabledByDefault);
    success &= valueLoader(m_isScrollEdge);

    success &= valueLoader(m_supportsScrollOnButtonDown);
    success &= valueLoader(m_scrollOnButtonDownEnabledByDefault);
    success &= valueLoader(m_isScrollOnButtonDown);

    success &= valueLoader(m_defaultScrollButton);
    success &= valueLoader(m_scrollButton);

    // click methods
    success &= valueLoader(m_supportsClickMethodAreas);
    success &= valueLoader(m_supportsClickMethodClickfinger);
    success &= valueLoader(m_defaultClickMethodAreas);
    success &= valueLoader(m_defaultClickMethodClickfinger);
    success &= valueLoader(m_clickMethodAreas);
    success &= valueLoader(m_clickMethodClickfinger);

    return success;
}

bool LibinputTouchpad::applyConfig()
{
    QList<QString> msgs;

    msgs << valueWriter(m_enabled) << valueWriter(m_tapToClick) << valueWriter(m_lrmTapButtonMap) << valueWriter(m_lmrTapButtonMap) << valueWriter(m_tapAndDrag)
         << valueWriter(m_tapDragLock) << valueWriter(m_leftHanded) << valueWriter(m_disableWhileTyping) << valueWriter(m_middleEmulation)
         << valueWriter(m_pointerAcceleration) << valueWriter(m_pointerAccelerationProfileFlat) << valueWriter(m_pointerAccelerationProfileAdaptive)
         << valueWriter(m_naturalScroll) << valueWriter(m_horizontalScrolling) << valueWriter(m_isScrollTwoFinger) << valueWriter(m_isScrollEdge)
         << valueWriter(m_isScrollOnButtonDown) << valueWriter(m_scrollButton) << valueWriter(m_clickMethodAreas) << valueWriter(m_clickMethodClickfinger);

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

    flush();
    return success;
}

bool LibinputTouchpad::getDefaultConfig()
{
    m_enabled.set(m_enabledDefault);
    m_tapToClick.set(m_tapToClickEnabledByDefault);
    m_lrmTapButtonMap.set(m_lrmTapButtonMap);
    m_lmrTapButtonMap.set(m_lmrTapButtonMapEnabledByDefault);
    m_tapAndDrag.set(m_tapAndDragEnabledByDefault);
    m_tapDragLock.set(m_tapDragLockEnabledByDefault);
    m_leftHanded.set(m_leftHandedEnabledByDefault);
    m_disableEventsOnExternalMouse.set(m_disableEventsOnExternalMouseDefault);
    m_disableWhileTyping.set(m_disableWhileTypingEnabledByDefault);
    m_middleEmulation.set(m_middleEmulationEnabledByDefault);
    m_pointerAcceleration.set(m_defaultPointerAcceleration);
    m_pointerAccelerationProfileFlat.set(m_defaultPointerAccelerationProfileFlat);
    m_pointerAccelerationProfileAdaptive.set(m_defaultPointerAccelerationProfileAdaptive);
    m_naturalScroll.set(m_naturalScrollEnabledByDefault);
    m_horizontalScrolling.set(true);
    m_isScrollTwoFinger.set(m_scrollTwoFingerEnabledByDefault);
    m_isScrollEdge.set(m_scrollEdgeEnabledByDefault);
    m_isScrollOnButtonDown.set(m_scrollOnButtonDownEnabledByDefault);
    m_scrollButton.set(m_defaultScrollButton);
    m_clickMethodAreas.set(m_defaultClickMethodAreas);
    m_clickMethodClickfinger.set(m_defaultClickMethodClickfinger);

    return true;
}

bool LibinputTouchpad::isChangedConfig()
{
    // clang-format off
    bool changed = m_enabled.changed() ||
            m_tapToClick.changed() ||
            m_lrmTapButtonMap.changed() ||
            m_lmrTapButtonMap.changed() ||
            m_tapAndDrag.changed() ||
            m_tapDragLock.changed() ||
            m_leftHanded.changed() ||
            m_disableEventsOnExternalMouse.changed() ||
            m_disableWhileTyping.changed() ||
            m_middleEmulation.changed() ||
            m_pointerAcceleration.changed() ||
            m_pointerAccelerationProfileFlat.changed() ||
            m_pointerAccelerationProfileAdaptive.changed() ||
            m_naturalScroll.changed() ||
            m_horizontalScrolling.changed() ||
            m_isScrollTwoFinger.changed() ||
            m_isScrollEdge.changed() ||
            m_isScrollOnButtonDown.changed() ||
            m_scrollButton.changed() ||
            m_clickMethodAreas.changed() ||
            m_clickMethodClickfinger.changed();
    // clang-format on

    return changed;
}

int LibinputTouchpad::touchpadOff()
{
    return m_enabled.val;
}

XcbAtom &LibinputTouchpad::touchpadOffAtom()
{
    return *m_atoms[QLatin1String(LIBINPUT_PROP_SENDEVENTS_ENABLED)].get();
}

template<typename T>
bool LibinputTouchpad::valueLoader(Prop<T> &prop)
{
    const Parameter *p = findParameter(QString::fromLatin1(prop.name));

    if (!p) {
        qCCritical(KCM_TOUCHPAD) << "Error on read of " << QString::fromLatin1(prop.name);
    }

    QVariant reply = getParameter(p);
    if (!reply.isValid()) {
        prop.avail = false;
        return true;
    }
    prop.avail = true;

    auto touchpadConfig = m_config->group(m_name);

    const T replyValue = valueLoaderPart<T>(reply);
    const T loadedValue = touchpadConfig.readEntry(QString(prop.name), replyValue);
    prop.old = replyValue;
    prop.val = loadedValue;

    return true;
}

template<typename T>
QString LibinputTouchpad::valueWriter(const Prop<T> &prop)
{
    const Parameter *p = findParameter(QString::fromLatin1(prop.name));

    if (!p || !prop.changed()) {
        return QString();
    }

    bool error = !setParameter(p, prop.val);
    if (error) {
        qCCritical(KCM_TOUCHPAD) << "Cannot set property " + QString::fromLatin1(prop.name);
        return QStringLiteral("Cannot set property ") + QString::fromLatin1(prop.name);
    }
    auto touchpadConfig = m_config->group(m_name);
    touchpadConfig.writeEntry(QString(prop.name), prop.val);
    touchpadConfig.config()->sync();
    return QString();
}

#include "moc_libinputtouchpad.cpp"
