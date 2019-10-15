/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <KModifierKeyInfo>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QDebug>
#include <QX11Info>

#include <math.h>

#include "debug.h"
#include "x11_helper.h"

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

// This code is taken from xset utility from XFree 4.3 (http://www.xfree86.org/)

static void set_repeatrate(int delay, double rate)
{
    if (!X11Helper::xkbSupported(nullptr)) {
        qCCritical(KCM_KEYBOARD) << "Failed to set keyboard repeat rate: xkb is not supported";
        return;
    }

    XkbDescPtr xkb = XkbAllocKeyboard();
    if (xkb) {
        Display* dpy = QX11Info::display();

        XkbGetControls(dpy, XkbRepeatKeysMask, xkb);
        xkb->ctrls->repeat_delay = static_cast<unsigned short>(delay);
        xkb->ctrls->repeat_interval = static_cast<unsigned short>(floor(1000 / rate + 0.5));

        XkbSetControls(dpy, XkbRepeatKeysMask, xkb);
        XkbFreeKeyboard(xkb, 0, true);
        return;
    }
}

enum class TriState {
    STATE_ON = 0,
    STATE_OFF = 1,
    STATE_UNCHANGED = 2
};

const int DEFAULT_REPEAT_DELAY = 600;
const double DEFAULT_REPEAT_RATE = 25.0;

static int set_repeat_mode(TriState keyboardRepeatMode)
{
    XKeyboardState kbd;
    XKeyboardControl kbdc;

    XGetKeyboardControl(QX11Info::display(), &kbd);

    unsigned long flags = 0;
    if (keyboardRepeatMode != TriState::STATE_UNCHANGED) {
        flags |= KBAutoRepeatMode;
        kbdc.auto_repeat_mode = (keyboardRepeatMode == TriState::STATE_ON ? AutoRepeatModeOn : AutoRepeatModeOff);
    }

    return XChangeKeyboardControl(QX11Info::display(), flags, &kbdc);
}

void init_keyboard_hardware()
{
    KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("kcminputrc")), "Keyboard");

    QString keyRepeatStr = config.readEntry("KeyboardRepeating", "0");
    TriState keyRepeat = TriState::STATE_UNCHANGED;
    if (keyRepeatStr == QLatin1String("true") || keyRepeatStr == "0") {
        keyRepeat = TriState::STATE_ON;
    } else if (keyRepeatStr == QLatin1String("false") || keyRepeatStr == "1") {
        keyRepeat = TriState::STATE_OFF;
    }

    if (keyRepeat == TriState::STATE_ON) {
        int delay_ = config.readEntry<int>("RepeatDelay", DEFAULT_REPEAT_DELAY);
        double rate_ = config.readEntry<double>("RepeatRate", DEFAULT_REPEAT_RATE);
        set_repeatrate(delay_, rate_);
    }

    set_repeat_mode(keyRepeat);

    TriState numlockState = TriState(config.readEntry("NumLock", int(TriState::STATE_UNCHANGED)));
    if (numlockState != TriState::STATE_UNCHANGED) {
        KModifierKeyInfo keyInfo;
        keyInfo.setKeyLocked(Qt::Key_NumLock, numlockState == TriState::STATE_ON);
    }
}
