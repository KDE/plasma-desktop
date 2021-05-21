/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xrecordkeyboardmonitor.h"

#include <cstdlib>
#include <limits>

#include <QScopedPointer>

#include <X11/Xlib.h>
#include <xcb/xcbext.h>

XRecordKeyboardMonitor::XRecordKeyboardMonitor(Display *display)
    : m_connection(xcb_connect(XDisplayString(display), nullptr))
    , m_modifiersPressed(0)
    , m_keysPressed(0)
{
    if (!m_connection) {
        return;
    }

    xcb_get_modifier_mapping_cookie_t modmapCookie = xcb_get_modifier_mapping(m_connection);

    m_context = xcb_generate_id(m_connection);
    xcb_record_range_t range;
    memset(&range, 0, sizeof(range));
    range.device_events.first = XCB_KEY_PRESS;
    range.device_events.last = XCB_KEY_RELEASE;
    xcb_record_client_spec_t cs = XCB_RECORD_CS_ALL_CLIENTS;
    xcb_record_create_context(m_connection, m_context, 0, 1, 1, &cs, &range);
    xcb_flush(m_connection);

    QScopedPointer<xcb_get_modifier_mapping_reply_t, QScopedPointerPodDeleter> modmap(xcb_get_modifier_mapping_reply(m_connection, modmapCookie, nullptr));
    if (!modmap) {
        return;
    }

    int nModifiers = xcb_get_modifier_mapping_keycodes_length(modmap.data());
    xcb_keycode_t *modifiers = xcb_get_modifier_mapping_keycodes(modmap.data());
    m_modifier.fill(false, std::numeric_limits<xcb_keycode_t>::max() + 1);
    for (xcb_keycode_t *i = modifiers; i < modifiers + nModifiers; i++) {
        m_modifier[*i] = true;
    }
    m_ignore.fill(false, std::numeric_limits<xcb_keycode_t>::max() + 1);
    for (xcb_keycode_t *i = modifiers; i < modifiers + modmap->keycodes_per_modifier; i++) {
        m_ignore[*i] = true;
    }
    m_pressed.fill(false, std::numeric_limits<xcb_keycode_t>::max() + 1);

    m_cookie = xcb_record_enable_context(m_connection, m_context);
    xcb_flush(m_connection);

    m_notifier = new QSocketNotifier(xcb_get_file_descriptor(m_connection), QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &XRecordKeyboardMonitor::processNextReply);
    m_notifier->setEnabled(true);
}

XRecordKeyboardMonitor::~XRecordKeyboardMonitor()
{
    if (!m_connection) {
        return;
    }

    xcb_record_disable_context(m_connection, m_context);
    xcb_record_free_context(m_connection, m_context);
    xcb_disconnect(m_connection);
}

void XRecordKeyboardMonitor::processNextReply()
{
    xcb_generic_event_t *event;
    while ((event = xcb_poll_for_event(m_connection))) {
        std::free(event);
    }

    void *reply = nullptr;
    xcb_generic_error_t *error = nullptr;
    while (m_cookie.sequence && xcb_poll_for_reply(m_connection, m_cookie.sequence, &reply, &error)) {
        // xcb_poll_for_reply may set both reply and error to null if connection has error.
        // break if xcb_connection has error, no point to continue anyway.
        if (xcb_connection_has_error(m_connection)) {
            break;
        }

        if (error) {
            std::free(error);
            break;
        }

        if (!reply) {
            continue;
        }

        QScopedPointer<xcb_record_enable_context_reply_t, QScopedPointerPodDeleter> data(reinterpret_cast<xcb_record_enable_context_reply_t *>(reply));
        process(data.data());
    }
}

void XRecordKeyboardMonitor::process(xcb_record_enable_context_reply_t *reply)
{
    bool prevActivity = activity();

    xcb_key_press_event_t *events = reinterpret_cast<xcb_key_press_event_t *>(xcb_record_enable_context_data(reply));
    int nEvents = xcb_record_enable_context_data_length(reply) / sizeof(xcb_key_press_event_t);
    bool wasActivity = prevActivity;
    for (xcb_key_press_event_t *e = events; e < events + nEvents; e++) {
        if (e->response_type != XCB_KEY_PRESS && e->response_type != XCB_KEY_RELEASE) {
            continue;
        }

        if (m_ignore[e->detail]) {
            continue;
        }

        bool pressed = (e->response_type == XCB_KEY_PRESS);
        if (m_pressed[e->detail] == pressed) {
            continue;
        }
        m_pressed[e->detail] = pressed;

        int &counter = m_modifier[e->detail] ? m_modifiersPressed : m_keysPressed;
        if (pressed) {
            counter++;
        } else {
            counter--;
        }

        wasActivity = wasActivity || activity();
    }

    if (!prevActivity && activity()) {
        Q_EMIT keyboardActivityStarted();
    } else if (!activity() && wasActivity) {
        Q_EMIT keyboardActivityFinished();
    }
}
