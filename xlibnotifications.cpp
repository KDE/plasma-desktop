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

#include <cstdlib>
#include <cstring>
#include <limits>
#include <cstdio>

#include <QScopedPointer>

#include <KApplication>

//Includes are ordered this way because of #defines in Xorg's headers
#include "xlibnotifications.h" // krazy:exclude=includes

#include <X11/extensions/XI.h>
#include <X11/extensions/XInput2.h>
#include <xcb/xcbext.h>

XlibNotifications::XlibNotifications(Display *display,
                                     xcb_connection_t *connection,
                                     int device)
    : m_connection(connection), m_device(device), m_recordConnection(0),
      m_modifiersPressed(0)
{
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
    kapp->installX11EventFilter(this);

    m_recordConnection = xcb_connect(0, 0);
    if (!m_recordConnection) {
        return;
    }

    xcb_get_modifier_mapping_cookie_t modCookie =
            xcb_get_modifier_mapping(m_recordConnection);

    m_recordContext = xcb_generate_id(m_recordConnection);
    xcb_record_range_t range;
    memset(&range, 0, sizeof(range));
    range.device_events.first = XCB_KEY_PRESS;
    range.device_events.last = XCB_KEY_RELEASE;
    xcb_record_client_spec_t cs = XCB_RECORD_CS_ALL_CLIENTS;
    xcb_record_create_context(m_recordConnection, m_recordContext, 0, 1, 1, &cs,
                              &range);
    xcb_flush(m_recordConnection);


    QScopedPointer<xcb_get_modifier_mapping_reply_t> modmap
            (xcb_get_modifier_mapping_reply(m_recordConnection, modCookie, 0));
    if (!modmap) {
        return;
    }

    int nModifiers = xcb_get_modifier_mapping_keycodes_length(modmap.data());
    xcb_keycode_t *modifiers = xcb_get_modifier_mapping_keycodes(modmap.data());
    m_modifier.fill(false, std::numeric_limits<xcb_keycode_t>::max() + 1);
    for (xcb_keycode_t *i = modifiers; i < modifiers + nModifiers; i++) {
        m_modifier[*i] = true;
    }
    m_pressed.fill(false, std::numeric_limits<xcb_keycode_t>::max() + 1);

    m_recordCookie =
            xcb_record_enable_context(m_recordConnection, m_recordContext);

    int fd = xcb_get_file_descriptor(m_recordConnection);
    m_recordNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_recordNotifier, SIGNAL(activated(int)), SLOT(recordEvent()));

    xcb_flush(m_recordConnection);
}

bool XlibNotifications::x11Event(XEvent *event)
{
    XGenericEventCookie *cookie = &event->xcookie;
    if (cookie->extension != m_inputOpcode) {
        return false;
    }

    XGetEventData(cookie->display, cookie);
    if (cookie->evtype == XI_PropertyEvent) {
        XIPropertyEvent *propEvent =
                reinterpret_cast<XIPropertyEvent *>(cookie->data);
        Q_EMIT propertyChanged(propEvent->property);
    } else if (cookie->evtype == XI_HierarchyChanged) {
        XIHierarchyEvent *hierarchyEvent =
                reinterpret_cast<XIHierarchyEvent *>(cookie->data);
        if (hierarchyEvent->flags & (XIMasterAdded |
                                     XIMasterRemoved |
                                     XISlaveAttached |
                                     XISlaveDetached |
                                     XIDeviceEnabled |
                                     XIDeviceDisabled))
        {
            Q_EMIT deviceChanged(hierarchyEvent->info->deviceid);
        }
    }
    XFreeEventData(cookie->display, cookie);

    return false;
}

void XlibNotifications::recordEvent()
{
    m_recordNotifier->setEnabled(false);

    xcb_record_enable_context_reply_t *reply = 0;
    xcb_poll_for_reply(m_recordConnection, m_recordCookie.sequence,
                       reinterpret_cast<void**>(&reply), 0);
    if (!reply) {
        std::free(xcb_poll_for_event(m_recordConnection));
        m_recordNotifier->setEnabled(true);
        return;
    }

    xcb_key_press_event_t *events = reinterpret_cast<xcb_key_press_event_t*>
            (xcb_record_enable_context_data(reply));
    int nEvents = xcb_record_enable_context_data_length(reply) /
            sizeof(xcb_key_press_event_t);
    int nKeys = 0;
    for (xcb_key_press_event_t *e = events; e < events + nEvents; e++) {
        if (e->response_type != XCB_KEY_PRESS &&
                e->response_type != XCB_KEY_RELEASE)
        {
            continue;
        }

        bool pressed = (e->response_type == XCB_KEY_PRESS);
        if (m_pressed[e->detail] == pressed) {
            continue;
        }
        m_pressed[e->detail] = pressed;

        if (m_modifier[e->detail]) {
            if (pressed) {
                m_modifiersPressed++;
            } else {
                m_modifiersPressed--;
            }
        } else if (pressed) {
            nKeys++;
        }
    }

    std::free(reply);

    std::fprintf(stderr, "Keys %d Modifiers %d\n", nKeys, m_modifiersPressed);
    if (nKeys && !m_modifiersPressed) {
        Q_EMIT keyboardActivity();
    }

    m_recordNotifier->setEnabled(true);
}

XlibNotifications::~XlibNotifications()
{
    xcb_destroy_window(m_connection, m_inputWindow);

    if (!m_recordConnection) {
        return;
    }

    xcb_record_disable_context(m_recordConnection, m_recordContext);
    xcb_record_free_context(m_recordConnection, m_recordContext);
    xcb_disconnect(m_recordConnection);
}

#include "moc_xlibnotifications.cpp"
