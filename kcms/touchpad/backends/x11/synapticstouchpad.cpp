/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>
    SPDX-FileContributor: 2002-2005, 2007 Peter Osterlund <petero2@telia.com>

    SPDX-License-Identifier: GPL-2.0-or-later AND LicenseRef-synaptics
*/

#include <QDebug>
#include <cmath>

#include "synapticstouchpad.h"

#include <limits.h>
#include <stddef.h>
#include <synaptics-properties.h>

#define SYN_MAX_BUTTONS 12

const struct Parameter synapticsProperties[] = {
    {"LeftEdge", PT_INT, 0, 10000, SYNAPTICS_PROP_EDGES, 32, 0},
    {"RightEdge", PT_INT, 0, 10000, SYNAPTICS_PROP_EDGES, 32, 1},
    {"TopEdge", PT_INT, 0, 10000, SYNAPTICS_PROP_EDGES, 32, 2},
    {"BottomEdge", PT_INT, 0, 10000, SYNAPTICS_PROP_EDGES, 32, 3},
    {"FingerLow", PT_INT, 0, 255, SYNAPTICS_PROP_FINGER, 32, 0},
    {"FingerHigh", PT_INT, 0, 255, SYNAPTICS_PROP_FINGER, 32, 1},
    {"MaxTapTime", PT_INT, 0, 1000, SYNAPTICS_PROP_TAP_TIME, 32, 0},
    {"MaxTapMove", PT_INT, 0, 2000, SYNAPTICS_PROP_TAP_MOVE, 32, 0},
    {"MaxDoubleTapTime", PT_INT, 0, 1000, SYNAPTICS_PROP_TAP_DURATIONS, 32, 1},
    {"SingleTapTimeout", PT_INT, 0, 1000, SYNAPTICS_PROP_TAP_DURATIONS, 32, 0},
    {"ClickTime", PT_INT, 0, 1000, SYNAPTICS_PROP_TAP_DURATIONS, 32, 2},
    {"FastTaps", PT_BOOL, 0, 1, SYNAPTICS_PROP_TAP_FAST, 8, 0},
    {"EmulateMidButtonTime", PT_INT, 0, 1000, SYNAPTICS_PROP_MIDDLE_TIMEOUT, 32, 0},
    {"EmulateTwoFingerMinZ", PT_INT, 0, 1000, SYNAPTICS_PROP_TWOFINGER_PRESSURE, 32, 0},
    {"EmulateTwoFingerMinW", PT_INT, 0, 15, SYNAPTICS_PROP_TWOFINGER_WIDTH, 32, 0},
    {"VertScrollDelta", PT_INT, -1000, 1000, SYNAPTICS_PROP_SCROLL_DISTANCE, 32, 0},
    {"HorizScrollDelta", PT_INT, -1000, 1000, SYNAPTICS_PROP_SCROLL_DISTANCE, 32, 1},
    {"VertEdgeScroll", PT_BOOL, 0, 1, SYNAPTICS_PROP_SCROLL_EDGE, 8, 0},
    {"HorizEdgeScroll", PT_BOOL, 0, 1, SYNAPTICS_PROP_SCROLL_EDGE, 8, 1},
    {"CornerCoasting", PT_BOOL, 0, 1, SYNAPTICS_PROP_SCROLL_EDGE, 8, 2},
    {"VertTwoFingerScroll", PT_BOOL, 0, 1, SYNAPTICS_PROP_SCROLL_TWOFINGER, 8, 0},
    {"HorizTwoFingerScroll", PT_BOOL, 0, 1, SYNAPTICS_PROP_SCROLL_TWOFINGER, 8, 1},
    {"MinSpeed", PT_DOUBLE, 0, 255.0, SYNAPTICS_PROP_SPEED, 0, /*float */ 0},
    {"MaxSpeed", PT_DOUBLE, 0, 255.0, SYNAPTICS_PROP_SPEED, 0, /*float */ 1},
    {"AccelFactor", PT_DOUBLE, 0, 1.0, SYNAPTICS_PROP_SPEED, 0, /*float */ 2},
    /*{"TouchpadOff",           PT_INT,    0, 2,     SYNAPTICS_PROP_OFF,        8,  0},*/
    {"LockedDrags", PT_BOOL, 0, 1, SYNAPTICS_PROP_LOCKED_DRAGS, 8, 0},
    {"LockedDragTimeout", PT_INT, 0, 30000, SYNAPTICS_PROP_LOCKED_DRAGS_TIMEOUT, 32, 0},
    {"RTCornerButton", PT_INT, 0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION, 8, 0},
    {"RBCornerButton", PT_INT, 0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION, 8, 1},
    {"LTCornerButton", PT_INT, 0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION, 8, 2},
    {"LBCornerButton", PT_INT, 0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION, 8, 3},
    {"OneFingerTapButton", PT_INT, 0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION, 8, 4},
    {"TwoFingerTapButton", PT_INT, 0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION, 8, 5},
    {"ThreeFingerTapButton", PT_INT, 0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION, 8, 6},
    {"ClickFinger1", PT_INT, 0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_CLICK_ACTION, 8, 0},
    {"ClickFinger2", PT_INT, 0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_CLICK_ACTION, 8, 1},
    {"ClickFinger3", PT_INT, 0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_CLICK_ACTION, 8, 2},
    {"CircularScrolling", PT_BOOL, 0, 1, SYNAPTICS_PROP_CIRCULAR_SCROLLING, 8, 0},
    {"CircScrollDelta", PT_DOUBLE, .01, 3, SYNAPTICS_PROP_CIRCULAR_SCROLLING_DIST, 0 /* float */, 0},
    {"CircScrollTrigger", PT_INT, 0, 8, SYNAPTICS_PROP_CIRCULAR_SCROLLING_TRIGGER, 8, 0},
    {"PalmDetect", PT_BOOL, 0, 1, SYNAPTICS_PROP_PALM_DETECT, 8, 0},
    {"PalmMinWidth", PT_INT, 0, 15, SYNAPTICS_PROP_PALM_DIMENSIONS, 32, 0},
    {"PalmMinZ", PT_INT, 0, 255, SYNAPTICS_PROP_PALM_DIMENSIONS, 32, 1},
    {"CoastingSpeed", PT_DOUBLE, 0, 255, SYNAPTICS_PROP_COASTING_SPEED, 0 /* float*/, 0},
    {"CoastingFriction", PT_DOUBLE, 0, 255, SYNAPTICS_PROP_COASTING_SPEED, 0 /* float*/, 1},
    {"PressureMotionMinZ", PT_INT, 1, 255, SYNAPTICS_PROP_PRESSURE_MOTION, 32, 0},
    {"PressureMotionMaxZ", PT_INT, 1, 255, SYNAPTICS_PROP_PRESSURE_MOTION, 32, 1},
    {"PressureMotionMinFactor", PT_DOUBLE, 0, 10.0, SYNAPTICS_PROP_PRESSURE_MOTION_FACTOR, 0 /*float*/, 0},
    {"PressureMotionMaxFactor", PT_DOUBLE, 0, 10.0, SYNAPTICS_PROP_PRESSURE_MOTION_FACTOR, 0 /*float*/, 1},
    {"GrabEventDevice", PT_BOOL, 0, 1, SYNAPTICS_PROP_GRAB, 8, 0},
    {"TapAndDragGesture", PT_BOOL, 0, 1, SYNAPTICS_PROP_GESTURES, 8, 0},
    {"AreaLeftEdge", PT_INT, 0, 10000, SYNAPTICS_PROP_AREA, 32, 0},
    {"AreaRightEdge", PT_INT, 0, 10000, SYNAPTICS_PROP_AREA, 32, 1},
    {"AreaTopEdge", PT_INT, 0, 10000, SYNAPTICS_PROP_AREA, 32, 2},
    {"AreaBottomEdge", PT_INT, 0, 10000, SYNAPTICS_PROP_AREA, 32, 3},
    {"HorizHysteresis", PT_INT, 0, 10000, SYNAPTICS_PROP_NOISE_CANCELLATION, 32, 0},
    {"VertHysteresis", PT_INT, 0, 10000, SYNAPTICS_PROP_NOISE_CANCELLATION, 32, 1},
    {"ClickPad", PT_BOOL, 0, 1, SYNAPTICS_PROP_CLICKPAD, 8, 0},
    {"RightButtonAreaLeft", PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS, 32, 0},
    {"RightButtonAreaRight", PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS, 32, 1},
    {"RightButtonAreaTop", PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS, 32, 2},
    {"RightButtonAreaBottom", PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS, 32, 3},
    {"MiddleButtonAreaLeft", PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS, 32, 4},
    {"MiddleButtonAreaRight", PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS, 32, 5},
    {"MiddleButtonAreaTop", PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS, 32, 6},
    {"MiddleButtonAreaBottom", PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS, 32, 7},
    {NULL, PT_INT, 0, 0, nullptr, 0, 0},
};

