/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KConfigGroup>
#include <KModifierKeyInfo>
#include <KSharedConfig>

#include <QDebug>
#include <QX11Info>

#include <math.h>

#include "debug.h"
#include "kcmmisc.h"
#include "x11_helper.h"

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

// This code is taken from xset utility from XFree 4.3 (https://www.xfree86.org/)

static void set_repeatrate(int delay, double rate)
{
    if (!X11Helper::xkbSupported(nullptr)) {
        qCCritical(KCM_KEYBOARD) << "Failed to set keyboard repeat rate: xkb is not supported";
        return;
    }

    XkbDescPtr xkb = XkbAllocKeyboard();
    if (xkb) {
        Display *dpy = QX11Info::display();
        // int res =
        XkbGetControls(dpy, XkbRepeatKeysMask, xkb);
        xkb->ctrls->repeat_delay = delay;
        xkb->ctrls->repeat_interval = (int)floor(1000 / rate + 0.5);
        // res =
        XkbSetControls(dpy, XkbRepeatKeysMask, xkb);
        XkbFreeKeyboard(xkb, 0, true);
        return;
    }
}

static int set_repeat_mode(bool enabled)
{
    XKeyboardState kbd;
    XKeyboardControl kbdc;

    XGetKeyboardControl(QX11Info::display(), &kbd);

    kbdc.auto_repeat_mode = (enabled ? AutoRepeatModeOn : AutoRepeatModeOff);

    return XChangeKeyboardControl(QX11Info::display(), KBAutoRepeatMode, &kbdc);
}

void init_keyboard_hardware()
{
    auto inputConfig = KSharedConfig::openConfig(QStringLiteral("kcminputrc"));
    inputConfig->reparseConfiguration();
    KConfigGroup config(inputConfig, "Keyboard");

    QString keyRepeatStr = config.readEntry("KeyRepeat", "accent");

    if (keyRepeatStr == QLatin1String("accent") || keyRepeatStr == QLatin1String("repeat")) {
        int delay_ = config.readEntry("RepeatDelay", DEFAULT_REPEAT_DELAY);
        double rate_ = config.readEntry("RepeatRate", DEFAULT_REPEAT_RATE);
        set_repeatrate(delay_, rate_);
        set_repeat_mode(true);
    } else {
        set_repeat_mode(false);
    }

    TriState numlockState = TriStateHelper::getTriState(config.readEntry("NumLock", TriStateHelper::getInt(STATE_UNCHANGED)));
    if (numlockState != STATE_UNCHANGED) {
        KModifierKeyInfo keyInfo;
        keyInfo.setKeyLocked(Qt::Key_NumLock, numlockState == STATE_ON);
    }
    XFlush(QX11Info::display());
}
