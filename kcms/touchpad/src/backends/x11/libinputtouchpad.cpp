/*
 * Copyright (C) 2015 Red Hat, Inc.
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

#include "libinputtouchpad.h"

#include <stddef.h>
#include <limits.h>

const struct Parameter libinputProperties[] = {
    /* This is a boolean for all three fingers, no per-finger config */
    {"Tapping",      PT_INT,    0, 1, "libinput Tapping Enabled",     8,     0},
    /* libinput normalizes the accel to -1/1 */
    {"AccelFactor",             PT_DOUBLE, -1.0, 1.0,   "libinput Accel Speed",  0 /*float */, 0},
    /* Only one of these may be set at one time */
    {"VertEdgeScroll",          PT_INT, 0, 1, "libinput Scroll Method Enabled", 8,     1},
    {"VertTwoFingerScroll",     PT_INT, 0, 1, "libinput Scroll Method Enabled", 8,     0},
    {"InvertVertScroll",     PT_INT, 0, 1, "libinput Natural Scrolling Enabled", 8,     0},
    /* libinput doesn't have a separate toggle for horiz scrolling */
    { NULL, PT_INT, 0, 0, 0, 0, 0 }
};

LibinputTouchpad::LibinputTouchpad(Display *display, int deviceId): XlibTouchpad(display, deviceId)
{
    loadSupportedProperties(libinputProperties);

    /* FIXME: has a different format than Synaptics Off but we don't expose
       the toggle so this is just to stop it from crashing when we check
       m_touchpadOffAtom  */
    m_touchpadOffAtom.intern(m_connection,
                             "libinput Send Events Mode enabled");


    XcbAtom scroll_methods(m_connection,
                           "libinput Scroll Methods Available",
                           true);
    if (scroll_methods.atom() != 0) {
        PropertyInfo methods(m_display,
                             m_deviceId,
                             scroll_methods.atom(),
                             0);
        if (!methods.value(0).toInt())
            m_supported.removeAll("VertTwoFingerScroll");
        else if (!methods.value(1).toInt())
            m_supported.removeAll("VertEdgeScroll");
    }
}
