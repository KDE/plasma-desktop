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

#include "xinput_helper.h"

#include <QCoreApplication>
#include <QX11Info>
#include <QDebug>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#ifdef HAVE_XINPUT_AND_DEVICE_NOTIFY
#include <X11/extensions/XInput.h>
#endif

#ifdef HAVE_XCB_XINPUT
#include <xcb/xinput.h>
#endif

#include "x11_helper.h"

#include <fixx11h.h>

static const int DEVICE_NONE = 0;
static const int DEVICE_KEYBOARD = 1;
static const int DEVICE_POINTER = 2;

XInputEventNotifier::XInputEventNotifier(QWidget* parent):
	XEventNotifier(), //TODO: destruct properly?
	xinputEventType(-1)
{
  Q_UNUSED(parent)
}

void XInputEventNotifier::start()
{
	if( QCoreApplication::instance() != NULL ) {
		registerForNewDeviceEvent(QX11Info::display());
	}

	XEventNotifier::start();
}

void XInputEventNotifier::stop()
{
	XEventNotifier::stop();

	if( QCoreApplication::instance() != NULL ) {
	//    XEventNotifier::unregisterForNewDeviceEvent(QX11Info::display());
	}
}

bool XInputEventNotifier::processOtherEvents(xcb_generic_event_t* event)
{
	int newDeviceType = getNewDeviceEventType(event);
	if( newDeviceType == DEVICE_KEYBOARD ) {
		emit(newKeyboardDevice());
	}
	else if( newDeviceType == DEVICE_POINTER ) {
		emit(newPointerDevice());
		emit(newKeyboardDevice());	// arghhh, looks like X resets xkb map even when only pointer device is connected
	}
	return true;
}


#ifdef HAVE_XINPUT_AND_DEVICE_NOTIFY

//extern "C" {
//    extern int _XiGetDevicePresenceNotifyEvent(Display *);
//}

// This is ugly but allows to skip multiple execution of setxkbmap 
// for all keyboard devices that don't care about layouts
static bool isRealKeyboard(const char* deviceName)
{
	return strstr(deviceName, "Video Bus") == NULL
		&& strstr(deviceName, "Sleep Button") == NULL
		&& strstr(deviceName, "Power Button") == NULL
		&& strstr(deviceName, "WMI hotkeys") == NULL;
}

int XInputEventNotifier::getNewDeviceEventType(xcb_generic_event_t* event)
{
	int newDeviceType = DEVICE_NONE;
#ifdef HAVE_XCB_XINPUT
	if( xinputEventType != -1 && event->response_type == xinputEventType ) {
		xcb_input_device_presence_notify_event_t *xdpne = reinterpret_cast<xcb_input_device_presence_notify_event_t *>(event);
		if( xdpne->devchange == DeviceEnabled ) {
			int ndevices;
			XDeviceInfo *devices = XListInputDevices(display, &ndevices);
			if( devices != NULL ) {
				qCDebug(KCM_KEYBOARD) << "New device id:" << xdpne->device_id;
				for(int i=0; i<ndevices; i++) {
					qCDebug(KCM_KEYBOARD) << "id:" << devices[i].id << "name:" << devices[i].name << "used as:" << devices[i].use;
					if( devices[i].id == xdpne->device_id ) {
						if( devices[i].use == IsXKeyboard || devices[i].use == IsXExtensionKeyboard ) {
							if( isRealKeyboard(devices[i].name) ) {
								newDeviceType = DEVICE_KEYBOARD;
								qCDebug(KCM_KEYBOARD) << "new keyboard device, id:" << devices[i].id << "name:" << devices[i].name << "used as:" << devices[i].use;
								break;
							}
						}
						if( devices[i].use == IsXPointer || devices[i].use == IsXExtensionPointer ) {
							newDeviceType = DEVICE_POINTER;
							qCDebug(KCM_KEYBOARD) << "new pointer device, id:" << devices[i].id << "name:" << devices[i].name << "used as:" << devices[i].use;
							break;
						}
					}
				}
				XFreeDeviceList(devices);
			}
		}
	}
#endif
	return newDeviceType;
}

int XInputEventNotifier::registerForNewDeviceEvent(Display* display_)
{
	int xitype;
	XEventClass xiclass;
	display = display_;

	DevicePresence(display, xitype, xiclass);
	XSelectExtensionEvent(display, DefaultRootWindow(display), &xiclass, 1);
	qCDebug(KCM_KEYBOARD) << "Registered for new device events from XInput, class" << xitype;
	xinputEventType = xitype;
	return xitype;
}

#else

#ifdef __GNUC__
#warning "Keyboard daemon is compiled without XInput, keyboard settings will be reset when new keyboard device is plugged in!"
#endif

int XInputEventNotifier::registerForNewDeviceEvent(Display* /*display*/)
{
	qWarning() << "Keyboard kded daemon is compiled without XInput, xkb configuration will be reset when new keyboard device is plugged in!";
	return -1;
}

int XInputEventNotifier::getNewDeviceEventType(xcb_generic_event_t* /*event*/)
{
	return DEVICE_NONE;
}

#endif
