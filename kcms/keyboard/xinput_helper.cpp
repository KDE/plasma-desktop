/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xinput_helper.h"
#include "debug.h"
#include <config-keyboard.h>

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QtGui/private/qtx11extras_p.h>

#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>

#if HAVE_XINPUT
#include <X11/extensions/XInput.h>
#include <xcb/xproto.h>
typedef struct xcb_input_device_presence_notify_event_t {
    uint8_t response_type;
    uint8_t pad0;
    uint16_t sequence;
    xcb_timestamp_t time;
    uint8_t devchange;
    uint8_t device_id;
    uint16_t control;
    uint8_t pad1[20];
} xcb_input_device_presence_notify_event_t;
// FIXME: #include <xcb/xinput.h> once xcb-xinput is stable
#endif

#include "udev_helper.h"

static const int DEVICE_NONE = 0;
static const int DEVICE_KEYBOARD = 1;
static const int DEVICE_POINTER = 2;

XInputEventNotifier::XInputEventNotifier()
    : QObject()
    , // TODO: destruct properly?
    xkbOpcode(-1)
    , xinputEventType(-1)
    , udevNotifier(nullptr)
    , keyboardNotificationTimer(new QTimer(this))
    , mouseNotificationTimer(new QTimer(this))
{
    // Q_EMIT signal only once, even after X11 re-enables N keyboards after resuming from suspend
    keyboardNotificationTimer->setSingleShot(true);
    keyboardNotificationTimer->setInterval(500);
    connect(keyboardNotificationTimer, &QTimer::timeout, this, &XInputEventNotifier::newKeyboardDevice);

    // same for mouse
    mouseNotificationTimer->setSingleShot(true);
    mouseNotificationTimer->setInterval(500);
    connect(mouseNotificationTimer, &QTimer::timeout, this, &XInputEventNotifier::newPointerDevice);
}

bool XInputEventNotifier::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *)
{
    //	qDebug() << "event type:" << eventType;
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);
        if (isXkbEvent(ev)) {
            processXkbEvents(ev);
        } else {
            processOtherEvents(ev);
        }
    }
    return false;
}

void XInputEventNotifier::start()
{
    registerForNewDeviceEvent(QX11Info::display());

    if (X11Helper::xkbSupported(&xkbOpcode)) {
        registerForXkbEvents(QX11Info::display());

        // start the event loop
        QCoreApplication::instance()->installNativeEventFilter(this);
    }
}

void XInputEventNotifier::stop()
{
    QCoreApplication::instance()->removeNativeEventFilter(this);
}

bool XInputEventNotifier::processOtherEvents(xcb_generic_event_t *event)
{
    int newDeviceType = getNewDeviceEventType(event);
    if (newDeviceType == DEVICE_KEYBOARD) {
        if (!keyboardNotificationTimer->isActive()) {
            keyboardNotificationTimer->start();
        }
    } else if (newDeviceType == DEVICE_POINTER) {
        if (!mouseNotificationTimer->isActive()) {
            mouseNotificationTimer->start();
        }
        // arghhh, looks like X resets xkb map even when only pointer device is connected
        if (!keyboardNotificationTimer->isActive()) {
            keyboardNotificationTimer->start();
        }
    }
    return true;
}

bool XInputEventNotifier::isXkbEvent(xcb_generic_event_t *event)
{
    //	qDebug() << "event response type:" << (event->response_type & ~0x80) << xkbOpcode << ((event->response_type & ~0x80) == xkbOpcode + XkbEventCode);
    return (event->response_type & ~0x80) == xkbOpcode + XkbEventCode;
}

bool XInputEventNotifier::processXkbEvents(xcb_generic_event_t *event)
{
    _xkb_event *xkbevt = reinterpret_cast<_xkb_event *>(event);
    if (isGroupSwitchEvent(xkbevt)) {
        //		qDebug() << "group switch event";
        Q_EMIT layoutChanged();
    } else if (isLayoutSwitchEvent(xkbevt)) {
        //		qDebug() << "layout switch event";
        Q_EMIT layoutMapChanged();
    }
    return true;
}

bool XInputEventNotifier::isGroupSwitchEvent(_xkb_event *xkbEvent)
{
//    XkbEvent *xkbEvent = (XkbEvent*) event;
#define GROUP_CHANGE_MASK (XkbGroupStateMask | XkbGroupBaseMask | XkbGroupLatchMask | XkbGroupLockMask)

    return xkbEvent->any.xkbType == XkbStateNotify && (xkbEvent->state_notify.changed & GROUP_CHANGE_MASK);
}

