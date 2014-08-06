/*
 *  Copyright (C) 2011-2012 Ni Hui <shuizhuyuanluo@126.com>
 *  Copyright (C) 2013-2014 Weng Xuetian <wengxt@gmail.com>
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
#include <string>
#include <QByteArray>
#include <QPair>
#include <QStringList>
#include <QDebug>
#include "panel.h"
#include "propertymanager.h"
#include "enginemanager.h"
#include "xkblayoutmanager.h"
#include "app.h"
#include "gtkaccelparse_p.h"

#ifndef DBUS_ERROR_FAILED
#define DBUS_ERROR_FAILED "org.freedesktop.DBus.Error.Failed"
#endif /* DBUS_ERROR_FAILED */
#define _UNUSED(x) ((void) x)

typedef struct _IBusPanelImpanelClass IBusPanelImpanelClass;

struct _IBusPanelImpanel {
    IBusPanelService    parent;
    IBusBus            *bus;
    GDBusConnection    *conn;
    IBusProperty       *logo_prop;
    PropertyManager* propManager;
    EngineManager* engineManager;
    XkbLayoutManager* xkbLayoutManager;
    App* app;
    gboolean useSystemKeyboardLayout;
    int selected;
};

struct _IBusPanelImpanelClass {
    IBusPanelServiceClass parent;
};


static void
impanel_set_engine(IBusPanelImpanel* impanel, const char* name);


static void
ibus_property_to_propstr (IBusProperty *property,
                          char *propstr,
                          gboolean useSymbol = FALSE,
                          IBusEngineDesc* engine = NULL);

void
ibus_panel_impanel_set_bus (IBusPanelImpanel *impanel,
                            IBusBus          *bus)
{
    impanel->bus = bus;
}

void ibus_panel_impanel_set_app(IBusPanelImpanel* impanel, App* app)
{
    impanel->app = app;
}

void ibus_panel_impanel_accept(IBusPanelImpanel* impanel)
{
    if (impanel->selected >= 0 && impanel->selected < impanel->engineManager->length()) {
        impanel_set_engine(impanel, ibus_engine_desc_get_name(impanel->engineManager->engines()[impanel->selected]));
        impanel->selected = -1;
    }
}

void ibus_panel_impanel_navigate(IBusPanelImpanel* impanel, gboolean start)
{
    if (start) {
        impanel->selected = -1;
    }

    if (impanel->engineManager->length() < 2) {
        return;
    }

    IBusEngineDesc* engine_desc = NULL;
    if (impanel->selected < 0) {
        engine_desc = ibus_bus_get_global_engine(impanel->bus);
    } else if (impanel->selected < impanel->engineManager->length()) {
        engine_desc = impanel->engineManager->engines()[impanel->selected];
    } else {
        engine_desc = impanel->engineManager->engines()[0];
    }

    if (engine_desc) {
        const char* name = impanel->engineManager->navigate(engine_desc);
        impanel->selected = impanel->engineManager->getIndexByName(name);
    }

    if (impanel->selected < impanel->engineManager->length()) {
        engine_desc = impanel->engineManager->engines()[impanel->selected];

        ibus_property_set_icon (impanel->logo_prop, ibus_engine_desc_get_icon (engine_desc));

        char propstr[512];
        propstr[0] = '\0';

        ibus_property_to_propstr(impanel->logo_prop, propstr, TRUE, engine_desc);

        g_dbus_connection_emit_signal (impanel->conn,
                                       NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateProperty",
                                       (g_variant_new ("(s)", propstr)),
                                       NULL);
    }
}

