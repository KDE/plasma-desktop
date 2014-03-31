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

#include <kdebug.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <QX11Info>
#include <QCursor>	// WTF? - otherwise compiler complains

#include <X11/Xlib.h>

#include <math.h>

#include "x11_helper.h"
#include "kcmmisc.h"

// from numlockx.c
extern "C" void numlockx_change_numlock_state(Display* dpy, int state);

#include <X11/XKBlib.h>
#include <X11/keysym.h>


// This code is taken from xset utility from XFree 4.3 (http://www.xfree86.org/)

static
void set_repeatrate(int delay, double rate)
{
	if( !X11Helper::xkbSupported(NULL) ) {
		kError() << "Failed to set keyboard repeat rate: xkb is not supported";
		return;
	}

	XkbDescPtr xkb = XkbAllocKeyboard();
	if (xkb) {
		Display* dpy = QX11Info::display();
		int res = XkbGetControls(dpy, XkbRepeatKeysMask, xkb);
		xkb->ctrls->repeat_delay = delay;
		xkb->ctrls->repeat_interval = (int)floor(1000/rate + 0.5);
		res = XkbSetControls(dpy, XkbRepeatKeysMask, xkb);
                XkbFreeKeyboard(xkb, 0, true);
		return;
	}
}

static
int set_volume(int clickVolumePercent, TriState keyboardRepeatMode)
{
	XKeyboardState   kbd;
	XKeyboardControl kbdc;

	XGetKeyboardControl(QX11Info::display(), &kbd);

	int flags = 0;
	if( clickVolumePercent != -1 ) {
		flags |= KBKeyClickPercent;
		kbdc.key_click_percent = clickVolumePercent;
	}
	if( keyboardRepeatMode != STATE_UNCHANGED ) {
		flags |= KBAutoRepeatMode;
		kbdc.auto_repeat_mode = (keyboardRepeatMode==STATE_ON ? AutoRepeatModeOn : AutoRepeatModeOff);
	}

	return XChangeKeyboardControl(QX11Info::display(), flags, &kbdc);
}

void init_keyboard_hardware()
{
    KConfigGroup config(KSharedConfig::openConfig( "kcminputrc" ), "Keyboard");

	QString keyRepeatStr = config.readEntry("KeyboardRepeating", TriStateHelper::getString(STATE_ON));
	TriState keyRepeat = STATE_UNCHANGED;
	if( keyRepeatStr == "true" || keyRepeatStr == TriStateHelper::getString(STATE_ON) ) {
		keyRepeat = STATE_ON;
	}
	else if( keyRepeatStr == "false" || keyRepeatStr == TriStateHelper::getString(STATE_OFF) ) {
		keyRepeat = STATE_OFF;
	}

	int clickVolumePercent = config.readEntry("ClickVolume", -1);
	if( clickVolumePercent != -1 && keyRepeat != STATE_UNCHANGED ) {
		set_volume(clickVolumePercent, keyRepeat);
	}

	if( keyRepeat == STATE_ON ) {
		int delay_ = config.readEntry("RepeatDelay", 250);
		double rate_ = config.readEntry("RepeatRate", 30.);
		set_repeatrate(delay_, rate_);
	}


	TriState numlockState = TriStateHelper::getTriState( config.readEntry( "NumLock", TriStateHelper::getInt(STATE_UNCHANGED) ) );
	if( numlockState != STATE_UNCHANGED ) {
		numlockx_change_numlock_state(QX11Info::display(), numlockState == STATE_ON );
	}
}
