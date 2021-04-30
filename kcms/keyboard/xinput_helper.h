/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef XINPUT_HELPER_H_
#define XINPUT_HELPER_H_

#include "x11_helper.h"

#include <X11/Xlib.h>
#include <fixx11h.h>

class QTimer;
class UdevDeviceNotifier;

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
    UdevDeviceNotifier *udevNotifier;
    QTimer *keyboardNotificationTimer;
    QTimer *mouseNotificationTimer;
};

#endif /* XINPUT_HELPER_H_ */