void ibus_panel_impanel_move_next(IBusPanelImpanel* impanel)
{
    if (impanel->engineManager->length() >= 2) {
        impanel_set_engine(impanel, ibus_engine_desc_get_name(impanel->engineManager->engines()[1]));
    }
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
    "    <signal name='ExecMenu'>"
    "      <arg type='as' name='actions'/>"
    "    </signal>"
    "  </interface>"
    "</node>";

/* functions prototype */
static void         ibus_panel_impanel_class_init               (IBusPanelImpanelClass  *klass);
static void         ibus_panel_impanel_init                     (IBusPanelImpanel       *impanel);
static void         ibus_panel_impanel_destroy                  (IBusPanelImpanel       *impanel);

static void         ibus_panel_impanel_focus_in                 (IBusPanelService       *panel,
                                                                 const gchar            *input_context_path);
static void         ibus_panel_impanel_focus_out                (IBusPanelService       *panel,
                                                                 const gchar            *input_context_path);
static void         ibus_panel_impanel_register_properties      (IBusPanelService       *panel,
                                                                 IBusPropList           *prop_list);
static void         ibus_panel_impanel_real_register_properties (IBusPanelImpanel       *impanel);
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

/* impanel signal handler function */
static void         ibus_panel_impanel_exec_im_menu             (IBusPanelImpanel* impanel);
static void         ibus_panel_impanel_exec_menu                (IBusPanelImpanel* impanel, IBusPropList* prop_list);

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
    QByteArray str(key);
    str.replace(':', '!');
    strcat(propstr, str.constData());
    strcat(propstr, prop_sep);
    strcat(propstr, label);
    strcat(propstr, prop_sep);
    strcat(propstr, icon);
    strcat(propstr, prop_sep);
    strcat(propstr, tooltip);
}

static void
ibus_property_to_propstr (IBusProperty *property,
                          char *propstr,
                          gboolean useSymbol,
                          IBusEngineDesc* engine)
{
    const gchar* label = ibus_text_get_text (useSymbol ? ibus_property_get_symbol (property) : ibus_property_get_label(property));
    const gchar* icon = ibus_property_get_icon (property);
    if (engine && strncmp("xkb:", ibus_engine_desc_get_name(engine), 4) == 0) {
        label = ibus_engine_desc_get_name(engine) + 4;
        icon = "";
    }
    ibus_property_args_to_propstr(ibus_property_get_key (property),
                                  label,
                                  icon,
                                  ibus_text_get_text (ibus_property_get_tooltip (property)),
                                  propstr);
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
    QByteArray data(name);
    data.replace(':', '!');
    strcat(propstr, data.constData());
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
    ibus_engine_desc_args_to_propstr(ibus_engine_desc_get_name(engine_desc),
                                     ibus_engine_desc_get_language(engine_desc),
                                     ibus_engine_desc_get_longname(engine_desc),
                                     ibus_engine_desc_get_icon(engine_desc),
                                     ibus_engine_desc_get_description(engine_desc),
                                     propstr);
}

