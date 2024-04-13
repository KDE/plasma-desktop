/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "x11_helper.h"

#include <X11/Xlib.h>
#include <fixx11h.h>

class QTimer;
class UdevDeviceNotifier;

class XInputEventNotifier : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit XInputEventNotifier();
    ~XInputEventNotifier();

    int registerForNewDeviceEvent(Display *dpy);

Q_SIGNALS:
    void newKeyboardDevice();
    void newPointerDevice();
    void layoutChanged();
    void layoutMapChanged();

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;

private:
    bool processOtherEvents(xcb_generic_event_t *event);
    bool processXkbEvents(xcb_generic_event_t *e);
    int getNewDeviceEventType(xcb_generic_event_t *event);
    int registerForXkbEvents(Display *display);
    bool isXkbEvent(xcb_generic_event_t *event);
    bool isGroupSwitchEvent(_xkb_event *event);
    bool isLayoutSwitchEvent(_xkb_event *event);

    int xkbOpcode;
    int xinputEventType;
    Display *display;
    UdevDeviceNotifier *udevNotifier;
    QTimer *keyboardNotificationTimer;
    QTimer *mouseNotificationTimer;
};
