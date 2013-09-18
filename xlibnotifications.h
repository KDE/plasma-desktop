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

#ifndef XLIBNOTIFICATIONS_H
#define XLIBNOTIFICATIONS_H

#include <QWidget>
#include <QSocketNotifier>
#include <QSet>

#include <xcb/xcb.h>
#include <xcb/record.h>
#include <X11/X.h>

class XlibNotifications : public QWidget
{
    Q_OBJECT
public:
    XlibNotifications(Display *display, xcb_connection_t *connection,
                      int device);
    ~XlibNotifications();

    bool x11Event(XEvent *event);

Q_SIGNALS:
    void propertyChanged(Atom);
    void deviceChanged(int);

    void keyboardActivityStarted();
    void keyboardActivityFinished();

private Q_SLOTS:
    void recordEvent();

private:
    bool keyboardActivity() const;

    xcb_connection_t *m_connection;
    xcb_window_t m_inputWindow;
    int m_device;
    uint8_t m_inputOpcode;

    xcb_connection_t *m_recordConnection;
    QSocketNotifier *m_recordNotifier;
    xcb_record_context_t m_recordContext;
    xcb_record_enable_context_cookie_t m_recordCookie;

    QSet<xcb_keycode_t> m_modifiers, m_keysPressed, m_modifiersPressed;
};

#endif // XLIBNOTIFICATIONS_H
