/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "x11_helper.h"

#include <X11/Xlib.h>
#include <fixx11h.h>

class QTimer;

class XInputEventNotifier : public XEventNotifier
{
    Q_OBJECT

public:
    explicit XInputEventNotifier(QWidget *parent = nullptr);

    void start() override;
    void stop() override;

    int registerForNewDeviceEvent(Display *dpy);

Q_SIGNALS:
    void newKeyboardDevice();
    void newPointerDevice();

protected:
    bool processOtherEvents(xcb_generic_event_t *event) override;

private:
    int getNewDeviceEventType(xcb_generic_event_t *event);

    int xinputEventType;
    Display *display;
    QTimer *keyboardNotificationTimer;
    QTimer *mouseNotificationTimer;
};
