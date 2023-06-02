/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QSocketNotifier>
#include <QWidget>
#include <QtGui/private/qtx11extras_p.h>

#include <X11/Xlib.h>
#include <xcb/xcb.h>

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