static void
impanel_get_default_engine(IBusPanelImpanel* impanel, char*** pengine_names, gsize* plen)
{
    GList* engines = ibus_bus_list_engines(impanel->bus);
    if (!engines) {
        *pengine_names = g_new0 (gchar*, 2);
        *plen = 1;
        (*pengine_names)[0] = g_strdup ("xkb:us::eng");
        return;
    }

    QList<QByteArray> engineList;
    impanel->xkbLayoutManager->getLayout();
    QStringList layouts = impanel->xkbLayoutManager->defaultLayout().split(',');
    QStringList variants = impanel->xkbLayoutManager->defaultVariant().split(',');

    for (int i = 0; i < layouts.size(); i ++ ) {
        QString variant;
        if (i < variants.size()) {
            variant = variants[i];
        }

        for (GList* engine = g_list_first(engines); engine != NULL ;
            engine = g_list_next(engine)) {
            IBusEngineDesc* desc = IBUS_ENGINE_DESC (engine->data);
            QByteArray name = ibus_engine_desc_get_name(desc);
            if (!name.startsWith("xkb:")) {
                continue;
            }

            if (QLatin1String(ibus_engine_desc_get_layout(desc)) == layouts[i] &&
                QLatin1String(ibus_engine_desc_get_layout_variant(desc)) == variant) {
                engineList << name;
            }
        }
    }
    const char* locale = setlocale(LC_CTYPE, NULL);
    if (!locale) {
        locale = "C";
    }

    QStringList localeList = QString(locale).split('.');
    QString lang = localeList.size() > 0 ? localeList[0] : "";

    bool added = false;
    for (GList* engine = g_list_first(engines); engine != NULL ;
        engine = g_list_next(engine)) {
        IBusEngineDesc* desc = IBUS_ENGINE_DESC (engine->data);
        QByteArray name = ibus_engine_desc_get_name(desc);
        if (name.startsWith("xkb:")) {
            continue;
        }

        if (QLatin1String(ibus_engine_desc_get_language(desc)) == lang &&
            ibus_engine_desc_get_rank(desc) > 0) {
            engineList << name;
            added = true;
        }
    }

    if (!added) {
        localeList = QString(lang).split('_');
        QString lang = localeList.size() > 0 ? localeList[0] : "";

        for (GList* engine = g_list_first(engines); engine != NULL ;
            engine = g_list_next(engine)) {
            IBusEngineDesc* desc = IBUS_ENGINE_DESC (engine->data);
            QByteArray name = ibus_engine_desc_get_name(desc);
            if (name.startsWith("xkb:")) {
                continue;
            }

            if (QLatin1String(ibus_engine_desc_get_language(desc)) == lang &&
                ibus_engine_desc_get_rank(desc) > 0) {
                engineList << name;
            }
        }
    }

    for (GList* engine = g_list_first(engines); engine != NULL ;
        engine = g_list_next(engine)) {
        IBusEngineDesc* desc = IBUS_ENGINE_DESC (engine->data);
        g_object_unref(desc);
    }

    g_list_free(engines);

    if (engineList.size() == 0) {
        *pengine_names = g_new0 (gchar*, 2);
        *plen = 1;
        (*pengine_names)[0] = g_strdup ("xkb:us::eng");
        return;
    } else {
        *pengine_names = g_new0 (gchar*, engineList.size() + 1);
        *plen = engineList.size();
        size_t i = 0;
        Q_FOREACH(const QByteArray& name, engineList) {
            (*pengine_names)[i] = g_strdup (name.constData());
            i ++;
        }
    }
}

static void
impanel_update_engines(IBusPanelImpanel* impanel, GVariant* var_engines) {
    gchar** engine_names = NULL;
    size_t len = 0;
    if (var_engines) {
        engine_names = g_variant_dup_strv(var_engines, &len);
    }
    if (len == 0) {
        g_strfreev(engine_names);
        engine_names = NULL;
    }

    if (!engine_names) {
        impanel_get_default_engine(impanel, &engine_names, &len);
        IBusConfig* config = ibus_bus_get_config(impanel->bus);
        GVariant* var = g_variant_new_strv(engine_names, len);
        ibus_config_set_value(config, "general", "preload_engines", var);
    }

    IBusEngineDesc** engines = ibus_bus_get_engines_by_names(impanel->bus, engine_names);
    g_strfreev(engine_names);

    impanel->engineManager->setEngines(engines, len);

    if (engines && engines[0]) {
        ibus_bus_set_global_engine_async(impanel->bus, ibus_engine_desc_get_name(engines[0]), 1000, NULL, NULL, NULL);
    }

    impanel->app->setDoGrab(len > 1);
}

static void
impanel_update_engines_order(IBusPanelImpanel* impanel, GVariant* var_engines) {
    const gchar** engine_names = NULL;
    size_t len = 0;
    engine_names = g_variant_get_strv(var_engines, &len);
    if (len == 0) {
        return;
    }

    impanel->engineManager->setOrder(engine_names, len);

    if (impanel->engineManager->engines()) {
        ibus_bus_set_global_engine_async(impanel->bus, ibus_engine_desc_get_name(impanel->engineManager->engines()[0]), 1000, NULL, NULL, NULL);
    }
}

