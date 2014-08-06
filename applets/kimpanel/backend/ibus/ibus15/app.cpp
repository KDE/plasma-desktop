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

App::App(int argc, char** argv): QApplication(argc, argv)
    ,m_bus(0)
    ,m_impanel(0)
    ,m_keyboardGrabbed(false)
    ,m_doGrab(false)
{
    QTimer::singleShot(0, this, SLOT(init()));
}

uint App::getPrimaryModifier(uint state)
{
    const GdkModifierType masks[] = {
        GDK_MOD5_MASK,
        GDK_MOD4_MASK,
        GDK_MOD3_MASK,
        GDK_MOD2_MASK,
        GDK_MOD1_MASK,
        GDK_CONTROL_MASK,
        GDK_LOCK_MASK,
        GDK_LOCK_MASK
    };
    for (size_t i = 0; i < sizeof(masks) / sizeof(masks[0]); i++) {
        GdkModifierType mask = masks[i];
        if ((state & mask) == mask)
            return mask;
    }
    return 0;
}

bool App::x11EventFilter(XEvent* event)
{
    if (event->xany.window == QX11Info::appRootWindow()) {
        if (event->type == KeyPress) {
            KeySym sym = XKeycodeToKeysym(QX11Info::display(), event->xkey.keycode, 0);
            uint state = event->xkey.state & USED_MASK;
            if (m_triggersList.contains(qMakePair<uint, uint>(sym, state))) {
                if (m_keyboardGrabbed) {
                    ibus_panel_impanel_navigate(m_impanel, false);
                } else {
                    if (grabXKeyboard()) {
                        ibus_panel_impanel_navigate(m_impanel, true);
                    } else {
                        ibus_panel_impanel_move_next(m_impanel);
                    }
                }
            }
        } else if (event->type == KeyRelease) {
            keyRelease(*event);
        }
    }
    return QApplication::x11EventFilter(event);
}

void App::keyRelease(const XEvent& event)
{
    const XKeyEvent& ev = event.xkey;

    unsigned int mk = ev.state &
                      (ShiftMask |
                       ControlMask |
                       Mod1Mask |
                       Mod4Mask);
    // ev.state is state before the key release, so just checking mk being 0 isn't enough
    // using XQueryPointer() also doesn't seem to work well, so the check that all
    // modifiers are released: only one modifier is active and the currently released
    // key is this modifier - if yes, release the grab
    int mod_index = -1;
    for (int i = ShiftMapIndex;
            i <= Mod5MapIndex;
            ++i)
        if ((mk & (1 << i)) != 0) {
            if (mod_index >= 0)
                return;
            mod_index = i;
        }
    bool release = false;
    if (mod_index == -1)
        release = true;
    else {
        XModifierKeymap* xmk = XGetModifierMapping(QX11Info::display());
        for (int i = 0; i < xmk->max_keypermod; i++)
            if (xmk->modifiermap[xmk->max_keypermod * mod_index + i]
                    == ev.keycode)
                release = true;
        XFreeModifiermap(xmk);
    }
    if (!release)
        return;
    if (m_keyboardGrabbed) {
        accept();
    }
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

bool App::grabXKeyboard() {
    if (QWidget::keyboardGrabber() != NULL)
        return false;
    if (m_keyboardGrabbed)
        return false;
    if (activePopupWidget() != NULL)
        return false;
    Qt::HANDLE w = QX11Info::appRootWindow();
    if (XGrabKeyboard(QX11Info::display(), w, False,
    GrabModeAsync, GrabModeAsync, QX11Info::appTime()) != GrabSuccess)
        return false;
    m_keyboardGrabbed = true;
    return true;
}

void App::ungrabXKeyboard()
{
    if (!m_keyboardGrabbed) {
        // grabXKeyboard() may fail sometimes, so don't fail, but at least warn anyway
        qDebug() << "ungrabXKeyboard() called but keyboard not grabbed!";
    }
    m_keyboardGrabbed = false;
    XUngrabKeyboard(QX11Info::display(), CurrentTime);
}

void App::accept()
{
    if (m_keyboardGrabbed)
        ungrabXKeyboard();

    ibus_panel_impanel_accept(m_impanel);
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
