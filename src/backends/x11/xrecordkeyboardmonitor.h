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

#ifndef XRECORDKEYBOARDMONITOR_H
#define XRECORDKEYBOARDMONITOR_H

#include <QVector>
#include <QSocketNotifier>
#include <QX11Info>

#include <xcb/xcb.h>
#include <xcb/record.h>

class XRecordKeyboardMonitor : public QObject
{
    Q_OBJECT

public:
    XRecordKeyboardMonitor(Display *display);
    ~XRecordKeyboardMonitor();

Q_SIGNALS:
    void keyboardActivityStarted();
    void keyboardActivityFinished();

private Q_SLOTS:
    void processNextReply();

private:
    void process(xcb_record_enable_context_reply_t *reply);
    bool activity() const { return m_keysPressed && !m_modifiersPressed; }

    QSocketNotifier *m_notifier;
    xcb_connection_t *m_connection;
    xcb_record_context_t m_context;
    xcb_record_enable_context_cookie_t m_cookie;

    QVector<bool> m_modifier, m_ignore, m_pressed;
    int m_modifiersPressed, m_keysPressed;
};

#endif // XRECORDKEYBOARDMONITOR_H