static void
impanel_update_triggers(IBusPanelImpanel* impanel, GVariant* variant) {
    gchar** triggers = NULL;
    size_t len = 0;
    if (variant) {
        triggers = g_variant_dup_strv(variant, &len);
    }
    if (len == 0) {
        g_strfreev(triggers);
        triggers = 0;
    }
    if (!triggers) {
        triggers = g_new0 (gchar*, 2);
        len = 1;
        triggers[0] = g_strdup ("<Super>space");
    }

    QList<QPair<uint, uint> > triggersList;
    for (size_t i = 0; i < len; i ++) {
        guint key = 0;
        GdkModifierType mod = (GdkModifierType) 0;
        _gtk_accelerator_parse(triggers[i], &key, &mod);
        if (key) {
            triggersList << qMakePair<uint, uint>(key, (uint) mod);
        }
    }
    impanel->app->setTriggerKeys(triggersList);
}


static void
impanel_update_use_system_keyboard_layout(IBusPanelImpanel* impanel, GVariant* variant) {
    impanel->useSystemKeyboardLayout = g_variant_get_boolean(variant);
}
static void
impanel_update_use_global_engine(IBusPanelImpanel* impanel, GVariant* variant) {
    impanel->engineManager->setUseGlobalEngine(g_variant_get_boolean(variant));
}

static void
impanel_update_latin_layouts(IBusPanelImpanel* impanel, GVariant* variant) {
    if (!variant) {
        return;
    }
    gsize length;
    const gchar** variants = g_variant_get_strv(variant, &length);

    impanel->xkbLayoutManager->setLatinLayouts(variants, length);
}

static void
impanel_config_value_changed_callback (IBusConfig* config, const gchar* section, const gchar* name, GVariant* value, gpointer user_data)
{
    _UNUSED(config);
    IBusPanelImpanel* impanel = ((IBusPanelImpanel *)user_data);

    if (g_strcmp0(section, "general") == 0 && g_strcmp0(name, "preload_engines") == 0) {
        impanel_update_engines(impanel, value);
    }
    else if (g_strcmp0(section, "general/hotkey") == 0 && g_strcmp0(name, "triggers") == 0) {
        impanel_update_triggers(impanel, value);
    } else if (g_strcmp0(section, "general") == 0 && g_strcmp0(name, "use-system-keyboard-layout") == 0) {
        impanel_update_use_system_keyboard_layout(impanel, value);
    } else if (g_strcmp0(section, "general") == 0 && g_strcmp0(name, "use-global-engine") == 0) {
        impanel_update_use_global_engine(impanel, value);
    }
}

static void
impanel_exit_callback (GDBusConnection *connection,
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
    IBusPanelImpanel* impanel = ((IBusPanelImpanel *)user_data);
    if (impanel->bus) {
        ibus_bus_exit(impanel->bus, FALSE);
    }
}

static void
impanel_panel_created_callback (GDBusConnection *connection,
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
    IBusPanelImpanel* impanel = ((IBusPanelImpanel *)user_data);
    ibus_panel_impanel_real_register_properties(impanel);

}

