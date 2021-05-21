/*
    SPDX-FileCopyrightText: 2013-2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "app.h"
#include "gdkkeysyms_p.h"
#include "gtkaccelparse_p.h"
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QTimer>
#include <QX11Info>

#define USED_MASK (XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_CONTROL | XCB_MOD_MASK_1 | XCB_MOD_MASK_4)

bool XcbEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long int *result)
{
    Q_UNUSED(result);
    if (eventType != "xcb_generic_event_t") {
        return false;
    }

    return qobject_cast<App *>(qApp)->nativeEvent(static_cast<xcb_generic_event_t *>(message));
}

// callback functions from glib code
static void name_acquired_cb(GDBusConnection *connection,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer self)
{
    Q_UNUSED(connection);
    Q_UNUSED(sender_name);
    Q_UNUSED(object_path);
    Q_UNUSED(interface_name);
    Q_UNUSED(signal_name);
    Q_UNUSED(parameters);
    App *app = (App *)self;
    app->nameAcquired();
}

static void name_lost_cb(GDBusConnection *connection,
                         const gchar *sender_name,
                         const gchar *object_path,
                         const gchar *interface_name,
                         const gchar *signal_name,
                         GVariant *parameters,
                         gpointer self)
{
    Q_UNUSED(connection);
    Q_UNUSED(sender_name);
    Q_UNUSED(object_path);
    Q_UNUSED(interface_name);
    Q_UNUSED(signal_name);
    Q_UNUSED(parameters);
    App *app = (App *)self;
    app->nameLost();
}

static void ibus_connected_cb(IBusBus *m_bus, gpointer user_data)
{
    Q_UNUSED(m_bus);
    App *app = (App *)user_data;
    app->init();
}

static void ibus_disconnected_cb(IBusBus *m_bus, gpointer user_data)
{
    Q_UNUSED(m_bus);
    App *app = (App *)user_data;
    app->finalize();
}

static void initIconMap(QMap<QByteArray, QByteArray> &iconMap)
{
    iconMap["gtk-about"] = "help-about";
    iconMap["gtk-add"] = "list-add";
    iconMap["gtk-bold"] = "format-text-bold";
    iconMap["gtk-cdrom"] = "media-optical";
    iconMap["gtk-clear"] = "edit-clear";
    iconMap["gtk-close"] = "window-close";
    iconMap["gtk-copy"] = "edit-copy";
    iconMap["gtk-cut"] = "edit-cut";
    iconMap["gtk-delete"] = "edit-delete";
    iconMap["gtk-dialog-authentication"] = "dialog-password";
    iconMap["gtk-dialog-info"] = "dialog-information";
    iconMap["gtk-dialog-warning"] = "dialog-warning";
    iconMap["gtk-dialog-error"] = "dialog-error";
    iconMap["gtk-dialog-question"] = "dialog-question";
    iconMap["gtk-directory"] = "folder";
    iconMap["gtk-execute"] = "system-run";
    iconMap["gtk-file"] = "text-x-generic";
    iconMap["gtk-find"] = "edit-find";
    iconMap["gtk-find-and-replace"] = "edit-find-replace";
    iconMap["gtk-floppy"] = "media-floppy";
    iconMap["gtk-fullscreen"] = "view-fullscreen";
    iconMap["gtk-goto-bottom"] = "go-bottom";
    iconMap["gtk-goto-first"] = "go-first";
    iconMap["gtk-goto-last"] = "go-last";
    iconMap["gtk-goto-top"] = "go-top";
    iconMap["gtk-go-back"] = "go-previous";
    iconMap["gtk-go-down"] = "go-down";
    iconMap["gtk-go-forward"] = "go-next";
    iconMap["gtk-go-up"] = "go-up";
    iconMap["gtk-harddisk"] = "drive-harddisk";
    iconMap["gtk-help"] = "help-browser";
    iconMap["gtk-home"] = "go-home";
    iconMap["gtk-indent"] = "format-indent-more";
    iconMap["gtk-info"] = "dialog-information";
    iconMap["gtk-italic"] = "format-text-italic";
    iconMap["gtk-jump-to"] = "go-jump";
    iconMap["gtk-justify-center"] = "format-justify-center";
    iconMap["gtk-justify-fill"] = "format-justify-fill";
    iconMap["gtk-justify-left"] = "format-justify-left";
    iconMap["gtk-justify-right"] = "format-justify-right";
    iconMap["gtk-leave-fullscreen"] = "view-restore";
    iconMap["gtk-missing-image"] = "image-missing";
    iconMap["gtk-media-forward"] = "media-seek-forward";
    iconMap["gtk-media-next"] = "media-skip-forward";
    iconMap["gtk-media-pause"] = "media-playback-pause";
    iconMap["gtk-media-play"] = "media-playback-start";
    iconMap["gtk-media-previous"] = "media-skip-backward";
    iconMap["gtk-media-record"] = "media-record";
    iconMap["gtk-media-rewind"] = "media-seek-backward";
    iconMap["gtk-media-stop"] = "media-playback-stop";
    iconMap["gtk-network"] = "network-workgroup";
    iconMap["gtk-new"] = "document-new";
    iconMap["gtk-open"] = "document-open";
    iconMap["gtk-page-setup"] = "document-page-setup";
    iconMap["gtk-paste"] = "edit-paste";
    iconMap["gtk-preferences"] = "preferences-system";
    iconMap["gtk-print"] = "document-print";
    iconMap["gtk-print-error"] = "printer-error";
    iconMap["gtk-properties"] = "document-properties";
    iconMap["gtk-quit"] = "application-exit";
    iconMap["gtk-redo"] = "edit-redo";
    iconMap["gtk-refresh"] = "view-refresh";
    iconMap["gtk-remove"] = "list-remove";
    iconMap["gtk-revert-to-saved"] = "document-revert";
    iconMap["gtk-save"] = "document-save";
    iconMap["gtk-save-as"] = "document-save-as";
    iconMap["gtk-select-all"] = "edit-select-all";
    iconMap["gtk-sort-ascending"] = "view-sort-ascending";
    iconMap["gtk-sort-descending"] = "view-sort-descending";
    iconMap["gtk-spell-check"] = "tools-check-spelling";
    iconMap["gtk-stop"] = "process-stop";
    iconMap["gtk-strikethrough"] = "format-text-strikethrough";
    iconMap["gtk-underline"] = "format-text-underline";
    iconMap["gtk-undo"] = "edit-undo";
    iconMap["gtk-unindent"] = "format-indent-less";
    iconMap["gtk-zoom-100"] = "zoom-original";
    iconMap["gtk-zoom-fit"] = "zoom-fit-best";
    iconMap["gtk-zoom-in"] = "zoom-in";
    iconMap["gtk-zoom-out"] = "zoom-out";
}

App::App(int &argc, char *argv[])
    : QGuiApplication(argc, argv)
    , m_eventFilter(new XcbEventFilter)
    , m_init(false)
    , m_bus(ibus_bus_new())
    , m_impanel(nullptr)
    , m_keyboardGrabbed(false)
    , m_doGrab(false)
    , m_syms(nullptr)
    , m_watcher(new QDBusServiceWatcher(this))
{
    m_syms = xcb_key_symbols_alloc(QX11Info::connection());
    installNativeEventFilter(m_eventFilter.data());

    initIconMap(m_iconMap);
    m_watcher->setConnection(QDBusConnection::sessionBus());
    m_watcher->addWatchedService(QStringLiteral("org.kde.impanel"));
    init();
}

uint App::getPrimaryModifier(uint state)
{
    const GdkModifierType masks[] = {GDK_MOD5_MASK, GDK_MOD4_MASK, GDK_MOD3_MASK, GDK_MOD2_MASK, GDK_MOD1_MASK, GDK_CONTROL_MASK, GDK_LOCK_MASK, GDK_LOCK_MASK};
    for (size_t i = 0; i < sizeof(masks) / sizeof(masks[0]); i++) {
        GdkModifierType mask = masks[i];
        if ((state & mask) == mask)
            return mask;
    }
    return 0;
}

bool App::nativeEvent(xcb_generic_event_t *event)
{
    if ((event->response_type & ~0x80) == XCB_KEY_PRESS) {
        auto keypress = reinterpret_cast<xcb_key_press_event_t *>(event);
        if (keypress->event == QX11Info::appRootWindow()) {
            auto sym = xcb_key_press_lookup_keysym(m_syms, keypress, 0);
            uint state = keypress->state & USED_MASK;
            bool forward;
            if ((forward = m_triggersList.contains(qMakePair<uint, uint>(sym, state)))
                || m_triggersList.contains(qMakePair<uint, uint>(sym, state & (~XCB_MOD_MASK_SHIFT)))) {
                if (m_keyboardGrabbed) {
                    ibus_panel_impanel_navigate(m_impanel, false, forward);
                } else {
                    if (grabXKeyboard()) {
                        ibus_panel_impanel_navigate(m_impanel, true, forward);
                    } else {
                        ibus_panel_impanel_move_next(m_impanel);
                    }
                }
            }
        }
    } else if ((event->response_type & ~0x80) == XCB_KEY_RELEASE) {
        auto keyrelease = reinterpret_cast<xcb_key_release_event_t *>(event);
        if (keyrelease->event == QX11Info::appRootWindow()) {
            keyRelease(keyrelease);
        }
    }
    return false;
}

void App::keyRelease(const xcb_key_release_event_t *event)
{
    unsigned int mk = event->state & USED_MASK;
    // ev.state is state before the key release, so just checking mk being 0 isn't enough
    // using XQueryPointer() also doesn't seem to work well, so the check that all
    // modifiers are released: only one modifier is active and the currently released
    // key is this modifier - if yes, release the grab
    int mod_index = -1;
    for (int i = XCB_MAP_INDEX_SHIFT; i <= XCB_MAP_INDEX_5; ++i)
        if ((mk & (1 << i)) != 0) {
            if (mod_index >= 0)
                return;
            mod_index = i;
        }
    bool release = false;
    if (mod_index == -1)
        release = true;
    else {
        auto cookie = xcb_get_modifier_mapping(QX11Info::connection());
        auto reply = xcb_get_modifier_mapping_reply(QX11Info::connection(), cookie, nullptr);
        if (reply) {
            auto keycodes = xcb_get_modifier_mapping_keycodes(reply);
            for (int i = 0; i < reply->keycodes_per_modifier; i++) {
                if (keycodes[reply->keycodes_per_modifier * mod_index + i] == event->detail) {
                    release = true;
                }
            }
        }
        free(reply);
    }
    if (!release) {
        return;
    }
    if (m_keyboardGrabbed) {
        accept();
    }
}

void App::init()
{
    // only init once
    if (m_init) {
        return;
    }
    if (!ibus_bus_is_connected(m_bus)) {
        return;
    }
    g_signal_connect(m_bus, "connected", G_CALLBACK(ibus_connected_cb), this);
    g_signal_connect(m_bus, "disconnected", G_CALLBACK(ibus_disconnected_cb), this);
    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, &App::finalize);
    GDBusConnection *connection = ibus_bus_get_connection(m_bus);
    g_dbus_connection_signal_subscribe(connection,
                                       "org.freedesktop.DBus",
                                       "org.freedesktop.DBus",
                                       "NameAcquired",
                                       "/org/freedesktop/DBus",
                                       IBUS_SERVICE_PANEL,
                                       G_DBUS_SIGNAL_FLAGS_NONE,
                                       name_acquired_cb,
                                       this,
                                       nullptr);

    g_dbus_connection_signal_subscribe(connection,
                                       "org.freedesktop.DBus",
                                       "org.freedesktop.DBus",
                                       "NameLost",
                                       "/org/freedesktop/DBus",
                                       IBUS_SERVICE_PANEL,
                                       G_DBUS_SIGNAL_FLAGS_NONE,
                                       name_lost_cb,
                                       this,
                                       nullptr);

    ibus_bus_request_name(m_bus, IBUS_SERVICE_PANEL, IBUS_BUS_NAME_FLAG_ALLOW_REPLACEMENT | IBUS_BUS_NAME_FLAG_REPLACE_EXISTING);
    m_init = true;
}

void App::nameAcquired()
{
    if (m_impanel) {
        g_object_unref(m_impanel);
    }
    m_impanel = ibus_panel_impanel_new(ibus_bus_get_connection(m_bus));
    ibus_panel_impanel_set_bus(m_impanel, m_bus);
    ibus_panel_impanel_set_app(m_impanel, this);
}

void App::nameLost()
{
    setDoGrab(false);
    if (m_impanel) {
        g_object_unref(m_impanel);
    }
    m_impanel = nullptr;
}

QByteArray App::normalizeIconName(const QByteArray &icon) const
{
    if (m_iconMap.contains(icon)) {
        return m_iconMap[icon];
    }

    return icon;
}

void App::setTriggerKeys(QList<TriggerKey> triggersList)
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
    if (m_doGrab != doGrab) {
        ;
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
    Q_FOREACH (const TriggerKey &key, m_triggersList) {
        xcb_keysym_t sym = key.first;
        uint modifiers = key.second;
        xcb_keycode_t *keycode = xcb_key_symbols_get_keycode(m_syms, sym);
        if (!keycode) {
            g_warning("Can not convert keyval=%u to keycode!", sym);
        } else {
            xcb_grab_key(QX11Info::connection(), true, QX11Info::appRootWindow(), modifiers, keycode[0], XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
            if ((modifiers & XCB_MOD_MASK_SHIFT) == 0) {
                xcb_grab_key(QX11Info::connection(),
                             true,
                             QX11Info::appRootWindow(),
                             modifiers | XCB_MOD_MASK_SHIFT,
                             keycode[0],
                             XCB_GRAB_MODE_ASYNC,
                             XCB_GRAB_MODE_ASYNC);
            }
        }
        free(keycode);
    }
}

void App::ungrabKey()
{
    Q_FOREACH (const TriggerKey &key, m_triggersList) {
        xcb_keysym_t sym = key.first;
        uint modifiers = key.second;
        xcb_keycode_t *keycode = xcb_key_symbols_get_keycode(m_syms, sym);
        if (!keycode) {
            g_warning("Can not convert keyval=%u to keycode!", sym);
        } else {
            xcb_ungrab_key(QX11Info::connection(), keycode[0], QX11Info::appRootWindow(), modifiers);
            if ((modifiers & XCB_MOD_MASK_SHIFT) == 0) {
                xcb_ungrab_key(QX11Info::connection(), keycode[0], QX11Info::appRootWindow(), modifiers | XCB_MOD_MASK_SHIFT);
            }
        }
        free(keycode);
    }
}

bool App::grabXKeyboard()
{
    if (m_keyboardGrabbed)
        return false;
    auto w = QX11Info::appRootWindow();
    auto cookie = xcb_grab_keyboard(QX11Info::connection(), false, w, XCB_CURRENT_TIME, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    auto reply = xcb_grab_keyboard_reply(QX11Info::connection(), cookie, nullptr);

    if (reply && reply->status == XCB_GRAB_STATUS_SUCCESS) {
        m_keyboardGrabbed = true;
    }
    free(reply);
    return m_keyboardGrabbed;
}

void App::ungrabXKeyboard()
{
    if (!m_keyboardGrabbed) {
        // grabXKeyboard() may fail sometimes, so don't fail, but at least warn anyway
        qDebug() << "ungrabXKeyboard() called but keyboard not grabbed!";
    }
    m_keyboardGrabbed = false;
    xcb_ungrab_keyboard(QX11Info::connection(), XCB_CURRENT_TIME);
}

void App::accept()
{
    if (m_keyboardGrabbed) {
        ungrabXKeyboard();
    }

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
        m_impanel = nullptr;
    }

    if (m_bus) {
        g_signal_handlers_disconnect_by_func(m_bus, (gpointer)ibus_disconnected_cb, this);
        g_signal_handlers_disconnect_by_func(m_bus, (gpointer)ibus_connected_cb, this);
        g_object_unref(m_bus);
        m_bus = nullptr;
    }
    ungrabKey();
}

App::~App()
{
    clean();
    if (m_syms) {
        xcb_key_symbols_free(m_syms);
    }
}
