/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xlibnotifications.h"

#include <cstring>

#include <X11/Xlib-xcb.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XI2proto.h>
#include <X11/extensions/XInput2.h>

XlibNotifications::XlibNotifications(Display *display, int device)
    : m_display(display)
    , m_device(device)
{
    m_connection = XGetXCBConnection(display);

    m_notifier = new QSocketNotifier(xcb_get_file_descriptor(m_connection), QSocketNotifier::Read, this);

    xcb_query_extension_cookie_t inputExtCookie = xcb_query_extension(m_connection, std::strlen(INAME), INAME);
    QScopedPointer<xcb_query_extension_reply_t, QScopedPointerPodDeleter> inputExt(xcb_query_extension_reply(m_connection, inputExtCookie, nullptr));
    if (!inputExt) {
        return;
    }
    m_inputOpcode = inputExt->major_opcode;

    const xcb_setup_t *setup = xcb_get_setup(m_connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = iter.data;

    m_inputWindow = xcb_generate_id(m_connection);
    xcb_create_window(m_connection, 0, m_inputWindow, screen->root, 0, 0, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_ONLY, 0, 0, nullptr);
    xcb_flush(m_connection);

    XIEventMask masks[2];

    unsigned char touchpadMask[] = {0, 0, 0, 0};
    masks[0].deviceid = device;
    masks[0].mask = touchpadMask;
    masks[0].mask_len = sizeof(touchpadMask);
    XISetMask(touchpadMask, XI_PropertyEvent);

    unsigned char allMask[] = {0, 0, 0, 0};
    masks[1].deviceid = XIAllDevices;
    masks[1].mask = allMask;
    masks[1].mask_len = sizeof(allMask);
    XISetMask(allMask, XI_HierarchyChanged);

    XISelectEvents(display, XDefaultRootWindow(display), masks, sizeof(masks) / sizeof(XIEventMask));
    XFlush(display);

    connect(m_notifier, SIGNAL(activated(int)), SLOT(processEvents()));
    m_notifier->setEnabled(true);
}

void XlibNotifications::processEvents()
{
    while (XPending(m_display)) {
        XEvent event;
        XNextEvent(m_display, &event);
        processEvent(&event);
    }
}

struct XEventDataDeleter {
    XEventDataDeleter(Display *display, XGenericEventCookie *cookie)
        : m_display(display)
        , m_cookie(cookie)
    {
        XGetEventData(m_display, m_cookie);
    }

    ~XEventDataDeleter()
    {
        if (m_cookie->data) {
            XFreeEventData(m_display, m_cookie);
        }
    }

    Display *m_display;
    XGenericEventCookie *m_cookie;
};

void XlibNotifications::processEvent(XEvent *event)
{
    if (event->xcookie.type != GenericEvent) {
        return;
    }
    if (event->xcookie.extension != m_inputOpcode) {
        return;
    }

    if (event->xcookie.evtype == XI_PropertyEvent) {
        XEventDataDeleter helper(m_display, &event->xcookie);
        if (!event->xcookie.data) {
            return;
        }

        XIPropertyEvent *propEvent = reinterpret_cast<XIPropertyEvent *>(event->xcookie.data);
        Q_EMIT propertyChanged(propEvent->property);
    } else if (event->xcookie.evtype == XI_HierarchyChanged) {
        XEventDataDeleter helper(m_display, &event->xcookie);
        if (!event->xcookie.data) {
            return;
        }

        XIHierarchyEvent *hierarchyEvent = reinterpret_cast<XIHierarchyEvent *>(event->xcookie.data);
        for (uint16_t i = 0; i < hierarchyEvent->num_info; i++) {
            if (hierarchyEvent->info[i].deviceid == m_device) {
                if (hierarchyEvent->info[i].flags & XISlaveRemoved) {
                    Q_EMIT touchpadDetached();
                    return;
                }
            }
            if (hierarchyEvent->info[i].use != XISlavePointer) {
                continue;
            }
            if (hierarchyEvent->info[i].flags & (XIDeviceEnabled | XIDeviceDisabled)) {
                Q_EMIT devicePlugged(hierarchyEvent->info[i].deviceid);
            }
        }
    }
}

XlibNotifications::~XlibNotifications()
{
    xcb_destroy_window(m_connection, m_inputWindow);
    xcb_flush(m_connection);
}

#include "moc_xlibnotifications.cpp"
