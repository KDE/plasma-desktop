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

#include <QX11Info>
#include <QWidget>
#include <QSocketNotifier>

#include <xcb/xcb.h>
#include <X11/Xlib.h>

class XlibNotifications : public QObject
{
    Q_OBJECT
public:
    XlibNotifications(Display *display, int device);
    ~XlibNotifications();

Q_SIGNALS:
    void propertyChanged(xcb_atom_t);
    void devicePlugged(int);
    void touchpadDetached();

private Q_SLOTS:
    void processEvents();

private:
    void processEvent(XEvent *);

    Display *m_display;
    xcb_connection_t *m_connection;
    QSocketNotifier *m_notifier;
    xcb_window_t m_inputWindow;
    uint8_t m_inputOpcode;
    int m_device;
};

#endif // XLIBNOTIFICATIONS_H