static void
impanel_set_engine(IBusPanelImpanel* impanel, const char* name)
{
    if (!name || !name[0]) {
        return;
    }
    if (ibus_bus_set_global_engine(impanel->bus, name)) {
        if (!impanel->useSystemKeyboardLayout) {
            IBusEngineDesc* engine_desc = ibus_bus_get_global_engine(impanel->bus);
            impanel->xkbLayoutManager->setLayout(engine_desc);
        }
        impanel->engineManager->setCurrentEngine(name);
    }
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
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL(user_data);
    gchar *s0 = NULL;
    g_variant_get (parameters, "(s)", &s0);
    if (!s0 || strlen(s0) <= 6)
        return;
    QByteArray prop_key(s0 + 6);// +6 to skip "/IBus/"
    prop_key.replace('!', ':');
    if (g_ascii_strncasecmp (prop_key.constData(), "Logo", 4) == 0)
        ibus_panel_impanel_exec_im_menu(impanel);
    else if (g_ascii_strncasecmp (prop_key.constData(), "Engine/", 7) == 0) {
        impanel_set_engine(impanel, prop_key.constData() + 7);
    }
    else {
        IBusProperty* property = impanel->propManager->property(prop_key.constData());
        if (property) {
            IBusPropState newstate = ibus_property_get_state(property);
            switch (ibus_property_get_prop_type(property)) {
                case PROP_TYPE_RADIO:
                case PROP_TYPE_TOGGLE:
                    if (ibus_property_get_prop_type(property) == PROP_TYPE_TOGGLE) {
                        if (newstate == PROP_STATE_CHECKED)
                            newstate = PROP_STATE_UNCHECKED;
                        else if (newstate == PROP_STATE_UNCHECKED)
                            newstate = PROP_STATE_CHECKED;
                    }
                    else if (ibus_property_get_prop_type(property) == PROP_TYPE_RADIO) {
                        newstate = PROP_STATE_CHECKED;
                    }
                case PROP_TYPE_NORMAL:
                    ibus_property_set_state(property, newstate);
                    ibus_panel_service_property_activate((IBusPanelService *)impanel, prop_key.constData(), newstate);
                    break;
                case PROP_TYPE_MENU:
                    ibus_panel_impanel_exec_menu(impanel, ibus_property_get_sub_props(property));
                case PROP_TYPE_SEPARATOR:
                    break;
                default:
                    break;
            }
        }
        else {
            ibus_panel_service_property_activate((IBusPanelService *)impanel, prop_key.constData(), PROP_STATE_CHECKED);
        }
    }
    g_free(s0);
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
    IBusPanelImpanel* impanel = ((IBusPanelImpanel *)user_data);
    impanel->conn = connection;

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
                                        "PanelCreated",
                                        "/org/kde/impanel",
                                        NULL,
                                        G_DBUS_SIGNAL_FLAGS_NONE,
                                        impanel_panel_created_callback,
                                        user_data,
                                        NULL);
    g_dbus_connection_signal_subscribe (connection,
                                        "org.kde.impanel",
                                        "org.kde.impanel",
                                        "Exit",
                                        "/org/kde/impanel",
                                        NULL,
                                        G_DBUS_SIGNAL_FLAGS_NONE,
                                        impanel_exit_callback,
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

    IBusConfig* config = ibus_bus_get_config(impanel->bus);
    if (config) {
        g_signal_connect_object(config, "value-changed", (GCallback) impanel_config_value_changed_callback, impanel, (GConnectFlags) 0);
        ibus_config_watch(config, "general", "preload_engines");
        ibus_config_watch(config, "general", "use-system-keyboard-layout");
        ibus_config_watch(config, "general/hotkey", "triggers");

        GVariant* var_engines = ibus_config_get_value(config, "general", "preload_engines");
        impanel_update_engines(impanel, var_engines);
        if (var_engines)
            g_variant_unref(var_engines);

        var_engines = ibus_config_get_value(config, "general", "engines-order");
        if (var_engines) {
            impanel_update_engines_order(impanel, var_engines);
            g_variant_unref(var_engines);
        }

        GVariant* var_triggers = ibus_config_get_value(config, "general/hotkey", "triggers");
        impanel_update_triggers(impanel, var_triggers);
        if (var_triggers)
            g_variant_unref(var_triggers);

        IBusConfig* config = ibus_bus_get_config(impanel->bus);
        GVariant* var_layouts = ibus_config_get_value(config, "general", "xkb-latin-layouts");
        if (var_layouts) {
            impanel_update_latin_layouts(impanel, var_layouts);
            g_variant_unref(var_layouts);
        }

        GVariant* var = ibus_config_get_value(config, "general", "use-system-keyboard-layout");
        if (var) {
            impanel_update_use_system_keyboard_layout(impanel, var);
            g_variant_unref(var);
        }

        var = ibus_config_get_value(config, "general", "use-global-engine");
        if (var) {
            impanel_update_use_global_engine(impanel, var);
            g_variant_unref(var);
        }
    }

    ibus_panel_impanel_real_register_properties(impanel);
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

G_DEFINE_TYPE (IBusPanelImpanel, ibus_panel_impanel, IBUS_TYPE_PANEL_SERVICE)

static void
ibus_panel_impanel_class_init (IBusPanelImpanelClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

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
    impanel->app = NULL;
    impanel->useSystemKeyboardLayout = false;
    impanel->selected = -1;

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

    impanel->propManager = new PropertyManager;
    impanel->engineManager = new EngineManager;
    impanel->xkbLayoutManager = new XkbLayoutManager;
}

static void
ibus_panel_impanel_destroy (IBusPanelImpanel *impanel)
{
    g_object_unref (impanel->logo_prop);
    impanel->logo_prop = NULL;
    delete impanel->propManager;
    impanel->propManager = NULL;
    delete impanel->engineManager;
    impanel->engineManager = NULL;
    delete impanel->xkbLayoutManager;
    impanel->xkbLayoutManager = NULL;

    g_bus_unown_name (owner_id);
    g_dbus_node_info_unref (introspection_data);

    IBUS_OBJECT_CLASS (ibus_panel_impanel_parent_class)->destroy ((IBusObject *)impanel);
}

static void
ibus_panel_impanel_focus_in (IBusPanelService *panel,
                             const gchar      *input_context_path)
{
    GDBusConnection *ibusconn;
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL (panel);
    g_object_get (impanel,
                  "connection", &ibusconn,
                  NULL);

    IBusEngineDesc *engine_desc = NULL;
    engine_desc = ibus_bus_get_global_engine(impanel->bus);

    const gchar* icon_name = "input-keyboard";
    if (engine_desc) {
        icon_name = ibus_engine_desc_get_icon (engine_desc);
    }

    ibus_property_set_icon (impanel->logo_prop, icon_name);

    char propstr[512];
    propstr[0] = '\0';

    ibus_property_to_propstr(impanel->logo_prop, propstr, TRUE, engine_desc);

    impanel->engineManager->setCurrentContext(input_context_path);
    if (!impanel->engineManager->useGlobalEngine()) {
        impanel_set_engine(impanel, impanel->engineManager->currentEngine().toUtf8().constData());
    }

    if (!impanel->conn)
        return;

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateProperty",
                                   (g_variant_new ("(s)", propstr)),
                                   NULL);
}

