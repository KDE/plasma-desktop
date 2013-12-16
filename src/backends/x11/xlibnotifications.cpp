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

#include "xlibnotifications.h"

#include <cstring>

#include <X11/extensions/XI.h>
#include <X11/extensions/XI2proto.h>
#include <X11/extensions/XInput2.h>

XlibNotifications::XlibNotifications(Display *display,
                                     xcb_connection_t *connection,
                                     int device)
    : m_connection(connection), m_device(device)
{
    m_notifier = new QSocketNotifier(xcb_get_file_descriptor(m_connection),
                                     QSocketNotifier::Read, this);

    xcb_query_extension_cookie_t inputExtCookie =
            xcb_query_extension(m_connection, std::strlen(INAME), INAME);
    xcb_query_extension_reply_t *inputExt =
            xcb_query_extension_reply(m_connection, inputExtCookie, 0);
    if (!inputExt) {
        return;
    }
    m_inputOpcode = inputExt->major_opcode;

    const xcb_setup_t *setup = xcb_get_setup(m_connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = iter.data;

    m_inputWindow = xcb_generate_id(m_connection);
    xcb_create_window(m_connection, 0, m_inputWindow, screen->root, 0, 0, 1, 1,
                      0, XCB_WINDOW_CLASS_INPUT_ONLY, 0, 0, 0);
    xcb_flush(m_connection);

    XIEventMask masks[2];

    unsigned char propertyMask[] = { 0, 0, 0, 0 };
    masks[0].deviceid = device;
    masks[0].mask = propertyMask;
    masks[0].mask_len = sizeof(propertyMask);
    XISetMask(propertyMask, XI_PropertyEvent);

    unsigned char hierarchyMask[] = { 0, 0, 0, 0 };
    masks[1].deviceid = XIAllDevices;
    masks[1].mask = hierarchyMask;
    masks[1].mask_len = sizeof(hierarchyMask);
    XISetMask(hierarchyMask, XI_HierarchyChanged);

    XISelectEvents(display, m_inputWindow, masks,
                   sizeof(masks) / sizeof(XIEventMask));
    XFlush(display);

    connect(m_notifier, SIGNAL(activated(int)), SLOT(processEvents()));
    m_notifier->setEnabled(true);
}

void XlibNotifications::processEvents()
{
    xcb_generic_event_t *event;
    while ((event = xcb_poll_for_event(m_connection))) {
        QScopedPointer<xcb_generic_event_t> eventData(event);
        processEvent(eventData.data());
    }
}

void XlibNotifications::processEvent(xcb_generic_event_t *event)
{
    if (event->response_type != GenericEvent) {
        return;
    }

    xGenericEvent *ge = reinterpret_cast<xGenericEvent*>(event);
    if (ge->extension != m_inputOpcode) {
        return;
    }

    if (ge->evtype == XI_PropertyEvent) {
        xXIPropertyEvent *propEvent = reinterpret_cast<xXIPropertyEvent *>(ge);
        Q_EMIT propertyChanged(propEvent->property);
    } else if (ge->evtype == XI_HierarchyChanged) {
        static const int acceptEvents =
                XIMasterAdded | XIMasterRemoved |
                XISlaveAttached | XISlaveDetached |
                XIDeviceEnabled | XIDeviceDisabled;

        xXIHierarchyEvent *hierarchyEvent =
                reinterpret_cast<xXIHierarchyEvent *>(ge);
        if (hierarchyEvent->flags & acceptEvents) {
            Q_EMIT deviceChanged(hierarchyEvent->deviceid);
        }
    }
}

XlibNotifications::~XlibNotifications()
{
    xcb_destroy_window(m_connection, m_inputWindow);
}

#include "moc_xlibnotifications.cpp"
