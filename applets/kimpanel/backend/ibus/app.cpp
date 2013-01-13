/*
 *  Copyright (C) 2013 Weng Xuetian <wengxt@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "app.h"
#include <QTimer>
#include <QDebug>
#include <QWidget>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

static void
ibus_disconnected_cb (IBusBus  *bus,
                      gpointer  user_data)
{
    Q_UNUSED(bus);
    App* app = (App*) user_data;
    app->finalize();
}

App::App(int argc, char** argv): QApplication(argc, argv)
{
    QTimer::singleShot(0, this, SLOT(init()));
}

bool App::x11EventFilter(XEvent* event)
{
#if 0
    if (event->xany.window == QX11Info::appRootWindow()) {
        if (event->type == KeyPress) {
            KeySym sym = XKeycodeToKeysym(QX11Info::display(), event->xkey.keycode, 0);
            if (event->xkey.state == 4 && sym == ' ') {

            }
        }
    }
#endif
    return QApplication::x11EventFilter(event);
}

void App::init()
{
    ibus_init ();
    bus = ibus_bus_new ();
    if (!bus || !ibus_bus_is_connected (bus)) {
        exit (1);
    }
    g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), this);
    impanel = ibus_panel_impanel_new (ibus_bus_get_connection (bus));
    ibus_bus_request_name (bus, IBUS_SERVICE_PANEL, 0);
    ibus_panel_impanel_set_bus(impanel, bus);
#if 0
    grabKey();
#endif
}

void App::grabKey()
{
    KeySym sym = ' ';
    uint modifiers = 4;
    Display* xdisplay = QX11Info::display();
    KeyCode keycode = XKeysymToKeycode (xdisplay, (gulong) sym);
    if (keycode == 0) {
        g_warning ("Can not convert keyval=%lu to keycode!", sym);
    }
    XGrabKey(QX11Info::display(), keycode, modifiers, QX11Info::appRootWindow(), True, GrabModeAsync, GrabModeAsync);
}

void App::ungrabKey()
{
    KeySym sym = ' ';
    uint modifiers = 4;
    Display* xdisplay = QX11Info::display();
    KeyCode keycode = XKeysymToKeycode (xdisplay, (gulong) sym);
    if (keycode == 0) {
        g_warning ("Can not convert keyval=%lu to keycode!", sym);
    }
    XUngrabKey(QX11Info::display(), keycode, modifiers, QX11Info::appRootWindow());
}


void App::finalize()
{
#if 0
    ungrabKey();
#endif
    clean();
    exit(0);
}

void App::clean()
{
    if (impanel) {
        g_object_unref(impanel);
        impanel = 0;
    }

    if (bus) {
        g_signal_handlers_disconnect_by_func(bus, (gpointer) ibus_disconnected_cb, this);
        g_object_unref(bus);
        bus = 0;
    }
}

App::~App()
{
    clean();
}