static void
ibus_panel_impanel_focus_out (IBusPanelService *panel,
                              const gchar      *input_context_path)
{
    _UNUSED(panel);
    _UNUSED(input_context_path);
}

static void
ibus_panel_impanel_register_properties (IBusPanelService *panel,
                                        IBusPropList     *prop_list)
{
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL (panel);
    impanel->propManager->setProperties(prop_list);
    ibus_panel_impanel_real_register_properties(impanel);
}

static void
ibus_panel_impanel_real_register_properties(IBusPanelImpanel* impanel)
{
    IBusProperty* property = NULL;
    guint i = 0;
    char propstr[512];

    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));

    ibus_property_to_propstr(impanel->logo_prop, propstr, TRUE);
    g_variant_builder_add (&builder, "s", propstr);

    IBusPropList* prop_list = impanel->propManager->properties();
    if (prop_list) {
        while ( ( property = ibus_prop_list_get( prop_list, i ) ) != NULL ) {
            ibus_property_to_propstr(property, propstr, TRUE);
            g_variant_builder_add (&builder, "s", propstr);
            ++i;
        }
    }

    if (!impanel->conn)
        return;

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "RegisterProperties",
                                   (g_variant_new ("(as)", &builder)),
                                   NULL);
}


static void
ibus_panel_impanel_set_cursor_location (IBusPanelService *panel,
                                        gint              x,
                                        gint              y,
                                        gint              w,
                                        gint              h)
{

    g_dbus_connection_call(IBUS_PANEL_IMPANEL (panel)->conn,
                            "org.kde.impanel",
                            "/org/kde/impanel",
                            "org.kde.impanel2",
                            "SetSpotRect",
                            (g_variant_new("(iiii)", x, y, w, h)),
                            NULL,
                            G_DBUS_CALL_FLAGS_NONE,
                           -1,           /* timeout */
                           NULL,
                           NULL,
                           NULL);
}