SynapticsTouchpad::SynapticsTouchpad(Display *display, int deviceId)
    : XlibTouchpad(display, deviceId)
    , m_resX(1)
    , m_resY(1)
{
    m_capsAtom.intern(m_connection, SYNAPTICS_PROP_CAPABILITIES);
    m_touchpadOffAtom.intern(m_connection, SYNAPTICS_PROP_OFF);
    XcbAtom resolutionAtom(m_connection, SYNAPTICS_PROP_RESOLUTION);
    XcbAtom edgesAtom(m_connection, SYNAPTICS_PROP_EDGES);

    loadSupportedProperties(synapticsProperties);

    m_toRadians.append("CircScrollDelta");

    PropertyInfo edges(m_display, m_deviceId, edgesAtom, 0);
    if (edges.i && edges.nitems == 4) {
        int w = qAbs(edges.i[1] - edges.i[0]);
        int h = qAbs(edges.i[3] - edges.i[2]);
        m_resX = w / 90;
        m_resY = h / 50;
        qDebug() << "Width: " << w << " height: " << h;
        qDebug() << "Approx. resX: " << m_resX << " resY: " << m_resY;
    }

    PropertyInfo resolution(m_display, m_deviceId, resolutionAtom, 0);
    if (resolution.i && resolution.nitems == 2 && resolution.i[0] > 1 && resolution.i[1] > 1) {
        m_resY = qMin(static_cast<unsigned long>(resolution.i[0]), static_cast<unsigned long>(INT_MAX));
        m_resX = qMin(static_cast<unsigned long>(resolution.i[1]), static_cast<unsigned long>(INT_MAX));
        qDebug() << "Touchpad resolution: x: " << m_resX << " y: " << m_resY;
    }

    m_scaleByResX.append("HorizScrollDelta");
    m_scaleByResY.append("VertScrollDelta");
    m_scaleByResX.append("MaxTapMove");
    m_scaleByResY.append("MaxTapMove");

    m_resX = qMax(10, m_resX);
    m_resY = qMax(10, m_resY);
    qDebug() << "Final resolution x:" << m_resX << " y:" << m_resY;
    m_negate["HorizScrollDelta"] = "InvertHorizScroll";
    m_negate["VertScrollDelta"] = "InvertVertScroll";
    m_supported.append(m_negate.values());
    m_supported.append("Coasting");

    PropertyInfo caps(m_display, m_deviceId, m_capsAtom.atom(), 0);
    if (!caps.b) {
        return;
    }

    enum TouchpadCapabilitiy {
        TouchpadHasLeftButton,
        TouchpadHasMiddleButton,
        TouchpadHasRightButton,
        TouchpadTwoFingerDetect,
        TouchpadThreeFingerDetect,
        TouchpadPressureDetect,
        TouchpadPalmDetect,
        TouchpadCapsCount,
    };

    QVector<bool> cap(TouchpadCapsCount, false);
    std::copy(caps.b, caps.b + qMin(cap.size(), static_cast<int>(caps.nitems)), cap.begin());

    if (!cap[TouchpadTwoFingerDetect]) {
        m_supported.removeAll("HorizTwoFingerScroll");
        m_supported.removeAll("VertTwoFingerScroll");
        m_supported.removeAll("TwoFingerTapButton");
    }

    if (!cap[TouchpadThreeFingerDetect]) {
        m_supported.removeAll("ThreeFingerTapButton");
    }

    if (!cap[TouchpadPressureDetect]) {
        m_supported.removeAll("FingerHigh");
        m_supported.removeAll("FingerLow");

        m_supported.removeAll("PalmMinZ");
        m_supported.removeAll("PressureMotionMinZ");
        m_supported.removeAll("PressureMotionMinFactor");
        m_supported.removeAll("PressureMotionMaxZ");
        m_supported.removeAll("PressureMotionMaxFactor");
        m_supported.removeAll("EmulateTwoFingerMinZ");
    }

    if (!cap[TouchpadPalmDetect]) {
        m_supported.removeAll("PalmDetect");
        m_supported.removeAll("PalmMinWidth");
        m_supported.removeAll("PalmMinZ");
        m_supported.removeAll("EmulateTwoFingerMinW");
    }

    for (QMap<QString, QString>::Iterator i = m_negate.begin(); i != m_negate.end(); ++i) {
        if (!m_supported.contains(i.key())) {
            m_supported.removeAll(i.value());
        }
    }

    m_paramList = synapticsProperties;
}

void SynapticsTouchpad::setTouchpadOff(int touchpadOff)
{
    PropertyInfo off(m_display, m_deviceId, m_touchpadOffAtom.atom(), 0);
    if (off.b && *(off.b) != touchpadOff) {
        *(off.b) = touchpadOff;
        off.set();
    }

    flush();
}

int SynapticsTouchpad::touchpadOff()
{
    PropertyInfo off(m_display, m_deviceId, m_touchpadOffAtom.atom(), 0);
    return off.value(0).toInt();
}

XcbAtom &SynapticsTouchpad::touchpadOffAtom()
{
    return m_touchpadOffAtom;
}

double SynapticsTouchpad::getPropertyScale(const QString &name) const
{
    if (m_scaleByResX.contains(name) && m_scaleByResY.contains(name)) {
        return std::sqrt(static_cast<double>(m_resX) * m_resX + static_cast<double>(m_resY) * m_resY);
    } else if (m_scaleByResX.contains(name)) {
        return m_resX;
    } else if (m_scaleByResY.contains(name)) {
        return m_resY;
    } else if (m_toRadians.contains(name)) {
        return M_PI_4 / 45.0;
    }
    return 1.0;
}
