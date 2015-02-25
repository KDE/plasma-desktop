/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
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

/*
//krazy:excludeall=copyright
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * Copyright © 2002-2005,2007 Peter Osterlund
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Red Hat
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.  Red
 * Hat makes no representations about the suitability of this software
 * for any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:
 *      Peter Osterlund (petero2@telia.com)
 */

#include "synclientproperties.h"

#include <stddef.h>
#include <limits.h>
#include <xorg/synaptics-properties.h>

#define SYN_MAX_BUTTONS 12

const struct Parameter synapticsProperties[] = {
    {"LeftEdge",              PT_INT,    0, 10000, SYNAPTICS_PROP_EDGES,	32,	0},
    {"RightEdge",             PT_INT,    0, 10000, SYNAPTICS_PROP_EDGES,	32,	1},
    {"TopEdge",               PT_INT,    0, 10000, SYNAPTICS_PROP_EDGES,	32,	2},
    {"BottomEdge",            PT_INT,    0, 10000, SYNAPTICS_PROP_EDGES,	32,	3},
    {"FingerLow",             PT_INT,    0, 255,   SYNAPTICS_PROP_FINGER,	32,	0},
    {"FingerHigh",            PT_INT,    0, 255,   SYNAPTICS_PROP_FINGER,	32,	1},
    {"MaxTapTime",            PT_INT,    0, 1000,  SYNAPTICS_PROP_TAP_TIME,	32,	0},
    {"MaxTapMove",            PT_INT,    0, 2000,  SYNAPTICS_PROP_TAP_MOVE,	32,	0},
    {"MaxDoubleTapTime",      PT_INT,    0, 1000,  SYNAPTICS_PROP_TAP_DURATIONS,32,	1},
    {"SingleTapTimeout",      PT_INT,    0, 1000,  SYNAPTICS_PROP_TAP_DURATIONS,32,	0},
    {"ClickTime",             PT_INT,    0, 1000,  SYNAPTICS_PROP_TAP_DURATIONS,32,	2},
    {"FastTaps",              PT_BOOL,   0, 1,     SYNAPTICS_PROP_TAP_FAST,	8,	0},
    {"EmulateMidButtonTime",  PT_INT,    0, 1000,  SYNAPTICS_PROP_MIDDLE_TIMEOUT,32,	0},
    {"EmulateTwoFingerMinZ",  PT_INT,    0, 1000,  SYNAPTICS_PROP_TWOFINGER_PRESSURE,	32,	0},
    {"EmulateTwoFingerMinW",  PT_INT,    0, 15,    SYNAPTICS_PROP_TWOFINGER_WIDTH,	32,	0},
    {"VertScrollDelta",       PT_INT,    -1000, 1000,  SYNAPTICS_PROP_SCROLL_DISTANCE,	32,	0},
    {"HorizScrollDelta",      PT_INT,    -1000, 1000,  SYNAPTICS_PROP_SCROLL_DISTANCE,	32,	1},
    {"VertEdgeScroll",        PT_BOOL,   0, 1,     SYNAPTICS_PROP_SCROLL_EDGE,	8,	0},
    {"HorizEdgeScroll",       PT_BOOL,   0, 1,     SYNAPTICS_PROP_SCROLL_EDGE,	8,	1},
    {"CornerCoasting",        PT_BOOL,   0, 1,     SYNAPTICS_PROP_SCROLL_EDGE,	8,	2},
    {"VertTwoFingerScroll",   PT_BOOL,   0, 1,     SYNAPTICS_PROP_SCROLL_TWOFINGER,	8,	0},
    {"HorizTwoFingerScroll",  PT_BOOL,   0, 1,     SYNAPTICS_PROP_SCROLL_TWOFINGER,	8,	1},
    {"MinSpeed",              PT_DOUBLE, 0, 255.0,   SYNAPTICS_PROP_SPEED,	0, /*float */	0},
    {"MaxSpeed",              PT_DOUBLE, 0, 255.0,   SYNAPTICS_PROP_SPEED,	0, /*float */	1},
    {"AccelFactor",           PT_DOUBLE, 0, 1.0,   SYNAPTICS_PROP_SPEED,	0, /*float */	2},
    /*{"TouchpadOff",           PT_INT,    0, 2,     SYNAPTICS_PROP_OFF,		8,	0},*/
    {"LockedDrags",           PT_BOOL,   0, 1,     SYNAPTICS_PROP_LOCKED_DRAGS,	8,	0},
    {"LockedDragTimeout",     PT_INT,    0, 30000, SYNAPTICS_PROP_LOCKED_DRAGS_TIMEOUT,	32,	0},
    {"RTCornerButton",        PT_INT,    0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION,	8,	0},
    {"RBCornerButton",        PT_INT,    0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION,	8,	1},
    {"LTCornerButton",        PT_INT,    0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION,	8,	2},
    {"LBCornerButton",        PT_INT,    0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION,	8,	3},
    {"OneFingerTapButton",    PT_INT,    0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION,	8,	4},
    {"TwoFingerTapButton",    PT_INT,    0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION,	8,	5},
    {"ThreeFingerTapButton",  PT_INT,    0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_TAP_ACTION,	8,	6},
    {"ClickFinger1",          PT_INT,    0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_CLICK_ACTION,	8,	0},
    {"ClickFinger2",          PT_INT,    0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_CLICK_ACTION,	8,	1},
    {"ClickFinger3",          PT_INT,    0, SYN_MAX_BUTTONS, SYNAPTICS_PROP_CLICK_ACTION,	8,	2},
    {"CircularScrolling",     PT_BOOL,   0, 1,     SYNAPTICS_PROP_CIRCULAR_SCROLLING,	8,	0},
    {"CircScrollDelta",       PT_DOUBLE, .01, 3,   SYNAPTICS_PROP_CIRCULAR_SCROLLING_DIST,	0 /* float */,	0},
    {"CircScrollTrigger",     PT_INT,    0, 8,     SYNAPTICS_PROP_CIRCULAR_SCROLLING_TRIGGER,	8,	0},
    {"PalmDetect",            PT_BOOL,   0, 1,     SYNAPTICS_PROP_PALM_DETECT,	8,	0},
    {"PalmMinWidth",          PT_INT,    0, 15,    SYNAPTICS_PROP_PALM_DIMENSIONS,	32,	0},
    {"PalmMinZ",              PT_INT,    0, 255,   SYNAPTICS_PROP_PALM_DIMENSIONS,	32,	1},
    {"CoastingSpeed",         PT_DOUBLE, 0, 255,    SYNAPTICS_PROP_COASTING_SPEED,	0 /* float*/,	0},
    {"CoastingFriction",      PT_DOUBLE, 0, 255,   SYNAPTICS_PROP_COASTING_SPEED,	0 /* float*/,	1},
    {"PressureMotionMinZ",    PT_INT,    1, 255,   SYNAPTICS_PROP_PRESSURE_MOTION,	32,	0},
    {"PressureMotionMaxZ",    PT_INT,    1, 255,   SYNAPTICS_PROP_PRESSURE_MOTION,	32,	1},
    {"PressureMotionMinFactor", PT_DOUBLE, 0, 10.0,SYNAPTICS_PROP_PRESSURE_MOTION_FACTOR,	0 /*float*/,	0},
    {"PressureMotionMaxFactor", PT_DOUBLE, 0, 10.0,SYNAPTICS_PROP_PRESSURE_MOTION_FACTOR,	0 /*float*/,	1},
    {"GrabEventDevice",       PT_BOOL,   0, 1,     SYNAPTICS_PROP_GRAB,	8,	0},
    {"TapAndDragGesture",     PT_BOOL,   0, 1,     SYNAPTICS_PROP_GESTURES,	8,	0},
    {"AreaLeftEdge",          PT_INT,    0, 10000, SYNAPTICS_PROP_AREA,	32,	0},
    {"AreaRightEdge",         PT_INT,    0, 10000, SYNAPTICS_PROP_AREA,	32,	1},
    {"AreaTopEdge",           PT_INT,    0, 10000, SYNAPTICS_PROP_AREA,	32,	2},
    {"AreaBottomEdge",        PT_INT,    0, 10000, SYNAPTICS_PROP_AREA,	32,	3},
    {"HorizHysteresis",       PT_INT,    0, 10000, SYNAPTICS_PROP_NOISE_CANCELLATION, 32,	0},
    {"VertHysteresis",        PT_INT,    0, 10000, SYNAPTICS_PROP_NOISE_CANCELLATION, 32,	1},
    {"ClickPad",              PT_BOOL,   0, 1,     SYNAPTICS_PROP_CLICKPAD,	8,	0},
    {"RightButtonAreaLeft",   PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS,	32,	0},
    {"RightButtonAreaRight",  PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS,	32,	1},
    {"RightButtonAreaTop",    PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS,	32,	2},
    {"RightButtonAreaBottom", PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS,	32,	3},
    {"MiddleButtonAreaLeft",  PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS,	32,	4},
    {"MiddleButtonAreaRight", PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS,	32,	5},
    {"MiddleButtonAreaTop",   PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS,	32,	6},
    {"MiddleButtonAreaBottom", PT_INT, INT_MIN, INT_MAX, SYNAPTICS_PROP_SOFTBUTTON_AREAS,	32,	7},
    { NULL, 0, 0, 0, 0, 0, 0 }
};