static void
ibus_panel_impanel_update_auxiliary_text (IBusPanelService *panel,
                                          IBusText         *text,
                                          gboolean          visible)
{
    const gchar* t = ibus_text_get_text (text);
    const gchar *attr = "";
    IBusPanelImpanel* impanel = (IBusPanelImpanel*) panel;

    if (!impanel->conn)
        return;

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateAux",
                                   (g_variant_new ("(ss)", t, attr)),
                                   NULL);

    if (visible == 0)
        ibus_panel_impanel_hide_auxiliary_text(panel);
    else
        ibus_panel_impanel_show_auxiliary_text(panel);
}

static void
ibus_panel_impanel_update_lookup_table (IBusPanelService *panel,
                                        IBusLookupTable  *lookup_table,
                                        gboolean          visible)
{
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;

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

        candidate = ibus_text_get_text (ibus_lookup_table_get_candidate (lookup_table, i));
        g_variant_builder_add (&builder_candidates, "s", candidate);

        g_variant_builder_add (&builder_attrs, "s", attr);
    }

    gboolean has_prev = 1;
    gboolean has_next = 1;

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateLookupTable",
                                   (g_variant_new ("(asasasbb)",
                                                   &builder_labels,
                                                   &builder_candidates,
                                                   &builder_attrs,
                                                   has_prev, has_next)),
                                   NULL);

    guint cursor_pos_in_page;
    if (ibus_lookup_table_is_cursor_visible(lookup_table))
        cursor_pos_in_page = cursor_pos % page_size;
    else
        cursor_pos_in_page = -1;

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateLookupTableCursor",
                                   (g_variant_new ("(i)", cursor_pos_in_page)),
                                   NULL);

    if (visible == 0)
        ibus_panel_impanel_hide_lookup_table(panel);
    else
        ibus_panel_impanel_show_lookup_table(panel);
}

static void
ibus_panel_impanel_update_preedit_text (IBusPanelService *panel,
                                        IBusText         *text,
                                        guint             cursor_pos,
                                        gboolean          visible)
{
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;

    const gchar* t = ibus_text_get_text (text);
    const gchar *attr = "";

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdatePreeditText",
                                   (g_variant_new ("(ss)", t, attr)),
                                   NULL);

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdatePreeditCaret",
                                   (g_variant_new ("(i)", cursor_pos)),
                                   NULL);

    if (visible == 0)
        ibus_panel_impanel_hide_preedit_text(panel);
    else
        ibus_panel_impanel_show_preedit_text(panel);
}

static void
ibus_panel_impanel_update_property (IBusPanelService *panel,
                                    IBusProperty     *prop)
{
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;

    impanel->propManager->updateProperty(prop);

    char propstr[512];
    propstr[0] = '\0';

    ibus_property_to_propstr(prop, propstr, TRUE);

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateProperty",
                                   (g_variant_new ("(s)", propstr)),
                                   NULL);
}

static void
ibus_panel_impanel_cursor_down_lookup_table (IBusPanelService *panel)
{
    _UNUSED(panel);
}

static void
ibus_panel_impanel_cursor_up_lookup_table (IBusPanelService *panel)
{
    _UNUSED(panel);
}

static void
ibus_panel_impanel_hide_auxiliary_text (IBusPanelService *panel)
{
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;
    gboolean toShow = 0;

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowAux",
                                   (g_variant_new ("(b)", toShow)),
                                   NULL);
}

static void
ibus_panel_impanel_hide_language_bar (IBusPanelService *panel)
{
    _UNUSED(panel);
}

static void
ibus_panel_impanel_hide_lookup_table (IBusPanelService *panel)
{
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;
    gboolean toShow = 0;

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowLookupTable",
                                   (g_variant_new ("(b)", toShow)),
                                   NULL);
}

static void
ibus_panel_impanel_hide_preedit_text (IBusPanelService *panel)
{
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;
    gboolean toShow = 0;

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowPreedit",
                                   (g_variant_new ("(b)", toShow)),
                                   NULL);
}

static void
ibus_panel_impanel_page_down_lookup_table (IBusPanelService *panel)
{
    _UNUSED(panel);
}

static void
ibus_panel_impanel_page_up_lookup_table (IBusPanelService *panel)
{
    _UNUSED(panel);
}

static void
ibus_panel_impanel_reset (IBusPanelService *panel)
{
    _UNUSED(panel);
}

