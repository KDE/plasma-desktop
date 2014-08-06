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
#include "gtkaccelparse_p.h"
#include "gdkkeysyms_p.h"
#include <QTimer>
#include <QDebug>
#include <QWidget>
#include <QX11Info>
#include <X11/Xlib.h>

#define USED_MASK (ShiftMask | ControlMask | Mod1Mask | Mod4Mask)

static void
ibus_disconnected_cb (IBusBus  *m_bus,
                      gpointer  user_data)
{
    Q_UNUSED(m_bus);
    App* app = (App*) user_data;
    app->finalize();
}

/*
 * copied from kwin/tabbox/tabbox.cpp
 * simplified a little
 */
static bool areKeySymXsDepressed(const uint keySyms[], int nKeySyms) {
    char keymap[32];

    XQueryKeymap(QX11Info::display(), keymap);

    for (int iKeySym = 0; iKeySym < nKeySyms; iKeySym++) {
        uint keySymX = keySyms[ iKeySym ];
        uchar keyCodeX = XKeysymToKeycode(QX11Info::display(), keySymX);
        int i = keyCodeX / 8;
        char mask = 1 << (keyCodeX - (i * 8));

        // Abort if bad index value,
        if (i < 0 || i >= 32)
            return false;

        // If ALL keys passed need to be depressed,
        // If we are looking for ANY key press, and this key is depressed,
        if (keymap[i] & mask)
            return true;
    }

    // If we were looking for ANY key press, then none was found, return false,
    return false;
}

/*
 * copied from kwin/tabbox/tabbox.cpp
 * simplified a little
 */
static bool areModKeysDepressed(guint mod) {
    uint rgKeySyms[10];
    int nKeySyms = 0;
    if (mod == 0)
        return false;

    mod &= GDK_MODIFIER_MASK;

    if (mod & ShiftMask) {
        rgKeySyms[nKeySyms++] = GDK_KEY_Shift_L;
        rgKeySyms[nKeySyms++] = GDK_KEY_Shift_R;
    }
    if (mod & ControlMask) {
        rgKeySyms[nKeySyms++] = GDK_KEY_Control_L;
        rgKeySyms[nKeySyms++] = GDK_KEY_Control_R;
    }
    if (mod & Mod1Mask) {
        rgKeySyms[nKeySyms++] = GDK_KEY_Alt_L;
        rgKeySyms[nKeySyms++] = GDK_KEY_Alt_R;
    }
    if (mod & Mod4Mask) {
        // It would take some code to determine whether the Win key
        // is associated with Super or Meta, so check for both.
        // See bug #140023 for details.
        rgKeySyms[nKeySyms++] = GDK_KEY_Super_L;
        rgKeySyms[nKeySyms++] = GDK_KEY_Super_R;
        rgKeySyms[nKeySyms++] = GDK_KEY_Meta_L;
        rgKeySyms[nKeySyms++] = GDK_KEY_Meta_R;
    }

    return areKeySymXsDepressed(rgKeySyms, nKeySyms);
}

App::App(int argc, char** argv): QApplication(argc, argv)
    ,m_bus(0)
    ,m_impanel(0)
    ,m_doGrab(false)
{
    QTimer::singleShot(0, this, SLOT(init()));
}

bool App::x11EventFilter(XEvent* event)
{
    if (event->xany.window == QX11Info::appRootWindow()) {
        if (event->type == KeyPress) {
            KeySym sym = XKeycodeToKeysym(QX11Info::display(), event->xkey.keycode, 0);
            uint state = event->xkey.state & USED_MASK;
            if (m_triggersList.contains(qMakePair<uint, uint>(sym, state))) {
                ibus_panel_impanel_navigate(m_impanel);
            }
        } else if (event->type == KeyRelease) {
        }
    }
    return QApplication::x11EventFilter(event);
}


void App::init()
{
    ibus_init ();
    m_bus = ibus_bus_new ();
    if (!m_bus || !ibus_bus_is_connected (m_bus)) {
        exit (1);
    }
    g_signal_connect (m_bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), this);
    m_impanel = ibus_panel_impanel_new (ibus_bus_get_connection (m_bus));
    ibus_panel_impanel_set_bus(m_impanel, m_bus);
    ibus_panel_impanel_set_app(m_impanel, this);
    ibus_bus_request_name (m_bus, IBUS_SERVICE_PANEL, 0);
}

void App::setTriggerKeys(QList< TriggerKey > triggersList)
{
    if (m_doGrab) {
        ungrabKey();
    }
    m_triggersList = triggersList;

    if (m_doGrab) {
        grabKey();
    }
}

void App::setDoGrab(bool doGrab)
{
    if (m_doGrab != doGrab) {;
        if (doGrab) {
            grabKey();
        } else {
            ungrabKey();
        }
        m_doGrab = doGrab;
    }
}

void App::grabKey()
{
    Q_FOREACH(const TriggerKey& key, m_triggersList) {
        KeySym sym = key.first;
        uint modifiers = key.second;
        Display* xdisplay = QX11Info::display();
        KeyCode keycode = XKeysymToKeycode (xdisplay, (gulong) sym);
        if (keycode == 0) {
            g_warning ("Can not convert keyval=%lu to keycode!", sym);
        }
        XGrabKey(QX11Info::display(), keycode, modifiers, QX11Info::appRootWindow(), True, GrabModeAsync, GrabModeAsync);
    }
}

void App::ungrabKey()
{
    Q_FOREACH(const TriggerKey& key, m_triggersList) {
        KeySym sym = key.first;
        uint modifiers = key.second;
        Display* xdisplay = QX11Info::display();
        KeyCode keycode = XKeysymToKeycode (xdisplay, (gulong) sym);
        if (keycode == 0) {
            g_warning ("Can not convert keyval=%lu to keycode!", sym);
        }
        XUngrabKey(QX11Info::display(), keycode, modifiers, QX11Info::appRootWindow());
    }
}

void App::finalize()
{
    clean();
    App::exit(0);
}

void App::clean()
{
    if (m_impanel) {
        g_object_unref(m_impanel);
        m_impanel = 0;
    }

    if (m_bus) {
        g_signal_handlers_disconnect_by_func(m_bus, (gpointer) ibus_disconnected_cb, this);
        g_object_unref(m_bus);
        m_bus = 0;
    }
    ungrabKey();
}

App::~App()
{
    clean();
}
