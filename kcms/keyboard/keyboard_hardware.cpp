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

#include <KConfigGroup>
#include <KSharedConfig>
#include <KModifierKeyInfo>

#include <QX11Info>
#include <QDebug>

#include <math.h>

#include "x11_helper.h"
#include "kcmmisc.h"
#include "debug.h"

#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

// This code is taken from xset utility from XFree 4.3 (https://www.xfree86.org/)

static
void set_repeatrate(int delay, double rate)
{
	if( !X11Helper::xkbSupported(nullptr) ) {
		qCCritical(KCM_KEYBOARD) << "Failed to set keyboard repeat rate: xkb is not supported";
		return;
	}

	XkbDescPtr xkb = XkbAllocKeyboard();
	if (xkb) {
		Display* dpy = QX11Info::display();
		//int res =
		XkbGetControls(dpy, XkbRepeatKeysMask, xkb);
		xkb->ctrls->repeat_delay = delay;
		xkb->ctrls->repeat_interval = (int)floor(1000/rate + 0.5);
		//res =
		XkbSetControls(dpy, XkbRepeatKeysMask, xkb);
        XkbFreeKeyboard(xkb, 0, true);
		return;
	}
}

static
int set_repeat_mode(TriState keyboardRepeatMode)
{
	XKeyboardState   kbd;
	XKeyboardControl kbdc;

	XGetKeyboardControl(QX11Info::display(), &kbd);

	int flags = 0;
	if( keyboardRepeatMode != STATE_UNCHANGED ) {
		flags |= KBAutoRepeatMode;
		kbdc.auto_repeat_mode = (keyboardRepeatMode==STATE_ON ? AutoRepeatModeOn : AutoRepeatModeOff);
	}

	return XChangeKeyboardControl(QX11Info::display(), flags, &kbdc);
}

void init_keyboard_hardware()
{
    KConfigGroup config(KSharedConfig::openConfig( QStringLiteral("kcminputrc") ), "Keyboard");

	QString keyRepeatStr = config.readEntry("KeyboardRepeating", TriStateHelper::getString(STATE_ON));
	TriState keyRepeat = STATE_UNCHANGED;
	if( keyRepeatStr == QLatin1String("true") || keyRepeatStr == TriStateHelper::getString(STATE_ON) ) {
		keyRepeat = STATE_ON;
	}
	else if( keyRepeatStr == QLatin1String("false") || keyRepeatStr == TriStateHelper::getString(STATE_OFF) ) {
		keyRepeat = STATE_OFF;
	}

	if( keyRepeat == STATE_ON ) {
		int delay_ = config.readEntry("RepeatDelay", DEFAULT_REPEAT_DELAY);
		double rate_ = config.readEntry("RepeatRate", DEFAULT_REPEAT_RATE);
		set_repeatrate(delay_, rate_);
	}

	set_repeat_mode(keyRepeat);

	TriState numlockState = TriStateHelper::getTriState( config.readEntry( "NumLock", TriStateHelper::getInt(STATE_UNCHANGED) ) );
	if( numlockState != STATE_UNCHANGED ) {
        KModifierKeyInfo keyInfo;
        keyInfo.setKeyLocked(Qt::Key_NumLock, numlockState == STATE_ON);
	}
}