static void
ibus_panel_impanel_show_auxiliary_text (IBusPanelService *panel)
{
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;
    gboolean toShow = 1;

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowAux",
                                   (g_variant_new ("(b)", toShow)),
                                   NULL);
}

static void
ibus_panel_impanel_show_language_bar (IBusPanelService *panel)
{
    _UNUSED(panel);
}

static void
ibus_panel_impanel_show_lookup_table (IBusPanelService *panel)
{
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;
    gboolean toShow = 1;

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowLookupTable",
                                   (g_variant_new ("(b)", toShow)),
                                   NULL);
}

static void
ibus_panel_impanel_show_preedit_text (IBusPanelService *panel)
{
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;
    gboolean toShow = 1;

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowPreedit",
                                   (g_variant_new ("(b)", toShow)),
                                   NULL);
}

static void
ibus_panel_impanel_start_setup (IBusPanelService *panel)
{
    _UNUSED(panel);
}

static void
ibus_panel_impanel_state_changed (IBusPanelService *panel)
{
    IBusPanelImpanel* impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;

    if (impanel->app->keyboardGrabbed()) {
        return;
    }

    IBusEngineDesc *engine_desc = ibus_bus_get_global_engine(impanel->bus);
    if (!engine_desc) {
        return;
    }

    ibus_property_set_icon (impanel->logo_prop, ibus_engine_desc_get_icon (engine_desc));

    char propstr[512];
    propstr[0] = '\0';

    ibus_property_to_propstr(impanel->logo_prop, propstr, TRUE, engine_desc);

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateProperty",
                                   (g_variant_new ("(s)", propstr)),
                                   NULL);

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "Enable",
                                   (g_variant_new ("(b)", TRUE)),
                                   NULL);

    impanel->engineManager->moveToFirst(engine_desc);
    QStringList engineList = impanel->engineManager->engineOrder();

    gchar** engine_names = g_new0 (gchar*, engineList.size() + 1);
    size_t i = 0;
    Q_FOREACH(const QString& name, engineList) {
        engine_names[i] = g_strdup (name.toUtf8().constData());
        i ++;
    }

    IBusConfig* config = ibus_bus_get_config(impanel->bus);
    if (config) {
        GVariant* var = g_variant_new_strv(engine_names, engineList.size());
        ibus_config_set_value(config, "general", "engines-order", var);
    }
    g_strfreev(engine_names);
}

static void
ibus_panel_impanel_exec_menu(IBusPanelImpanel* impanel,
                             IBusPropList* prop_list)
{
    char propstr[512];
    if (!impanel->conn)
        return;

    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));

    int i = 0;
    while (true) {
        IBusProperty* prop = ibus_prop_list_get(prop_list, i);
        if (!prop)
            break;
        ibus_property_to_propstr(prop, propstr);
        g_variant_builder_add (&builder, "s", propstr);
        i ++;
    }

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ExecMenu",
                                   (g_variant_new ("(as)", &builder)),
                                   NULL);
}


static void
ibus_panel_impanel_exec_im_menu (IBusPanelImpanel* impanel)
{
    char propstr[512];
    if (!impanel->conn)
        return;

    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));

    IBusEngineDesc** engines = impanel->engineManager->engines();
    if (engines) {
        int i = 0;
        while (engines[i]) {
            ibus_engine_desc_to_propstr(engines[i], propstr);
            g_variant_builder_add (&builder, "s", propstr);
            i ++;
        }
    }

    g_dbus_connection_emit_signal (impanel->conn,
                                   NULL, "/kimpanel", "org.kde.kimpanel.inputmethod", "ExecMenu",
                                   (g_variant_new ("(as)", &builder)),
                                   NULL);
}

IBusPanelImpanel *
ibus_panel_impanel_new (GDBusConnection    *connection)
{
    IBusPanelImpanel *panel;
    panel = (IBusPanelImpanel *) g_object_new (IBUS_TYPE_PANEL_IMPANEL,
                                               "object-path", IBUS_PATH_PANEL,
                                               "connection", connection,
                                               NULL);
    return panel;
}

