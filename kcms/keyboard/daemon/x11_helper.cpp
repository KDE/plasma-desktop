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

#include "x11_helper.h"
#include "debug.h"
#include "../xkb_rules.h"

#define explicit explicit_is_keyword_in_cpp
#include <xcb/xkb.h>
#undef explicit

#include <QX11Info>
#include <QCoreApplication>
#include <QDebug>

#include <KLocalizedString>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>
#include <xcb/xcb.h>
#include <fixx11h.h>

// more information about the limit https://bugs.freedesktop.org/show_bug.cgi?id=19501
const int X11Helper::MAX_GROUP_COUNT = 4;
const int X11Helper::ARTIFICIAL_GROUP_LIMIT_COUNT = 8;

const char X11Helper::LEFT_VARIANT_STR[] = "(";
const char X11Helper::RIGHT_VARIANT_STR[] = ")";

bool X11Helper::xkbSupported(int* xkbOpcode)
{
    if (!QX11Info::isPlatformX11()) {
        return false;
    }
    // Verify the Xlib has matching XKB extension.

    int major = XkbMajorVersion;
    int minor = XkbMinorVersion;

    if (!XkbLibraryVersion(&major, &minor))
    {
        qCWarning(KCM_KEYBOARD) << "Xlib XKB extension " << major << '.' << minor <<
            " != " << XkbMajorVersion << '.' << XkbMinorVersion;
        return false;
    }

    // Verify the X server has matching XKB extension.

    int opcode_rtrn;
    int error_rtrn;
    int xkb_opcode;
    if( ! XkbQueryExtension(QX11Info::display(), &opcode_rtrn, &xkb_opcode, &error_rtrn, &major, &minor)) {
        qCWarning(KCM_KEYBOARD) << "X server XKB extension " << major << '.' << minor <<
            " != " << XkbMajorVersion << '.' << XkbMinorVersion;
        return false;
    }

    if( xkbOpcode != NULL ) {
    	*xkbOpcode = xkb_opcode;
    }

    return true;
}

XEventNotifier::XEventNotifier():
		xkbOpcode(-1)
{
	if( QCoreApplication::instance() == NULL ) {
		qCWarning(KCM_KEYBOARD) << "Layout Widget won't work properly without QCoreApplication instance";
	}
}

void XEventNotifier::start()
{
	qCDebug(KCM_KEYBOARD) << "qCoreApp" << QCoreApplication::instance();
	if( QCoreApplication::instance() != NULL && X11Helper::xkbSupported(&xkbOpcode) ) {
		registerForXkbEvents(QX11Info::display());

		// start the event loop
		QCoreApplication::instance()->installNativeEventFilter(this);
	}
}

void XEventNotifier::stop()
{
	if( QCoreApplication::instance() != NULL ) {
		//TODO: unregister
	//    XEventNotifier::unregisterForXkbEvents(QX11Info::display());

		// stop the event loop
		QCoreApplication::instance()->removeNativeEventFilter(this);
	}
}

bool XEventNotifier::isXkbEvent(xcb_generic_event_t* event)
{
//	qDebug() << "event response type:" << (event->response_type & ~0x80) << xkbOpcode << ((event->response_type & ~0x80) == xkbOpcode + XkbEventCode);
    return (event->response_type & ~0x80) == xkbOpcode + XkbEventCode;
}

bool XEventNotifier::processOtherEvents(xcb_generic_event_t* /*event*/)
{
	return true;
}

bool XEventNotifier::processXkbEvents(xcb_generic_event_t* event)
{
	_xkb_event *xkbevt = reinterpret_cast<_xkb_event *>(event);
	if( XEventNotifier::isGroupSwitchEvent(xkbevt) ) {
//		qDebug() << "group switch event";
		emit(layoutChanged());
	}
	else if( XEventNotifier::isLayoutSwitchEvent(xkbevt) ) {
//		qDebug() << "layout switch event";
		emit(layoutMapChanged());
	}
	return true;
}

bool XEventNotifier::nativeEventFilter(const QByteArray &eventType, void *message, long *)
{
//	qDebug() << "event type:" << eventType;
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* ev = static_cast<xcb_generic_event_t *>(message);
        if( isXkbEvent(ev) ) {
    		processXkbEvents(ev);
    	}
    	else {
    		processOtherEvents(ev);
    	}
    }
    return false;
}

//bool XEventNotifier::x11Event(XEvent * event)
//{
//	//    qApp->x11ProcessEvent ( event );
//	if( isXkbEvent(event) ) {
//		processXkbEvents(event);
//	}
//	else {
//		processOtherEvents(event);
//	}
//	return QWidget::x11Event(event);
//}

bool XEventNotifier::isGroupSwitchEvent(_xkb_event* xkbEvent)
{
//    XkbEvent *xkbEvent = (XkbEvent*) event;
#define GROUP_CHANGE_MASK \
    ( XkbGroupStateMask | XkbGroupBaseMask | XkbGroupLatchMask | XkbGroupLockMask )

    return xkbEvent->any.xkbType == XkbStateNotify && (xkbEvent->state_notify.changed & GROUP_CHANGE_MASK);
}

bool XEventNotifier::isLayoutSwitchEvent(_xkb_event* xkbEvent)
{
//    XkbEvent *xkbEvent = (XkbEvent*) event;

    return //( (xkbEvent->any.xkb_type == XkbMapNotify) && (xkbEvent->map.changed & XkbKeySymsMask) ) ||
/*    	  || ( (xkbEvent->any.xkb_type == XkbNamesNotify) && (xkbEvent->names.changed & XkbGroupNamesMask) || )*/
    	   (xkbEvent->any.xkbType == XkbNewKeyboardNotify);
}

int XEventNotifier::registerForXkbEvents(Display* display)
{
    int eventMask = XkbNewKeyboardNotifyMask | XkbStateNotifyMask;
    if( ! XkbSelectEvents(display, XkbUseCoreKbd, eventMask, eventMask) ) {
    	qCWarning(KCM_KEYBOARD) << "Couldn't select desired XKB events";
    	return false;
    }
    return true;
}

QString getDisplayText(const QString& layout, const QString& variant, const XkbRules* rules)
{
    if( variant.isEmpty() )
        return layout;
    if( rules == nullptr || rules->version == QLatin1String("1.0") )
        return i18nc("layout - variant", "%1 - %2", layout, variant);
    return variant;
}