bool XInputEventNotifier::isLayoutSwitchEvent(_xkb_event *xkbEvent)
{
    //    XkbEvent *xkbEvent = (XkbEvent*) event;

    return //( (xkbEvent->any.xkb_type == XkbMapNotify) && (xkbEvent->map.changed & XkbKeySymsMask) ) ||
        /*    	  || ( (xkbEvent->any.xkb_type == XkbNamesNotify) && (xkbEvent->names.changed & XkbGroupNamesMask) || )*/
        (xkbEvent->any.xkbType == XkbNewKeyboardNotify);
}

int XInputEventNotifier::registerForXkbEvents(Display *display)
{
    int eventMask = XkbNewKeyboardNotifyMask | XkbStateNotifyMask;
    if (!XkbSelectEvents(display, XkbUseCoreKbd, eventMask, eventMask)) {
        qCWarning(KCM_KEYBOARD) << "Couldn't select desired XKB events";
        return false;
    }
    return true;
}

#if HAVE_XINPUT

// This is ugly but allows to skip multiple execution of setxkbmap
// for all keyboard devices that don't care about layouts
static bool isRealKeyboard(const char *deviceName)
{
    return strstr(deviceName, "Video Bus") == nullptr && strstr(deviceName, "Sleep Button") == nullptr && strstr(deviceName, "Power Button") == nullptr
        && strstr(deviceName, "WMI hotkeys") == nullptr && strstr(deviceName, "Camera") == nullptr;
}

int XInputEventNotifier::getNewDeviceEventType(xcb_generic_event_t *event)
{
    int newDeviceType = DEVICE_NONE;
    if (xinputEventType != -1 && event->response_type == xinputEventType) {
        xcb_input_device_presence_notify_event_t *xdpne = reinterpret_cast<xcb_input_device_presence_notify_event_t *>(event);
        if (xdpne->devchange == DeviceEnabled) {
            int ndevices;
            XDeviceInfo *devices = XListInputDevices(display, &ndevices);
            if (devices != nullptr) {
                qCDebug(KCM_KEYBOARD) << "New device id:" << xdpne->device_id;
                for (int i = 0; i < ndevices; i++) {
                    qCDebug(KCM_KEYBOARD) << "id:" << devices[i].id << "name:" << devices[i].name << "used as:" << devices[i].use;
                    if (devices[i].id == xdpne->device_id) {
                        if (devices[i].use == IsXKeyboard || devices[i].use == IsXExtensionKeyboard) {
                            if (isRealKeyboard(devices[i].name)) {
                                newDeviceType = DEVICE_KEYBOARD;
                                qCDebug(KCM_KEYBOARD) << "new keyboard device, id:" << devices[i].id << "name:" << devices[i].name
                                                      << "used as:" << devices[i].use;
                                break;
                            }
                        }
                        if (devices[i].use == IsXPointer || devices[i].use == IsXExtensionPointer) {
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
    return newDeviceType;
}

int XInputEventNotifier::registerForNewDeviceEvent(Display *display_)
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

#elif defined(HAVE_UDEV)

int XInputEventNotifier::registerForNewDeviceEvent(Display * /*display*/)
{
    if (!udevNotifier) {
        udevNotifier = new UdevDeviceNotifier(this);
        connect(udevNotifier, &UdevDeviceNotifier::newKeyboardDevice, this, &XInputEventNotifier::newKeyboardDevice);
        connect(udevNotifier, &UdevDeviceNotifier::newPointerDevice, this, &XInputEventNotifier::newPointerDevice);
        // Same as with XInput notifier, also Q_EMIT newKeyboardDevice when pointer device is found
        connect(udevNotifier, &UdevDeviceNotifier::newPointerDevice, this, &XInputEventNotifier::newKeyboardDevice);
    }

    return -1;
}

int XInputEventNotifier::getNewDeviceEventType(xcb_generic_event_t * /*event*/)
{
    return DEVICE_NONE;
}

#else

#ifdef __GNUC__
#warning "Keyboard daemon is compiled without XInput and UDev, keyboard settings will be reset when new keyboard device is plugged in!"
#endif

int XInputEventNotifier::registerForNewDeviceEvent(Display * /*display*/)
{
    qCWarning(KCM_KEYBOARD) << "Keyboard kded daemon is compiled without XInput, xkb configuration will be reset when new keyboard device is plugged in!";
    return -1;
}

int XInputEventNotifier::getNewDeviceEventType(xcb_generic_event_t * /*event*/)
{
    return DEVICE_NONE;
}

#endif
