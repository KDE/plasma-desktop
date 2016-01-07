/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011-2012 Ni Hui <shuizhuyuanluo@126.com>
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

#include <stdlib.h>
#include <string.h>
#include <ibus.h>
#if !IBUS_CHECK_VERSION(1,3,99)
#include <gio/gio.h>
#include <ibuspanelservice.h>
#endif
#include "panel.h"

#ifndef DBUS_ERROR_FAILED
#define DBUS_ERROR_FAILED "org.freedesktop.DBus.Error.Failed"
#endif /* DBUS_ERROR_FAILED */
#define _UNUSED(x) ((void) x)

typedef struct _IBusPanelImpanelClass IBusPanelImpanelClass;

struct _IBusPanelImpanel {
    IBusPanelService    parent;
    IBusBus            *bus;
    GDBusConnection    *conn;
    IBusInputContext   *input_context;
    IBusProperty       *logo_prop;
    IBusProperty       *about_prop;
};

struct _IBusPanelImpanelClass {
    IBusPanelServiceClass parent;
};

void
ibus_panel_impanel_set_bus (IBusPanelImpanel *impanel,
                            IBusBus          *bus)
{
    impanel->bus = bus;
}

static GDBusNodeInfo *introspection_data = NULL;

static guint owner_id;

static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='org.kde.kimpanel.inputmethod'>"
    "    <signal name='Enable'>"
    "      <arg type='b' name='enable'/>"
    "    </signal>"
    "    <signal name='RegisterProperties'>"
    "      <arg type='as' name='prop'/>"
    "    </signal>"
    "    <signal name='UpdateProperty'>"
    "      <arg type='s' name='prop'/>"
    "    </signal>"
    "    <signal name='RemoveProperty'>"
    "      <arg type='s' name='prop'/>"
    "    </signal>"
    "    <signal name='ShowAux'>"
    "      <arg type='b' name='toshow'/>"
    "    </signal>"
    "    <signal name='ShowLookupTable'>"
    "      <arg type='b' name='toshow'/>"
    "    </signal>"
    "    <signal name='ShowPreedit'>"
    "      <arg type='b' name='toshow'/>"
    "    </signal>"
    "    <signal name='UpdateAux'>"
    "      <arg type='s' name='text'/>"
    "      <arg type='s' name='attr'/>"
    "    </signal>"
    "    <signal name='UpdateLookupTableCursor'>"
    "      <arg type='i' name='pos'/>"
    "    </signal>"
    "    <signal name='UpdateLookupTable'>"
    "      <arg type='as' name='labels'/>"
    "      <arg type='as' name='candidates'/>"
    "      <arg type='as' name='attrs'/>"
    "      <arg type='b' name='hasprev'/>"
    "      <arg type='b' name='hasnext'/>"
    "    </signal>"
    "    <signal name='UpdatePreeditCaret'>"
    "      <arg type='i' name='pos'/>"
    "    </signal>"
    "    <signal name='UpdatePreeditText'>"
    "      <arg type='s' name='text'/>"
    "      <arg type='s' name='attr'/>"
    "    </signal>"
    "    <signal name='UpdateSpotLocation'>"
    "      <arg type='i' name='x'/>"
    "      <arg type='i' name='y'/>"
    "    </signal>"
    "    <signal name='ExecDialog'>"
    "      <arg type='s' name='prop'/>"
    "    </signal>"
    "    <signal name='ExecMenu'>"
    "      <arg type='as' name='actions'/>"
    "    </signal>"
    "  </interface>"
    "</node>";

