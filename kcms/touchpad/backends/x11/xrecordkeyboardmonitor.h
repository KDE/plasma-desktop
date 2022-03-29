/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XRECORDKEYBOARDMONITOR_H
#define XRECORDKEYBOARDMONITOR_H

#include <QSocketNotifier>
#include <QVector>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QX11Info>
#else
#include <QtGui/private/qtx11extras_p.h>
#endif

#include <xcb/record.h>
#include <xcb/xcb.h>

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
    bool activity() const
    {
        return m_keysPressed && !m_modifiersPressed;
    }

    QSocketNotifier *m_notifier;
    xcb_connection_t *m_connection;
    xcb_record_context_t m_context;
    xcb_record_enable_context_cookie_t m_cookie;

    QVector<bool> m_modifier, m_ignore, m_pressed;
    int m_modifiersPressed, m_keysPressed;
};

#endif // XRECORDKEYBOARDMONITOR_H