/* functions prototype */
static void         ibus_panel_impanel_class_init               (IBusPanelImpanelClass  *class);
static void         ibus_panel_impanel_init                     (IBusPanelImpanel       *impanel);
static void         ibus_panel_impanel_destroy                  (IBusPanelImpanel       *impanel);
#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean     ibus_panel_impanel_focus_in                 (IBusPanelService       *panel,
                                                                 const gchar            *input_context_path,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_focus_out                (IBusPanelService       *panel,
                                                                 const gchar            *input_context_path,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_register_properties      (IBusPanelService       *panel,
                                                                 IBusPropList           *prop_list,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_set_cursor_location      (IBusPanelService       *panel,
                                                                 gint                    x,
                                                                 gint                    y,
                                                                 gint                    w,
                                                                 gint                    h,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_update_auxiliary_text    (IBusPanelService       *panel,
                                                                 IBusText               *text,
                                                                 gboolean                visible,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_update_lookup_table      (IBusPanelService       *panel,
                                                                 IBusLookupTable        *lookup_table,
                                                                 gboolean                visible,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_update_preedit_text      (IBusPanelService       *panel,
                                                                 IBusText               *text,
                                                                 guint                   cursor_pos,
                                                                 gboolean                visible,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_update_property          (IBusPanelService       *panel,
                                                                 IBusProperty           *prop,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_cursor_down_lookup_table (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_cursor_up_lookup_table   (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_hide_auxiliary_text      (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_hide_language_bar        (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_hide_lookup_table        (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_hide_preedit_text        (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_page_down_lookup_table   (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_page_up_lookup_table     (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_reset                    (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_show_auxiliary_text      (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_show_language_bar        (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_show_lookup_table        (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_show_preedit_text        (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_start_setup              (IBusPanelService       *panel,
                                                                 IBusError             **error);
static gboolean     ibus_panel_impanel_state_changed            (IBusPanelService       *panel,
                                                                 IBusError             **error);
#else
static void         ibus_panel_impanel_focus_in                 (IBusPanelService       *panel,
                                                                 const gchar            *input_context_path);
static void         ibus_panel_impanel_focus_out                (IBusPanelService       *panel,
                                                                 const gchar            *input_context_path);
static void         ibus_panel_impanel_register_properties      (IBusPanelService       *panel,
                                                                 IBusPropList           *prop_list);
static void         ibus_panel_impanel_set_cursor_location      (IBusPanelService       *panel,
                                                                 gint                    x,
                                                                 gint                    y,
                                                                 gint                    w,
                                                                 gint                    h);
static void         ibus_panel_impanel_update_auxiliary_text    (IBusPanelService       *panel,
                                                                 IBusText               *text,
                                                                 gboolean                visible);
static void         ibus_panel_impanel_update_lookup_table      (IBusPanelService       *panel,
                                                                 IBusLookupTable        *lookup_table,
                                                                 gboolean                visible);
static void         ibus_panel_impanel_update_preedit_text      (IBusPanelService       *panel,
                                                                 IBusText               *text,
                                                                 guint                   cursor_pos,
                                                                 gboolean                visible);
static void         ibus_panel_impanel_update_property          (IBusPanelService       *panel,
                                                                 IBusProperty           *prop);
static void         ibus_panel_impanel_cursor_down_lookup_table (IBusPanelService       *panel);
static void         ibus_panel_impanel_cursor_up_lookup_table   (IBusPanelService       *panel);
static void         ibus_panel_impanel_hide_auxiliary_text      (IBusPanelService       *panel);
static void         ibus_panel_impanel_hide_language_bar        (IBusPanelService       *panel);
static void         ibus_panel_impanel_hide_lookup_table        (IBusPanelService       *panel);
static void         ibus_panel_impanel_hide_preedit_text        (IBusPanelService       *panel);
static void         ibus_panel_impanel_page_down_lookup_table   (IBusPanelService       *panel);
static void         ibus_panel_impanel_page_up_lookup_table     (IBusPanelService       *panel);
static void         ibus_panel_impanel_reset                    (IBusPanelService       *panel);
static void         ibus_panel_impanel_show_auxiliary_text      (IBusPanelService       *panel);
static void         ibus_panel_impanel_show_language_bar        (IBusPanelService       *panel);
static void         ibus_panel_impanel_show_lookup_table        (IBusPanelService       *panel);
static void         ibus_panel_impanel_show_preedit_text        (IBusPanelService       *panel);
static void         ibus_panel_impanel_start_setup              (IBusPanelService       *panel);
static void         ibus_panel_impanel_state_changed            (IBusPanelService       *panel);
#endif

/* impanel signal handler function */
static void         ibus_panel_impanel_exec_dialog              (IBusPanelService       *panel);
static void         ibus_panel_impanel_exec_menu                (IBusPanelService       *panel);

#if !IBUS_CHECK_VERSION(1,3,99)
static IBusPanelServiceClass *parent_class = NULL;
#endif

static const char prop_sep[] = ":";

static void
ibus_property_args_to_propstr (const char *key,
                               const char *label,
                               const char *icon,
                               const char *tooltip,
                               char *propstr)
{
    static const char pre[] = "/IBus/";
    propstr[0] = '\0';
    strcat(propstr, pre);
    strcat(propstr, key);
    strcat(propstr, prop_sep);
    strcat(propstr, label);
    strcat(propstr, prop_sep);
    strcat(propstr, icon);
    strcat(propstr, prop_sep);
    strcat(propstr, tooltip);
}

static void
ibus_property_to_propstr (IBusProperty *property,
                          char *propstr)
{
#if !IBUS_CHECK_VERSION(1,3,99)
    ibus_property_args_to_propstr(property->key,
                                  property->label->text,
                                  property->icon,
                                  property->tooltip->text,
                                  propstr);
#else
    ibus_property_args_to_propstr(ibus_property_get_key (property),
                                  ibus_text_get_text (ibus_property_get_label (property)),
                                  ibus_property_get_icon (property),
                                  ibus_text_get_text (ibus_property_get_tooltip (property)),
                                  propstr);
#endif
}

static void
ibus_engine_desc_args_to_propstr (const char *name,
                                  const char *language,
                                  const char *longname,
                                  const char *icon,
                                  const char *description,
                                  char *propstr)
{
    static const char pre[] = "/IBus/Engine/";
    propstr[0] = '\0';
    strcat(propstr, pre);
    strcat(propstr, name);
    strcat(propstr, prop_sep);
    if (language) {
        strcat(propstr, language);
        strcat(propstr, " - ");
    }
    strcat(propstr, longname);
    strcat(propstr, prop_sep);
    strcat(propstr, icon);
    strcat(propstr, prop_sep);
    strcat(propstr, description);
}

static void
ibus_engine_desc_to_propstr (IBusEngineDesc *engine_desc,
                             char *propstr)
{
#if !IBUS_CHECK_VERSION(1,3,99)
    ibus_engine_desc_args_to_propstr(engine_desc->name,
                                     engine_desc->language,
                                     engine_desc->longname,
                                     engine_desc->icon,
                                     engine_desc->description,
                                     propstr);
#else
    ibus_engine_desc_args_to_propstr(ibus_engine_desc_get_name(engine_desc),
                                     ibus_engine_desc_get_language(engine_desc),
                                     ibus_engine_desc_get_longname(engine_desc),
                                     ibus_engine_desc_get_icon(engine_desc),
                                     ibus_engine_desc_get_description(engine_desc),
                                     propstr);
#endif
}

static void
impanel_trigger_property_callback (GDBusConnection *connection,
                                   const gchar     *sender_name,
                                   const gchar     *object_path,
                                   const gchar     *interface_name,
                                   const gchar     *signal_name,
                                   GVariant        *parameters,
                                   gpointer         user_data)
{
    _UNUSED(connection);
    _UNUSED(sender_name);
    _UNUSED(object_path);
    _UNUSED(interface_name);
    _UNUSED(signal_name);
    gchar *s0;
    g_variant_get (parameters, "(s)", &s0);
    gchar* prop_key = s0 + 6;// +6 to skip "/IBus/"
    if (g_ascii_strncasecmp (prop_key, "Logo", 4) == 0)
        ibus_panel_impanel_exec_menu((IBusPanelService *)user_data);
    else if (g_ascii_strncasecmp (prop_key, "About", 5) == 0)
        ibus_panel_impanel_exec_dialog((IBusPanelService *)user_data);
    else if (g_ascii_strncasecmp (prop_key, "Engine/", 7) == 0) {
        prop_key += 7;// +7 to skip "Engine/"
        ibus_input_context_set_engine(((IBusPanelImpanel *)user_data)->input_context, prop_key);
    }
    else
#if !IBUS_CHECK_VERSION(1,3,99)
        ibus_panel_service_property_active((IBusPanelService *)user_data, prop_key, PROP_STATE_CHECKED);
#else
        ibus_panel_service_property_activate((IBusPanelService *)user_data, prop_key, PROP_STATE_CHECKED);
#endif
}

static void
impanel_configure_callback (GDBusConnection *connection,
                            const gchar     *sender_name,
                            const gchar     *object_path,
                            const gchar     *interface_name,
                            const gchar     *signal_name,
                            GVariant        *parameters,
                            gpointer         user_data)
{
    _UNUSED(connection);
    _UNUSED(sender_name);
    _UNUSED(object_path);
    _UNUSED(interface_name);
    _UNUSED(signal_name);
    _UNUSED(parameters);
    _UNUSED(user_data);
    pid_t pid = fork();
    if (pid == 0) {
        execlp ("ibus-setup", "ibus-setup", (char *)0);
        exit (0);
    }
}

static void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
    _UNUSED(name);
    ((IBusPanelImpanel *)user_data)->conn = connection;

    g_dbus_connection_register_object (connection,
                                       "/kimpanel",
                                       introspection_data->interfaces[0],
                                       NULL,  /*&interface_vtable*/
                                       NULL,  /* user_data */
                                       NULL,  /* user_data_free_func */
                                       NULL); /* GError** */

    g_dbus_connection_signal_subscribe (connection,
                                        "org.kde.impanel",
                                        "org.kde.impanel",
                                        "TriggerProperty",
                                        "/org/kde/impanel",
                                        NULL,
                                        G_DBUS_SIGNAL_FLAGS_NONE,
                                        impanel_trigger_property_callback,
                                        user_data,
                                        NULL);
    g_dbus_connection_signal_subscribe (connection,
                                        "org.kde.impanel",
                                        "org.kde.impanel",
                                        "Configure",
                                        "/org/kde/impanel",
                                        NULL,
                                        G_DBUS_SIGNAL_FLAGS_NONE,
                                        impanel_configure_callback,
                                        user_data,
                                        NULL);
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
    _UNUSED(connection);
    _UNUSED(name);
    _UNUSED(user_data);
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
    _UNUSED(connection);
    _UNUSED(name);
    _UNUSED(user_data);
    exit (1);
}

#if !IBUS_CHECK_VERSION(1,3,99)
GType
ibus_panel_impanel_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusPanelImpanelClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_panel_impanel_class_init,
        (GClassFinalizeFunc)NULL,
        NULL,
        sizeof (IBusPanelImpanel),
        0,
        (GInstanceInitFunc) ibus_panel_impanel_init,
        NULL
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_PANEL_SERVICE,
                                       "IBusPanelImpanel",
                                       &type_info,
                                       (GTypeFlags) 0);
    }

    return type;
}
#else
G_DEFINE_TYPE (IBusPanelImpanel, ibus_panel_impanel, IBUS_TYPE_PANEL_SERVICE)
#endif

static void
ibus_panel_impanel_class_init (IBusPanelImpanelClass *class)
{
    GObjectClass *object_class = G_OBJECT_CLASS (class);

#if !IBUS_CHECK_VERSION(1,3,99)
    parent_class = (IBusPanelServiceClass *) g_type_class_peek_parent (class);
#endif

    IBUS_OBJECT_CLASS (object_class)->destroy = (IBusObjectDestroyFunc) ibus_panel_impanel_destroy;
    IBUS_PANEL_SERVICE_CLASS (object_class)->focus_in                   = ibus_panel_impanel_focus_in;
    IBUS_PANEL_SERVICE_CLASS (object_class)->focus_out                  = ibus_panel_impanel_focus_out;
    IBUS_PANEL_SERVICE_CLASS (object_class)->register_properties        = ibus_panel_impanel_register_properties;
    IBUS_PANEL_SERVICE_CLASS (object_class)->set_cursor_location        = ibus_panel_impanel_set_cursor_location;
    IBUS_PANEL_SERVICE_CLASS (object_class)->update_auxiliary_text      = ibus_panel_impanel_update_auxiliary_text;
    IBUS_PANEL_SERVICE_CLASS (object_class)->update_lookup_table        = ibus_panel_impanel_update_lookup_table;
    IBUS_PANEL_SERVICE_CLASS (object_class)->update_preedit_text        = ibus_panel_impanel_update_preedit_text;
    IBUS_PANEL_SERVICE_CLASS (object_class)->update_property            = ibus_panel_impanel_update_property;
    IBUS_PANEL_SERVICE_CLASS (object_class)->cursor_down_lookup_table   = ibus_panel_impanel_cursor_down_lookup_table;
    IBUS_PANEL_SERVICE_CLASS (object_class)->cursor_up_lookup_table     = ibus_panel_impanel_cursor_up_lookup_table;
    IBUS_PANEL_SERVICE_CLASS (object_class)->hide_auxiliary_text        = ibus_panel_impanel_hide_auxiliary_text;
    IBUS_PANEL_SERVICE_CLASS (object_class)->hide_language_bar          = ibus_panel_impanel_hide_language_bar;
    IBUS_PANEL_SERVICE_CLASS (object_class)->hide_lookup_table          = ibus_panel_impanel_hide_lookup_table;
    IBUS_PANEL_SERVICE_CLASS (object_class)->hide_preedit_text          = ibus_panel_impanel_hide_preedit_text;
    IBUS_PANEL_SERVICE_CLASS (object_class)->page_down_lookup_table     = ibus_panel_impanel_page_down_lookup_table;
    IBUS_PANEL_SERVICE_CLASS (object_class)->page_up_lookup_table       = ibus_panel_impanel_page_up_lookup_table;
    IBUS_PANEL_SERVICE_CLASS (object_class)->reset                      = ibus_panel_impanel_reset;
    IBUS_PANEL_SERVICE_CLASS (object_class)->show_auxiliary_text        = ibus_panel_impanel_show_auxiliary_text;
    IBUS_PANEL_SERVICE_CLASS (object_class)->show_language_bar          = ibus_panel_impanel_show_language_bar;
    IBUS_PANEL_SERVICE_CLASS (object_class)->show_lookup_table          = ibus_panel_impanel_show_lookup_table;
    IBUS_PANEL_SERVICE_CLASS (object_class)->show_preedit_text          = ibus_panel_impanel_show_preedit_text;
    IBUS_PANEL_SERVICE_CLASS (object_class)->start_setup                = ibus_panel_impanel_start_setup;
    IBUS_PANEL_SERVICE_CLASS (object_class)->state_changed              = ibus_panel_impanel_state_changed;
}

static void
ibus_panel_impanel_init (IBusPanelImpanel *impanel)
{
    impanel->bus = NULL;
    impanel->input_context = NULL;

    introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
    owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                               "org.kde.kimpanel.inputmethod",
                               G_BUS_NAME_OWNER_FLAGS_REPLACE,
                               on_bus_acquired,
                               on_name_acquired,
                               on_name_lost,
                               impanel, NULL);

    // some custom property
    impanel->logo_prop = ibus_property_new ("Logo",
                                            PROP_TYPE_NORMAL,
                                            ibus_text_new_from_string ("IBus"),
                                            "ibus",
                                            ibus_text_new_from_string ("IBus input method"),
                                            FALSE,
                                            FALSE,
                                            PROP_STATE_UNCHECKED,
                                            NULL);

    impanel->about_prop = ibus_property_new ("About",
                                             PROP_TYPE_NORMAL,
                                             ibus_text_new_from_string ("IBus intelligent input bus"),
                                             "ibus-help",
                                             ibus_text_new_from_string ("IBus is an intelligent input bus for Linux/Unix.\n\n"
                                                                        "Huang Peng <shawn.p.huang@gmail.com>"),
                                             FALSE,
                                             FALSE,
                                             PROP_STATE_UNCHECKED,
                                             NULL);
}

static void
ibus_panel_impanel_destroy (IBusPanelImpanel *impanel)
{
    g_object_unref (impanel->logo_prop);
    impanel->logo_prop = NULL;
    g_object_unref (impanel->about_prop);
    impanel->about_prop = NULL;

    g_bus_unown_name (owner_id);
    g_dbus_node_info_unref (introspection_data);

#if !IBUS_CHECK_VERSION(1,3,99)
    IBUS_OBJECT_CLASS (parent_class)->destroy ((IBusObject *)impanel);
#else
    IBUS_OBJECT_CLASS (ibus_panel_impanel_parent_class)->destroy ((IBusObject *)impanel);
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_focus_in (IBusPanelService *panel,
                             const gchar      *input_context_path,
                             IBusError       **error)
#else
static void
ibus_panel_impanel_focus_in (IBusPanelService *panel,
                             const gchar      *input_context_path)
#endif
{
#if !IBUS_CHECK_VERSION(1,3,99)
    IBusConnection *ibusconn;
    _UNUSED(error);
#else
    GDBusConnection *ibusconn;
#endif
    g_object_get (IBUS_PANEL_IMPANEL (panel),
                  "connection", &ibusconn,
                  NULL);

    IBusEngineDesc *engine_desc = ibus_bus_get_global_engine(IBUS_PANEL_IMPANEL (panel)->bus);

    IBusInputContext *ic = ibus_input_context_get_input_context(input_context_path, ibusconn);
    IBUS_PANEL_IMPANEL (panel)->input_context = ic;

    const gchar* icon_name = "ibus-keyboard";
    if (engine_desc) {
#if !IBUS_CHECK_VERSION(1,3,99)
        icon_name = engine_desc->icon;
#else
        icon_name = ibus_engine_desc_get_icon (engine_desc);
#endif
    }

#if !IBUS_CHECK_VERSION(1,3,99)
    ibus_property_set_icon (IBUS_PANEL_IMPANEL (panel)->logo_prop, icon_name);
#else
    ibus_property_set_icon (IBUS_PANEL_IMPANEL (panel)->logo_prop, icon_name);
#endif

    char propstr[512];
    propstr[0] = '\0';

    ibus_property_to_propstr(IBUS_PANEL_IMPANEL (panel)->logo_prop, propstr);

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateProperty",
                                   g_variant_new ("(s)", propstr),
                                   NULL);
#if !IBUS_CHECK_VERSION(1,3,99)
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_focus_out (IBusPanelService *panel,
                              const gchar      *input_context_path,
                              IBusError       **error)
#else
static void
ibus_panel_impanel_focus_out (IBusPanelService *panel,
                              const gchar      *input_context_path)
#endif
{
    _UNUSED(panel);
    _UNUSED(input_context_path);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_register_properties (IBusPanelService *panel,
                                        IBusPropList     *prop_list,
                                        IBusError       **error)
#else
static void
ibus_panel_impanel_register_properties (IBusPanelService *panel,
                                        IBusPropList     *prop_list)
#endif
{
    IBusProperty* property = NULL;
    guint i = 0;
    char propstr[512];

    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));

    ibus_property_to_propstr(IBUS_PANEL_IMPANEL (panel)->logo_prop, propstr);
    g_variant_builder_add (&builder, "s", propstr);

    while ( ( property = ibus_prop_list_get( prop_list, i ) ) != NULL ) {
        ibus_property_to_propstr(property, propstr);
        g_variant_builder_add (&builder, "s", propstr);
        ++i;
    }

    ibus_property_to_propstr(IBUS_PANEL_IMPANEL (panel)->about_prop, propstr);
    g_variant_builder_add (&builder, "s", propstr);

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "RegisterProperties",
                                   g_variant_new ("(as)", &builder),
                                   NULL);

#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_set_cursor_location (IBusPanelService *panel,
                                        gint              x,
                                        gint              y,
                                        gint              w,
                                        gint              h,
                                        IBusError       **error)
#else
static void
ibus_panel_impanel_set_cursor_location (IBusPanelService *panel,
                                        gint              x,
                                        gint              y,
                                        gint              w,
                                        gint              h)
#endif
{

    g_dbus_connection_call(IBUS_PANEL_IMPANEL (panel)->conn,
                            "org.kde.impanel",
                            "/org/kde/impanel",
                            "org.kde.impanel2",
                            "SetSpotRect",
                            g_variant_new("(iiii)", x, y, w, h),
                            NULL,
                            G_DBUS_CALL_FLAGS_NONE,
                           -1,           /* timeout */
                           NULL,
                           NULL,
                           NULL);

#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_update_auxiliary_text (IBusPanelService *panel,
                                          IBusText         *text,
                                          gboolean          visible,
                                          IBusError       **error)
#else
static void
ibus_panel_impanel_update_auxiliary_text (IBusPanelService *panel,
                                          IBusText         *text,
                                          gboolean          visible)
#endif
{
#if !IBUS_CHECK_VERSION(1,3,99)
    const gchar* t = text->text;
#else
    const gchar* t = ibus_text_get_text (text);
#endif
    const gchar *attr = "";

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateAux",
                                   g_variant_new ("(ss)", t, attr),
                                   NULL);

    if (visible == 0)
#if !IBUS_CHECK_VERSION(1,3,99)
        ibus_panel_impanel_hide_auxiliary_text(panel, error);
#else
        ibus_panel_impanel_hide_auxiliary_text(panel);
#endif
    else
#if !IBUS_CHECK_VERSION(1,3,99)
        ibus_panel_impanel_show_auxiliary_text(panel, error);
#else
        ibus_panel_impanel_show_auxiliary_text(panel);
#endif
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_update_lookup_table (IBusPanelService *panel,
                                        IBusLookupTable  *lookup_table,
                                        gboolean          visible,
                                        IBusError       **error)
#else
static void
ibus_panel_impanel_update_lookup_table (IBusPanelService *panel,
                                        IBusLookupTable  *lookup_table,
                                        gboolean          visible)
#endif
{
    guint page_size = ibus_lookup_table_get_page_size(lookup_table);
    guint cursor_pos = ibus_lookup_table_get_cursor_pos(lookup_table);
    guint page = cursor_pos / page_size;
    guint start = page * page_size;
    guint end = start + page_size;
    guint num = ibus_lookup_table_get_number_of_candidates(lookup_table);
    if (end > num) {
        end = num;
    }

//     fprintf(stderr, "%d ~ %d pgsize %d num %d\n", start, end, page_size, num);

    guint i;

    gchar label[16][4];// WARNING large enough I think --- nihui
    const gchar *candidate;

    GVariantBuilder builder_labels;
    GVariantBuilder builder_candidates;
    GVariantBuilder builder_attrs;
    g_variant_builder_init (&builder_labels, G_VARIANT_TYPE ("as"));
    g_variant_builder_init (&builder_candidates, G_VARIANT_TYPE ("as"));
    g_variant_builder_init (&builder_attrs, G_VARIANT_TYPE ("as"));

    const gchar *attr = "";
    for (i = start; i < end; i++) {
        g_snprintf (label[i-start], 4, "%d", (i-start+1) % 10);
        // NOTE ibus always return NULL for ibus_lookup_table_get_label
//         label = ibus_lookup_table_get_label(lookup_table, i)->text;
        g_variant_builder_add (&builder_labels, "s", label[i-start]);

#if !IBUS_CHECK_VERSION(1,3,99)
        candidate = ibus_lookup_table_get_candidate (lookup_table, i)->text;
#else
        candidate = ibus_text_get_text (ibus_lookup_table_get_candidate (lookup_table, i));
#endif
        g_variant_builder_add (&builder_candidates, "s", candidate);

        g_variant_builder_add (&builder_attrs, "s", attr);
    }

    gboolean has_prev = 1;
    gboolean has_next = 1;

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateLookupTable",
                                   g_variant_new ("(asasasbb)",
                                                  &builder_labels,
                                                  &builder_candidates,
                                                  &builder_attrs,
                                                  has_prev, has_next),
                                   NULL);

    guint cursor_pos_in_page;
    if (ibus_lookup_table_is_cursor_visible(lookup_table))
        cursor_pos_in_page = cursor_pos % page_size;
    else
        cursor_pos_in_page = -1;

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateLookupTableCursor",
                                   g_variant_new ("(i)", cursor_pos_in_page),
                                   NULL);

    if (visible == 0)
#if !IBUS_CHECK_VERSION(1,3,99)
        ibus_panel_impanel_hide_lookup_table(panel, error);
#else
        ibus_panel_impanel_hide_lookup_table(panel);
#endif
    else
#if !IBUS_CHECK_VERSION(1,3,99)
        ibus_panel_impanel_show_lookup_table(panel, error);
#else
        ibus_panel_impanel_show_lookup_table(panel);
#endif
#if !IBUS_CHECK_VERSION(1,3,99)
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_update_preedit_text (IBusPanelService *panel,
                                        IBusText         *text,
                                        guint             cursor_pos,
                                        gboolean          visible,
                                        IBusError       **error)
#else
static void
ibus_panel_impanel_update_preedit_text (IBusPanelService *panel,
                                        IBusText         *text,
                                        guint             cursor_pos,
                                        gboolean          visible)
#endif
{
#if !IBUS_CHECK_VERSION(1,3,99)
    const gchar* t = text->text;
#else
    const gchar* t = ibus_text_get_text (text);
#endif
    const gchar *attr = "";

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdatePreeditText",
                                   g_variant_new ("(ss)", t, attr),
                                   NULL);

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdatePreeditCaret",
                                   g_variant_new ("(i)", cursor_pos),
                                   NULL);

    if (visible == 0)
#if !IBUS_CHECK_VERSION(1,3,99)
        ibus_panel_impanel_hide_preedit_text(panel, error);
#else
        ibus_panel_impanel_hide_preedit_text(panel);
#endif
    else
#if !IBUS_CHECK_VERSION(1,3,99)
        ibus_panel_impanel_show_preedit_text(panel, error);
#else
        ibus_panel_impanel_show_preedit_text(panel);
#endif
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_update_property (IBusPanelService *panel,
                                    IBusProperty     *prop,
                                    IBusError       **error)
#else
static void
ibus_panel_impanel_update_property (IBusPanelService *panel,
                                    IBusProperty     *prop)
#endif
{
    char propstr[512];
    propstr[0] = '\0';

    ibus_property_to_propstr(prop, propstr);

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateProperty",
                                   g_variant_new ("(s)", propstr),
                                   NULL);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_cursor_down_lookup_table (IBusPanelService *panel,
                                             IBusError       **error)
#else
static void
ibus_panel_impanel_cursor_down_lookup_table (IBusPanelService *panel)
#endif
{
    _UNUSED(panel);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_cursor_up_lookup_table (IBusPanelService *panel,
                                           IBusError       **error)
#else
static void
ibus_panel_impanel_cursor_up_lookup_table (IBusPanelService *panel)
#endif
{
    _UNUSED(panel);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_hide_auxiliary_text (IBusPanelService *panel,
                                        IBusError       **error)
#else
static void
ibus_panel_impanel_hide_auxiliary_text (IBusPanelService *panel)
#endif
{
    gboolean toShow = 0;

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowAux",
                                   g_variant_new ("(b)", toShow),
                                   NULL);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_hide_language_bar (IBusPanelService *panel,
                                      IBusError       **error)
#else
static void
ibus_panel_impanel_hide_language_bar (IBusPanelService *panel)
#endif
{
    _UNUSED(panel);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_hide_lookup_table (IBusPanelService *panel,
                                      IBusError       **error)
#else
static void
ibus_panel_impanel_hide_lookup_table (IBusPanelService *panel)
#endif
{
    gboolean toShow = 0;

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowLookupTable",
                                   g_variant_new ("(b)", toShow),
                                   NULL);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_hide_preedit_text (IBusPanelService *panel,
                                      IBusError       **error)
#else
static void
ibus_panel_impanel_hide_preedit_text (IBusPanelService *panel)
#endif
{
    gboolean toShow = 0;

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowPreedit",
                                   g_variant_new ("(b)", toShow),
                                   NULL);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_page_down_lookup_table (IBusPanelService *panel,
                                           IBusError       **error)
#else
static void
ibus_panel_impanel_page_down_lookup_table (IBusPanelService *panel)
#endif
{
    _UNUSED(panel);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_page_up_lookup_table (IBusPanelService *panel,
                                         IBusError       **error)
#else
static void
ibus_panel_impanel_page_up_lookup_table (IBusPanelService *panel)
#endif
{
    _UNUSED(panel);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_reset (IBusPanelService *panel,
                          IBusError       **error)
#else
static void
ibus_panel_impanel_reset (IBusPanelService *panel)
#endif
{
    _UNUSED(panel);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_show_auxiliary_text (IBusPanelService *panel,
                                        IBusError       **error)
#else
static void
ibus_panel_impanel_show_auxiliary_text (IBusPanelService *panel)
#endif
{
    gboolean toShow = 1;

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowAux",
                                   g_variant_new ("(b)", toShow),
                                   NULL);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_show_language_bar (IBusPanelService *panel,
                                      IBusError       **error)
#else
static void
ibus_panel_impanel_show_language_bar (IBusPanelService *panel)
#endif
{
    _UNUSED(panel);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_show_lookup_table (IBusPanelService *panel,
                                      IBusError       **error)
#else
static void
ibus_panel_impanel_show_lookup_table (IBusPanelService *panel)
#endif
{
    gboolean toShow = 1;

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowLookupTable",
                                   g_variant_new ("(b)", toShow),
                                   NULL);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_show_preedit_text (IBusPanelService *panel,
                                      IBusError       **error)
#else
static void
ibus_panel_impanel_show_preedit_text (IBusPanelService *panel)
#endif
{
    gboolean toShow = 1;

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowPreedit",
                                   g_variant_new ("(b)", toShow),
                                   NULL);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_start_setup (IBusPanelService *panel,
                                IBusError       **error)
#else
static void
ibus_panel_impanel_start_setup (IBusPanelService *panel)
#endif
{
    _UNUSED(panel);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
static gboolean
ibus_panel_impanel_state_changed (IBusPanelService *panel,
                                  IBusError       **error)
#else
static void
ibus_panel_impanel_state_changed (IBusPanelService *panel)
#endif
{
    IBusEngineDesc *engine_desc = ibus_input_context_get_engine(IBUS_PANEL_IMPANEL (panel)->input_context);
    if (!engine_desc) {
#if !IBUS_CHECK_VERSION(1,3,99)
        return FALSE;
#else
        return;
#endif
    }

#if !IBUS_CHECK_VERSION(1,3,99)
    ibus_property_set_icon (IBUS_PANEL_IMPANEL (panel)->logo_prop, engine_desc->icon);
#else
    ibus_property_set_icon (IBUS_PANEL_IMPANEL (panel)->logo_prop, ibus_engine_desc_get_icon (engine_desc));
#endif

    char propstr[512];
    propstr[0] = '\0';

    ibus_property_to_propstr(IBUS_PANEL_IMPANEL (panel)->logo_prop, propstr);

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateProperty",
                                   g_variant_new ("(s)", propstr),
                                   NULL);

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "Enable",
                                   g_variant_new ("(b)", TRUE),
                                   NULL);
#if !IBUS_CHECK_VERSION(1,3,99)
    _UNUSED(error);
    return TRUE;
#endif
}

static void
ibus_panel_impanel_exec_dialog (IBusPanelService *panel)
{
    char propstr[512];
    propstr[0] = '\0';

    ibus_property_to_propstr(IBUS_PANEL_IMPANEL (panel)->about_prop, propstr);

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ExecDialog",
                                   g_variant_new ("(s)", propstr),
                                   NULL);
}

static void
ibus_panel_impanel_exec_menu (IBusPanelService *panel)
{
    char propstr[512];

    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));

    GList *engines = ibus_bus_list_active_engines (IBUS_PANEL_IMPANEL (panel)->bus);
    IBusEngineDesc *engine_desc = NULL;
    GList *node = g_list_first (engines);
    while (node) {
        engine_desc = (IBusEngineDesc *)(node->data);
        node = g_list_next (node);

        ibus_engine_desc_to_propstr(engine_desc, propstr);
        g_variant_builder_add (&builder, "s", propstr);
    }

    g_dbus_connection_emit_signal (IBUS_PANEL_IMPANEL (panel)->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ExecMenu",
                                   g_variant_new ("(as)", &builder),
                                   NULL);
}

IBusPanelImpanel *
#if !IBUS_CHECK_VERSION(1,3,99)
ibus_panel_impanel_new (IBusConnection     *connection)
#else
ibus_panel_impanel_new (GDBusConnection    *connection)
#endif
{
    IBusPanelImpanel *panel;
    panel = (IBusPanelImpanel *) g_object_new (IBUS_TYPE_PANEL_IMPANEL,
#if !IBUS_CHECK_VERSION(1,3,99)
                                               "path", IBUS_PATH_PANEL,
#else
                                               "object-path", IBUS_PATH_PANEL,
#endif
                                               "connection", connection,
                                               NULL);
    return panel;
}
