/*
    SPDX-FileCopyrightText: 2011-2012 Ni Hui <shuizhuyuanluo@126.com>
    SPDX-FileCopyrightText: 2013-2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "panel.h"
#include "app.h"
#include "enginemanager.h"
#include "gtkaccelparse_p.h"
#include "propertymanager.h"
#include "xkblayoutmanager.h"
#include <QByteArray>
#include <QDebug>
#include <QPair>
#include <QStringList>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#ifndef DBUS_ERROR_FAILED
#define DBUS_ERROR_FAILED "org.freedesktop.DBus.Error.Failed"
#endif /* DBUS_ERROR_FAILED */

#define IBUS_SCHEMA_GENERAL "org.freedesktop.ibus.general"
#define IBUS_SCHEMA_HOTKEY "org.freedesktop.ibus.general.hotkey"
#define IBUS_SCHEMA_PANEL "org.freedesktop.ibus.panel"

typedef struct _IBusPanelImpanelClass IBusPanelImpanelClass;

struct _IBusPanelImpanel {
    IBusPanelService parent;
    IBusBus *bus;
    GDBusConnection *conn;
    PropertyManager *propManager;
    EngineManager *engineManager;
    XkbLayoutManager *xkbLayoutManager;
    App *app;
    gboolean useSystemKeyboardLayout;
    int selected;
    GSettings *settings_general;
    GSettings *settings_hotkey;
};

struct _IBusPanelImpanelClass {
    IBusPanelServiceClass parent;
};

/* functions prototype */
static void ibus_panel_impanel_class_init(IBusPanelImpanelClass *klass);
static void ibus_panel_impanel_init(IBusPanelImpanel *impanel);
static void ibus_panel_impanel_destroy(IBusPanelImpanel *impanel);

static void ibus_panel_impanel_focus_in(IBusPanelService *panel, const gchar *input_context_path);
static void ibus_panel_impanel_focus_out(IBusPanelService *panel, const gchar *input_context_path);
static void ibus_panel_impanel_register_properties(IBusPanelService *panel, IBusPropList *prop_list);
static void ibus_panel_impanel_real_register_properties(IBusPanelImpanel *impanel);
static void ibus_panel_impanel_set_cursor_location(IBusPanelService *panel, gint x, gint y, gint w, gint h);
static void ibus_panel_impanel_update_auxiliary_text(IBusPanelService *panel, IBusText *text, gboolean visible);
static void ibus_panel_impanel_update_lookup_table(IBusPanelService *panel, IBusLookupTable *lookup_table, gboolean visible);
static void ibus_panel_impanel_update_preedit_text(IBusPanelService *panel, IBusText *text, guint cursor_pos, gboolean visible);
static void ibus_panel_impanel_update_property(IBusPanelService *panel, IBusProperty *prop);
static void ibus_panel_impanel_cursor_down_lookup_table(IBusPanelService *panel);
static void ibus_panel_impanel_cursor_up_lookup_table(IBusPanelService *panel);
static void ibus_panel_impanel_hide_auxiliary_text(IBusPanelService *panel);
static void ibus_panel_impanel_hide_language_bar(IBusPanelService *panel);
static void ibus_panel_impanel_hide_lookup_table(IBusPanelService *panel);
static void ibus_panel_impanel_hide_preedit_text(IBusPanelService *panel);
static void ibus_panel_impanel_page_down_lookup_table(IBusPanelService *panel);
static void ibus_panel_impanel_page_up_lookup_table(IBusPanelService *panel);
static void ibus_panel_impanel_reset(IBusPanelService *panel);
static void ibus_panel_impanel_show_auxiliary_text(IBusPanelService *panel);
static void ibus_panel_impanel_show_language_bar(IBusPanelService *panel);
static void ibus_panel_impanel_show_lookup_table(IBusPanelService *panel);
static void ibus_panel_impanel_show_preedit_text(IBusPanelService *panel);
static void ibus_panel_impanel_start_setup(IBusPanelService *panel);
static void ibus_panel_impanel_state_changed(IBusPanelService *panel);

/* impanel signal handler function */
static void ibus_panel_impanel_exec_im_menu(IBusPanelImpanel *impanel);
static void ibus_panel_impanel_exec_menu(IBusPanelImpanel *impanel, IBusPropList *prop_list);

static void impanel_set_engine(IBusPanelImpanel *impanel, const char *name);

static QByteArray ibus_property_to_propstr(IBusProperty *property, gboolean useSymbol = FALSE);

static QByteArray ibus_engine_desc_to_logo_propstr(IBusEngineDesc *engine);

void impanel_update_logo_by_engine(IBusPanelImpanel *impanel, IBusEngineDesc *engine_desc)
{
    if (!impanel->conn) {
        return;
    }

    QByteArray propstr = ibus_engine_desc_to_logo_propstr(engine_desc);

    g_dbus_connection_emit_signal(impanel->conn,
                                  nullptr,
                                  "/kimpanel",
                                  "org.kde.kimpanel.inputmethod",
                                  "UpdateProperty",
                                  (g_variant_new("(s)", propstr.constData())),
                                  nullptr);
}

void ibus_panel_impanel_set_bus(IBusPanelImpanel *impanel, IBusBus *bus)
{
    impanel->bus = bus;
}

void ibus_panel_impanel_set_app(IBusPanelImpanel *impanel, App *app)
{
    impanel->app = app;
}

void ibus_panel_impanel_accept(IBusPanelImpanel *impanel)
{
    if (impanel->selected >= 0 && static_cast<size_t>(impanel->selected) < impanel->engineManager->length()) {
        impanel_set_engine(impanel, ibus_engine_desc_get_name(impanel->engineManager->engines()[impanel->selected]));
        impanel->selected = -1;
    }
}

void ibus_panel_impanel_navigate(IBusPanelImpanel *impanel, gboolean start, gboolean forward)
{
    if (start) {
        impanel->selected = -1;
    }

    if (impanel->engineManager->length() < 2) {
        return;
    }

    IBusEngineDesc *engine_desc = nullptr;
    if (impanel->selected < 0) {
        engine_desc = ibus_bus_get_global_engine(impanel->bus);
    } else if (static_cast<size_t>(impanel->selected) < impanel->engineManager->length()) {
        engine_desc = impanel->engineManager->engines()[impanel->selected];
        g_object_ref(engine_desc);
    }

    if (!engine_desc) {
        engine_desc = impanel->engineManager->engines()[0];
        g_object_ref(engine_desc);
    }

    if (engine_desc) {
        const char *name = impanel->engineManager->navigate(engine_desc, forward);
        impanel->selected = impanel->engineManager->getIndexByName(name);
        g_object_unref(engine_desc);
    } else {
        return;
    }

    if (impanel->selected >= 0 && static_cast<size_t>(impanel->selected) < impanel->engineManager->length()) {
        ibus_panel_impanel_real_register_properties(impanel);
    }
}

void ibus_panel_impanel_move_next(IBusPanelImpanel *impanel)
{
    if (impanel->engineManager->length() >= 2) {
        impanel_set_engine(impanel, ibus_engine_desc_get_name(impanel->engineManager->engines()[1]));
    }
}

static GDBusNodeInfo *introspection_data = nullptr;

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

static const char prop_sep[] = ":";

static QByteArray ibus_property_args_to_propstr(const char *key, const char *label, const char *icon, const char *tooltip, const char *hint = "")
{
    QByteArray propstr("/IBus/");
    QByteArray str(key);
    str.replace(':', '!');

    App *app = static_cast<App *>(qApp);

    propstr += str;
    propstr += prop_sep;
    propstr += QByteArray(label).replace(':', '-').constData();
    propstr += prop_sep;
    propstr += app->normalizeIconName(QByteArray(icon).replace(':', '-'));
    propstr += prop_sep;
    propstr += QByteArray(tooltip).replace(':', '-').constData();
    propstr += prop_sep;
    propstr += QByteArray(hint).replace(':', '-').constData();

    return propstr;
}

static QByteArray ibus_engine_desc_to_logo_propstr(IBusEngineDesc *engine)
{
    const gchar *label = "IBus";
    const gchar *tooltip = "";
    const gchar *icon = "input-keyboard";

    gchar xkbLabel[3];
    if (engine) {
        const gchar *iconname = ibus_engine_desc_get_icon(engine);
        if (iconname && iconname[0]) {
            icon = iconname;
        }

        if (strncmp("xkb:", ibus_engine_desc_get_name(engine), 4) == 0) {
            strncpy(xkbLabel, ibus_engine_desc_get_name(engine) + 4, 2);
            xkbLabel[2] = 0;
            int i = 0;
            while (xkbLabel[i]) {
                if (xkbLabel[i] == ':') {
                    xkbLabel[i] = 0;
                }
                i++;
            }
            label = xkbLabel;
            icon = "";
        }

        const gchar *longname = ibus_engine_desc_get_longname(engine);
        if (longname && longname[0]) {
            tooltip = longname;
        }
    }

    return ibus_property_args_to_propstr("Logo", label, icon, tooltip);
}

static QByteArray ibus_property_to_propstr(IBusProperty *property, gboolean useSymbol)
{
    const gchar *label = nullptr;
    const gchar *tooltip = ibus_text_get_text(ibus_property_get_tooltip(property));
    const gchar *icon = ibus_property_get_icon(property);

    if (useSymbol) {
        label = ibus_text_get_text(ibus_property_get_symbol(property));
        if (!label || label[0] == '\0') {
            label = ibus_text_get_text(ibus_property_get_label(property));
        }
    } else {
        label = ibus_text_get_text(ibus_property_get_label(property));
    }

    const char *hint = "";
    if (ibus_property_get_prop_type(property) == PROP_TYPE_TOGGLE) {
        if (ibus_property_get_state(property) != PROP_STATE_CHECKED) {
            hint = "disable";
        }
    } else if (ibus_property_get_prop_type(property) == PROP_TYPE_RADIO) {
        if (ibus_property_get_state(property) == PROP_STATE_CHECKED) {
            hint = "checked";
        }
    }

    return ibus_property_args_to_propstr(ibus_property_get_key(property), label, icon, tooltip, hint);
}

static QByteArray ibus_engine_desc_args_to_propstr(const char *name, const char *language, const char *longname, const char *icon, const char *description)
{
    QByteArray propstr("/IBus/Engine/");
    QByteArray data(name);
    data.replace(':', '!');
    propstr += data;
    propstr += prop_sep;
    if (language) {
        propstr += language;
        propstr += " - ";
    }
    propstr += longname;
    propstr += prop_sep;
    propstr += icon;
    propstr += prop_sep;
    propstr += description;
    return propstr;
}

static QByteArray ibus_engine_desc_to_propstr(IBusEngineDesc *engine_desc)
{
    return ibus_engine_desc_args_to_propstr(ibus_engine_desc_get_name(engine_desc),
                                            ibus_engine_desc_get_language(engine_desc),
                                            ibus_engine_desc_get_longname(engine_desc),
                                            ibus_engine_desc_get_icon(engine_desc),
                                            ibus_engine_desc_get_description(engine_desc));
}

static void impanel_get_default_engine(IBusPanelImpanel *impanel, char ***pengine_names, gsize *plen)
{
    GList *engines = ibus_bus_list_engines(impanel->bus);
    if (!engines) {
        *pengine_names = g_new0(gchar *, 2);
        *plen = 1;
        (*pengine_names)[0] = g_strdup("xkb:us::eng");
        return;
    }

    QList<QByteArray> engineList;
    impanel->xkbLayoutManager->getLayout();
    QStringList layouts = impanel->xkbLayoutManager->defaultLayout().split(QLatin1Char{','});
    QStringList variants = impanel->xkbLayoutManager->defaultVariant().split(QLatin1Char{','});

    for (int i = 0; i < layouts.size(); i++) {
        QString variant;
        if (i < variants.size()) {
            variant = variants[i];
        }

        for (GList *engine = g_list_first(engines); engine != nullptr; engine = g_list_next(engine)) {
            IBusEngineDesc *desc = IBUS_ENGINE_DESC(engine->data);
            QByteArray name = ibus_engine_desc_get_name(desc);
            if (!name.startsWith("xkb:")) {
                continue;
            }

            if (QLatin1String(ibus_engine_desc_get_layout(desc)) == layouts[i] && QLatin1String(ibus_engine_desc_get_layout_variant(desc)) == variant) {
                engineList << name;
            }
        }
    }
    const char *locale = setlocale(LC_CTYPE, nullptr);
    if (!locale) {
        locale = "C";
    }

    QStringList localeList = QString::fromLocal8Bit(locale).split(QLatin1Char{'.'});
    const QString lang = localeList.size() > 0 ? localeList.at(0) : QString{};

    bool added = false;
    for (GList *engine = g_list_first(engines); engine != nullptr; engine = g_list_next(engine)) {
        IBusEngineDesc *desc = IBUS_ENGINE_DESC(engine->data);
        QByteArray name = ibus_engine_desc_get_name(desc);
        if (name.startsWith("xkb:")) {
            continue;
        }

        if (QLatin1String(ibus_engine_desc_get_language(desc)) == lang && ibus_engine_desc_get_rank(desc) > 0) {
            engineList << name;
            added = true;
        }
    }

    if (!added) {
        localeList = QString(lang).split(QLatin1Char{'_'});
        QString _lang = localeList.size() > 0 ? localeList.at(0) : QString{};

        for (GList *engine = g_list_first(engines); engine != nullptr; engine = g_list_next(engine)) {
            IBusEngineDesc *desc = IBUS_ENGINE_DESC(engine->data);
            QByteArray name = ibus_engine_desc_get_name(desc);
            if (name.startsWith("xkb:")) {
                continue;
            }

            if (QLatin1String(ibus_engine_desc_get_language(desc)) == _lang && ibus_engine_desc_get_rank(desc) > 0) {
                engineList << name;
            }
        }
    }

    for (GList *engine = g_list_first(engines); engine != nullptr; engine = g_list_next(engine)) {
        IBusEngineDesc *desc = IBUS_ENGINE_DESC(engine->data);
        g_object_unref(desc);
    }

    g_list_free(engines);

    if (engineList.size() == 0) {
        *pengine_names = g_new0(gchar *, 2);
        *plen = 1;
        (*pengine_names)[0] = g_strdup("xkb:us::eng");
        return;
    } else {
        *pengine_names = g_new0(gchar *, engineList.size() + 1);
        *plen = engineList.size();
        size_t i = 0;
        Q_FOREACH (const QByteArray &name, engineList) {
            (*pengine_names)[i] = g_strdup(name.constData());
            i++;
        }
    }
}

bool contains(gchar **strlist, const gchar *str)
{
    for (; strlist; ++strlist) {
        if (g_strcmp0(*strlist, str) == 0)
            return true;
    }
    return false;
}

static void impanel_update_engines(IBusPanelImpanel *impanel, GVariant *var_engines)
{
    gchar **engine_names = nullptr;
    size_t len = 0;
    if (var_engines) {
        engine_names = g_variant_dup_strv(var_engines, &len);
    }
    if (len == 0) {
        g_strfreev(engine_names);
        engine_names = nullptr;
    }

    if (!engine_names) {
        impanel_get_default_engine(impanel, &engine_names, &len);
        GVariant *var = g_variant_new_strv(engine_names, len);
        g_settings_set_value(impanel->settings_general, "preload-engines", var);
    }

    IBusEngineDesc **engines = ibus_bus_get_engines_by_names(impanel->bus, engine_names);

    impanel->engineManager->setEngines(engines);
    if (engines && engines[0]
        && (!ibus_bus_get_global_engine(impanel->bus) || !contains(engine_names, ibus_engine_desc_get_name(ibus_bus_get_global_engine(impanel->bus))))) {
        ibus_bus_set_global_engine(impanel->bus, ibus_engine_desc_get_name(engines[0]));
    }
    g_strfreev(engine_names);

    impanel->app->setDoGrab(len > 1);
}

static void impanel_update_engines_order(IBusPanelImpanel *impanel, GVariant *var_engines)
{
    const gchar **engine_names = nullptr;
    size_t len = 0;
    engine_names = g_variant_get_strv(var_engines, &len);
    if (len) {
        impanel->engineManager->setOrder(engine_names, len);

        if (impanel->engineManager->engines()) {
            ibus_bus_set_global_engine(impanel->bus, ibus_engine_desc_get_name(impanel->engineManager->engines()[0]));
        }
    }
    g_free(engine_names);
}

static void impanel_update_triggers(IBusPanelImpanel *impanel, GVariant *variant)
{
    gchar **triggers = nullptr;
    size_t len = 0;
    if (variant) {
        triggers = g_variant_dup_strv(variant, &len);
    }
    if (len == 0) {
        g_strfreev(triggers);
        triggers = nullptr;
    }
    if (!triggers) {
        triggers = g_new0(gchar *, 2);
        len = 1;
        triggers[0] = g_strdup("<Super>space");
    }

    QList<QPair<uint, uint>> triggersList;
    for (size_t i = 0; i < len; i++) {
        guint key = 0;
        GdkModifierType mod = (GdkModifierType)0;
        _gtk_accelerator_parse(triggers[i], &key, &mod);
        if (key) {
            triggersList << qMakePair<uint, uint>(key, (uint)mod);
        }
    }
    impanel->app->setTriggerKeys(triggersList);
}

static void impanel_update_use_system_keyboard_layout(IBusPanelImpanel *impanel, GVariant *variant)
{
    impanel->useSystemKeyboardLayout = g_variant_get_boolean(variant);
}
static void impanel_update_use_global_engine(IBusPanelImpanel *impanel, GVariant *variant)
{
    impanel->engineManager->setUseGlobalEngine(g_variant_get_boolean(variant));
}

static void impanel_update_latin_layouts(IBusPanelImpanel *impanel, GVariant *variant)
{
    if (!variant) {
        return;
    }
    gsize length;
    const gchar **variants = g_variant_get_strv(variant, &length);

    impanel->xkbLayoutManager->setLatinLayouts(variants, length);
    g_free(variants);
}

static void impanel_settings_changed_callback(GSettings *settings, const gchar *key, gpointer user_data)
{
    IBusPanelImpanel *impanel = ((IBusPanelImpanel *)user_data);
    gchar *schema = nullptr;
    GVariant *value = g_settings_get_value(settings, key);

    g_object_get(G_OBJECT(settings), "schema", &schema, NULL);

    if (g_strcmp0(schema, IBUS_SCHEMA_GENERAL) == 0 && g_strcmp0(key, "preload-engines") == 0) {
        impanel_update_engines(impanel, value);
    } else if (g_strcmp0(schema, IBUS_SCHEMA_HOTKEY) == 0 && g_strcmp0(key, "triggers") == 0) {
        impanel_update_triggers(impanel, value);
    } else if (g_strcmp0(schema, IBUS_SCHEMA_GENERAL) == 0 && g_strcmp0(key, "use-system-keyboard-layout") == 0) {
        impanel_update_use_system_keyboard_layout(impanel, value);
    } else if (g_strcmp0(schema, IBUS_SCHEMA_GENERAL) == 0 && g_strcmp0(key, "use-global-engine") == 0) {
        impanel_update_use_global_engine(impanel, value);
    }
    g_free(schema);
}

static void impanel_exit_callback(GDBusConnection *connection,
                                  const gchar *sender_name,
                                  const gchar *object_path,
                                  const gchar *interface_name,
                                  const gchar *signal_name,
                                  GVariant *parameters,
                                  gpointer user_data)
{
    Q_UNUSED(connection);
    Q_UNUSED(sender_name);
    Q_UNUSED(object_path);
    Q_UNUSED(interface_name);
    Q_UNUSED(signal_name);
    Q_UNUSED(parameters);
    IBusPanelImpanel *impanel = ((IBusPanelImpanel *)user_data);
    if (impanel->bus) {
        ibus_bus_exit(impanel->bus, FALSE);
    }
}

static void impanel_panel_created_callback(GDBusConnection *connection,
                                           const gchar *sender_name,
                                           const gchar *object_path,
                                           const gchar *interface_name,
                                           const gchar *signal_name,
                                           GVariant *parameters,
                                           gpointer user_data)
{
    Q_UNUSED(connection);
    Q_UNUSED(sender_name);
    Q_UNUSED(object_path);
    Q_UNUSED(interface_name);
    Q_UNUSED(signal_name);
    Q_UNUSED(parameters);
    IBusPanelImpanel *impanel = ((IBusPanelImpanel *)user_data);
    ibus_panel_impanel_real_register_properties(impanel);
}

static void impanel_set_engine(IBusPanelImpanel *impanel, const char *name)
{
    if (!name || !name[0]) {
        return;
    }
    if (ibus_bus_set_global_engine(impanel->bus, name)) {
        if (!impanel->useSystemKeyboardLayout) {
            IBusEngineDesc *engine_desc = ibus_bus_get_global_engine(impanel->bus);
            if (engine_desc) {
                impanel->xkbLayoutManager->setLayout(engine_desc);
            }
            g_object_unref(engine_desc);
        }
        impanel->engineManager->setCurrentEngine(name);
    } else {
        qDebug() << "set engine failed.";
    }
}

static void impanel_trigger_property_callback(GDBusConnection *connection,
                                              const gchar *sender_name,
                                              const gchar *object_path,
                                              const gchar *interface_name,
                                              const gchar *signal_name,
                                              GVariant *parameters,
                                              gpointer user_data)
{
    Q_UNUSED(connection);
    Q_UNUSED(sender_name);
    Q_UNUSED(object_path);
    Q_UNUSED(interface_name);
    Q_UNUSED(signal_name);
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(user_data);
    gchar *s0 = nullptr;
    g_variant_get(parameters, "(s)", &s0);
    if (!s0 || strlen(s0) <= 6)
        return;
    QByteArray prop_key(s0 + 6); // +6 to skip "/IBus/"
    prop_key.replace('!', ':');
    if (g_ascii_strncasecmp(prop_key.constData(), "Logo", 4) == 0)
        ibus_panel_impanel_exec_im_menu(impanel);
    else if (g_ascii_strncasecmp(prop_key.constData(), "Engine/", 7) == 0) {
        impanel_set_engine(impanel, prop_key.constData() + 7);
    } else {
        IBusProperty *property = impanel->propManager->property(prop_key.constData());
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
                } else if (ibus_property_get_prop_type(property) == PROP_TYPE_RADIO) {
                    newstate = PROP_STATE_CHECKED;
                }
                Q_FALLTHROUGH();
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
        } else {
            ibus_panel_service_property_activate((IBusPanelService *)impanel, prop_key.constData(), PROP_STATE_CHECKED);
        }
    }
    g_free(s0);
}

static void impanel_select_candidate_callback(GDBusConnection *connection,
                                              const gchar *sender_name,
                                              const gchar *object_path,
                                              const gchar *interface_name,
                                              const gchar *signal_name,
                                              GVariant *parameters,
                                              gpointer user_data)
{
    Q_UNUSED(connection);
    Q_UNUSED(sender_name);
    Q_UNUSED(object_path);
    Q_UNUSED(interface_name);
    Q_UNUSED(signal_name);

    gint i;
    g_variant_get(parameters, "(i)", &i);
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(user_data);
    ibus_panel_service_candidate_clicked((IBusPanelService *)impanel, i, 0, 0);
}

static void impanel_prev_page_callback(GDBusConnection *connection,
                                       const gchar *sender_name,
                                       const gchar *object_path,
                                       const gchar *interface_name,
                                       const gchar *signal_name,
                                       GVariant *parameters,
                                       gpointer user_data)
{
    Q_UNUSED(connection);
    Q_UNUSED(sender_name);
    Q_UNUSED(object_path);
    Q_UNUSED(interface_name);
    Q_UNUSED(signal_name);
    Q_UNUSED(parameters);

    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(user_data);
    ibus_panel_service_page_up((IBusPanelService *)impanel);
}

static void impanel_next_page_callback(GDBusConnection *connection,
                                       const gchar *sender_name,
                                       const gchar *object_path,
                                       const gchar *interface_name,
                                       const gchar *signal_name,
                                       GVariant *parameters,
                                       gpointer user_data)
{
    Q_UNUSED(connection);
    Q_UNUSED(sender_name);
    Q_UNUSED(object_path);
    Q_UNUSED(interface_name);
    Q_UNUSED(signal_name);
    Q_UNUSED(parameters);

    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(user_data);
    ibus_panel_service_page_down((IBusPanelService *)impanel);
}

static void impanel_configure_callback(GDBusConnection *connection,
                                       const gchar *sender_name,
                                       const gchar *object_path,
                                       const gchar *interface_name,
                                       const gchar *signal_name,
                                       GVariant *parameters,
                                       gpointer user_data)
{
    Q_UNUSED(connection);
    Q_UNUSED(sender_name);
    Q_UNUSED(object_path);
    Q_UNUSED(interface_name);
    Q_UNUSED(signal_name);
    Q_UNUSED(parameters);
    Q_UNUSED(user_data);
    pid_t pid = fork();
    if (pid == 0) {
        execlp("ibus-setup", "ibus-setup", (char *)nullptr);
        exit(0);
    }
}

static void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    Q_UNUSED(name);
    IBusPanelImpanel *impanel = ((IBusPanelImpanel *)user_data);
    impanel->conn = connection;

    g_dbus_connection_register_object(connection,
                                      "/kimpanel",
                                      introspection_data->interfaces[0],
                                      nullptr, /*&interface_vtable*/
                                      nullptr, /* user_data */
                                      nullptr, /* user_data_free_func */
                                      nullptr); /* GError** */

    g_dbus_connection_signal_subscribe(connection,
                                       "org.kde.impanel",
                                       "org.kde.impanel",
                                       "TriggerProperty",
                                       "/org/kde/impanel",
                                       nullptr,
                                       G_DBUS_SIGNAL_FLAGS_NONE,
                                       impanel_trigger_property_callback,
                                       user_data,
                                       nullptr);
    g_dbus_connection_signal_subscribe(connection,
                                       "org.kde.impanel",
                                       "org.kde.impanel",
                                       "SelectCandidate",
                                       "/org/kde/impanel",
                                       nullptr,
                                       G_DBUS_SIGNAL_FLAGS_NONE,
                                       impanel_select_candidate_callback,
                                       user_data,
                                       nullptr);
    g_dbus_connection_signal_subscribe(connection,
                                       "org.kde.impanel",
                                       "org.kde.impanel",
                                       "LookupTablePageUp",
                                       "/org/kde/impanel",
                                       nullptr,
                                       G_DBUS_SIGNAL_FLAGS_NONE,
                                       impanel_prev_page_callback,
                                       user_data,
                                       nullptr);
    g_dbus_connection_signal_subscribe(connection,
                                       "org.kde.impanel",
                                       "org.kde.impanel",
                                       "LookupTablePageDown",
                                       "/org/kde/impanel",
                                       nullptr,
                                       G_DBUS_SIGNAL_FLAGS_NONE,
                                       impanel_next_page_callback,
                                       user_data,
                                       nullptr);
    g_dbus_connection_signal_subscribe(connection,
                                       "org.kde.impanel",
                                       "org.kde.impanel",
                                       "PanelCreated",
                                       "/org/kde/impanel",
                                       nullptr,
                                       G_DBUS_SIGNAL_FLAGS_NONE,
                                       impanel_panel_created_callback,
                                       user_data,
                                       nullptr);
    g_dbus_connection_signal_subscribe(connection,
                                       "org.kde.impanel",
                                       "org.kde.impanel",
                                       "Exit",
                                       "/org/kde/impanel",
                                       nullptr,
                                       G_DBUS_SIGNAL_FLAGS_NONE,
                                       impanel_exit_callback,
                                       user_data,
                                       nullptr);
    g_dbus_connection_signal_subscribe(connection,
                                       "org.kde.impanel",
                                       "org.kde.impanel",
                                       "Configure",
                                       "/org/kde/impanel",
                                       nullptr,
                                       G_DBUS_SIGNAL_FLAGS_NONE,
                                       impanel_configure_callback,
                                       user_data,
                                       nullptr);

    GVariant *var_engines = g_settings_get_value(impanel->settings_general, "preload-engines");
    impanel_update_engines(impanel, var_engines);
    if (var_engines) {
        g_variant_unref(var_engines);
    }

    var_engines = g_settings_get_value(impanel->settings_general, "engines-order");
    if (var_engines) {
        impanel_update_engines_order(impanel, var_engines);
        g_variant_unref(var_engines);
    }

    GVariant *var_triggers = g_settings_get_value(impanel->settings_hotkey, "triggers");
    impanel_update_triggers(impanel, var_triggers);
    if (var_triggers) {
        g_variant_unref(var_triggers);
    }

    GVariant *var_layouts = g_settings_get_value(impanel->settings_general, "xkb-latin-layouts");
    if (var_layouts) {
        impanel_update_latin_layouts(impanel, var_layouts);
        g_variant_unref(var_layouts);
    }

    GVariant *var = g_settings_get_value(impanel->settings_general, "use-system-keyboard-layout");
    if (var) {
        impanel_update_use_system_keyboard_layout(impanel, var);
        g_variant_unref(var);
    }

    var = g_settings_get_value(impanel->settings_general, "use-global-engine");
    if (var) {
        impanel_update_use_global_engine(impanel, var);
        g_variant_unref(var);
    }

    ibus_panel_impanel_real_register_properties(impanel);
}

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    Q_UNUSED(connection);
    Q_UNUSED(name);
    Q_UNUSED(user_data);
}

static void on_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    Q_UNUSED(connection);
    Q_UNUSED(name);
    Q_UNUSED(user_data);
    exit(1);
}

G_DEFINE_TYPE(IBusPanelImpanel, ibus_panel_impanel, IBUS_TYPE_PANEL_SERVICE)

static void ibus_panel_impanel_class_init(IBusPanelImpanelClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    // clang-format off
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
    // clang-format on
}

static void ibus_panel_impanel_init(IBusPanelImpanel *impanel)
{
    impanel->bus = nullptr;
    impanel->app = nullptr;
    impanel->useSystemKeyboardLayout = false;
    impanel->selected = -1;

    introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, nullptr);
    owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
                              "org.kde.kimpanel.inputmethod",
                              G_BUS_NAME_OWNER_FLAGS_REPLACE,
                              on_bus_acquired,
                              on_name_acquired,
                              on_name_lost,
                              impanel,
                              nullptr);

    impanel->propManager = new PropertyManager;
    impanel->engineManager = new EngineManager;
    impanel->xkbLayoutManager = new XkbLayoutManager;
    impanel->settings_general = g_settings_new(IBUS_SCHEMA_GENERAL);
    impanel->settings_hotkey = g_settings_new(IBUS_SCHEMA_HOTKEY);
    g_signal_connect(impanel->settings_general, "changed", G_CALLBACK(impanel_settings_changed_callback), impanel);
    g_signal_connect(impanel->settings_hotkey, "changed", G_CALLBACK(impanel_settings_changed_callback), impanel);
}

static void ibus_panel_impanel_destroy(IBusPanelImpanel *impanel)
{
    delete impanel->propManager;
    impanel->propManager = nullptr;
    delete impanel->engineManager;
    impanel->engineManager = nullptr;
    delete impanel->xkbLayoutManager;
    impanel->xkbLayoutManager = nullptr;

    g_signal_handlers_disconnect_by_func(impanel->settings_general, (gpointer)impanel_settings_changed_callback, impanel);
    g_signal_handlers_disconnect_by_func(impanel->settings_hotkey, (gpointer)impanel_settings_changed_callback, impanel);
    g_clear_object(&impanel->settings_general);
    g_clear_object(&impanel->settings_hotkey);

    g_bus_unown_name(owner_id);
    g_dbus_node_info_unref(introspection_data);

    IBUS_OBJECT_CLASS(ibus_panel_impanel_parent_class)->destroy((IBusObject *)impanel);
}

static void ibus_panel_impanel_focus_in(IBusPanelService *panel, const gchar *input_context_path)
{
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);
    if (impanel->app->keyboardGrabbed()) {
        return;
    }

    auto engine_desc = ibus_bus_get_global_engine(impanel->bus);
    if (engine_desc) {
        impanel_update_logo_by_engine(impanel, engine_desc);
        g_object_unref(engine_desc);
    }

    impanel->engineManager->setCurrentContext(input_context_path);
    if (!impanel->engineManager->useGlobalEngine()) {
        impanel_set_engine(impanel, impanel->engineManager->currentEngine().toUtf8().constData());
    }
}

static void ibus_panel_impanel_focus_out(IBusPanelService *panel, const gchar *input_context_path)
{
    Q_UNUSED(panel);
    Q_UNUSED(input_context_path);
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);

    if (impanel->app->keyboardGrabbed()) {
        return;
    }

    if (impanel->engineManager->useGlobalEngine()) {
        return;
    }
    impanel->engineManager->setCurrentContext("");
}

static void ibus_panel_impanel_register_properties(IBusPanelService *panel, IBusPropList *prop_list)
{
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);
    impanel->propManager->setProperties(prop_list);
    ibus_panel_impanel_real_register_properties(impanel);
}

static void ibus_panel_impanel_real_register_properties(IBusPanelImpanel *impanel)
{
    if (!impanel->conn)
        return;

    IBusProperty *property = nullptr;
    guint i = 0;

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));

    if (impanel->selected >= 0 && static_cast<size_t>(impanel->selected) < impanel->engineManager->length()) {
        auto engine_desc = impanel->engineManager->engines()[impanel->selected];
        QByteArray propstr = ibus_engine_desc_to_logo_propstr(engine_desc);
        g_variant_builder_add(&builder, "s", propstr.constData());
    } else {
        QByteArray propstr;
        auto engine_desc = ibus_bus_get_global_engine(impanel->bus);
        if (engine_desc) {
            propstr = ibus_engine_desc_to_logo_propstr(engine_desc);
            g_variant_builder_add(&builder, "s", propstr.constData());
            g_object_unref(engine_desc);
        }

        IBusPropList *prop_list = impanel->propManager->properties();
        if (prop_list) {
            while ((property = ibus_prop_list_get(prop_list, i)) != nullptr) {
                propstr = ibus_property_to_propstr(property, TRUE);
                g_variant_builder_add(&builder, "s", propstr.constData());
                ++i;
            }
        }
    }

    g_dbus_connection_emit_signal(impanel->conn,
                                  nullptr,
                                  "/kimpanel",
                                  "org.kde.kimpanel.inputmethod",
                                  "RegisterProperties",
                                  (g_variant_new("(as)", &builder)),
                                  nullptr);
}

static void ibus_panel_impanel_set_cursor_location(IBusPanelService *panel, gint x, gint y, gint w, gint h)
{
    g_dbus_connection_call(IBUS_PANEL_IMPANEL(panel)->conn,
                           "org.kde.impanel",
                           "/org/kde/impanel",
                           "org.kde.impanel2",
                           "SetSpotRect",
                           (g_variant_new("(iiii)", x, y, w, h)),
                           nullptr,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1, /* timeout */
                           nullptr,
                           nullptr,
                           nullptr);
}

static void ibus_panel_impanel_update_auxiliary_text(IBusPanelService *panel, IBusText *text, gboolean visible)
{
    const gchar *t = ibus_text_get_text(text);
    const gchar *attr = "";
    IBusPanelImpanel *impanel = (IBusPanelImpanel *)panel;

    if (!impanel->conn)
        return;

    g_dbus_connection_emit_signal(impanel->conn, nullptr, "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateAux", (g_variant_new("(ss)", t, attr)), nullptr);

    if (visible == 0)
        ibus_panel_impanel_hide_auxiliary_text(panel);
    else
        ibus_panel_impanel_show_auxiliary_text(panel);
}

static void ibus_panel_impanel_update_lookup_table(IBusPanelService *panel, IBusLookupTable *lookup_table, gboolean visible)
{
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);
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

    gchar label[16][4]; // WARNING large enough I think --- nihui
    const gchar *candidate;

    GVariantBuilder builder_labels;
    GVariantBuilder builder_candidates;
    GVariantBuilder builder_attrs;
    g_variant_builder_init(&builder_labels, G_VARIANT_TYPE("as"));
    g_variant_builder_init(&builder_candidates, G_VARIANT_TYPE("as"));
    g_variant_builder_init(&builder_attrs, G_VARIANT_TYPE("as"));

    const gchar *attr = "";
    for (i = start; i < end; i++) {
        g_snprintf(label[i - start], 4, "%d", (i - start + 1) % 10);
        // NOTE ibus always return NULL for ibus_lookup_table_get_label
        //         label = ibus_lookup_table_get_label(lookup_table, i)->text;
        g_variant_builder_add(&builder_labels, "s", label[i - start]);

        candidate = ibus_text_get_text(ibus_lookup_table_get_candidate(lookup_table, i));
        g_variant_builder_add(&builder_candidates, "s", candidate);

        g_variant_builder_add(&builder_attrs, "s", attr);
    }

    gboolean has_prev = 1;
    gboolean has_next = 1;

    guint cursor_pos_in_page;
    if (ibus_lookup_table_is_cursor_visible(lookup_table))
        cursor_pos_in_page = cursor_pos % page_size;
    else
        cursor_pos_in_page = -1;

    gint orientation = ibus_lookup_table_get_orientation(lookup_table);
    if (orientation == IBUS_ORIENTATION_HORIZONTAL) {
        orientation = 2;
    } else if (orientation == IBUS_ORIENTATION_VERTICAL) {
        orientation = 1;
    } else {
        orientation = 0;
    }

    g_dbus_connection_call(
        impanel->conn,
        "org.kde.impanel",
        "/org/kde/impanel",
        "org.kde.impanel2",
        "SetLookupTable",
        (g_variant_new("(asasasbbii)", &builder_labels, &builder_candidates, &builder_attrs, has_prev, has_next, cursor_pos_in_page, orientation)),
        nullptr,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        nullptr,
        nullptr);

    if (visible == 0)
        ibus_panel_impanel_hide_lookup_table(panel);
    else
        ibus_panel_impanel_show_lookup_table(panel);
}

static void ibus_panel_impanel_update_preedit_text(IBusPanelService *panel, IBusText *text, guint cursor_pos, gboolean visible)
{
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;

    const gchar *t = ibus_text_get_text(text);
    const gchar *attr = "";

    g_dbus_connection_emit_signal(impanel->conn,
                                  nullptr,
                                  "/kimpanel",
                                  "org.kde.kimpanel.inputmethod",
                                  "UpdatePreeditText",
                                  (g_variant_new("(ss)", t, attr)),
                                  nullptr);

    g_dbus_connection_emit_signal(impanel->conn,
                                  nullptr,
                                  "/kimpanel",
                                  "org.kde.kimpanel.inputmethod",
                                  "UpdatePreeditCaret",
                                  (g_variant_new("(i)", cursor_pos)),
                                  nullptr);

    if (visible == 0)
        ibus_panel_impanel_hide_preedit_text(panel);
    else
        ibus_panel_impanel_show_preedit_text(panel);
}

static void ibus_panel_impanel_update_property(IBusPanelService *panel, IBusProperty *prop)
{
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;

    impanel->propManager->updateProperty(prop);

    QByteArray propstr = ibus_property_to_propstr(prop, TRUE);

    g_dbus_connection_emit_signal(impanel->conn,
                                  nullptr,
                                  "/kimpanel",
                                  "org.kde.kimpanel.inputmethod",
                                  "UpdateProperty",
                                  (g_variant_new("(s)", propstr.constData())),
                                  nullptr);
}

static void ibus_panel_impanel_cursor_down_lookup_table(IBusPanelService *panel)
{
    Q_UNUSED(panel);
}

static void ibus_panel_impanel_cursor_up_lookup_table(IBusPanelService *panel)
{
    Q_UNUSED(panel);
}

static void ibus_panel_impanel_hide_auxiliary_text(IBusPanelService *panel)
{
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;
    gboolean toShow = 0;

    g_dbus_connection_emit_signal(impanel->conn, nullptr, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowAux", (g_variant_new("(b)", toShow)), nullptr);
}

static void ibus_panel_impanel_hide_language_bar(IBusPanelService *panel)
{
    Q_UNUSED(panel);
}

static void ibus_panel_impanel_hide_lookup_table(IBusPanelService *panel)
{
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;
    gboolean toShow = 0;

    g_dbus_connection_emit_signal(impanel->conn,
                                  nullptr,
                                  "/kimpanel",
                                  "org.kde.kimpanel.inputmethod",
                                  "ShowLookupTable",
                                  (g_variant_new("(b)", toShow)),
                                  nullptr);
}

static void ibus_panel_impanel_hide_preedit_text(IBusPanelService *panel)
{
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;
    gboolean toShow = 0;

    g_dbus_connection_emit_signal(impanel->conn, nullptr, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowPreedit", (g_variant_new("(b)", toShow)), nullptr);
}

static void ibus_panel_impanel_page_down_lookup_table(IBusPanelService *panel)
{
    Q_UNUSED(panel);
}

static void ibus_panel_impanel_page_up_lookup_table(IBusPanelService *panel)
{
    Q_UNUSED(panel);
}

static void ibus_panel_impanel_reset(IBusPanelService *panel)
{
    Q_UNUSED(panel);
}

static void ibus_panel_impanel_show_auxiliary_text(IBusPanelService *panel)
{
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;
    gboolean toShow = 1;

    g_dbus_connection_emit_signal(impanel->conn, nullptr, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowAux", (g_variant_new("(b)", toShow)), nullptr);
}

static void ibus_panel_impanel_show_language_bar(IBusPanelService *panel)
{
    Q_UNUSED(panel);
}

static void ibus_panel_impanel_show_lookup_table(IBusPanelService *panel)
{
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;
    gboolean toShow = 1;

    g_dbus_connection_emit_signal(impanel->conn,
                                  nullptr,
                                  "/kimpanel",
                                  "org.kde.kimpanel.inputmethod",
                                  "ShowLookupTable",
                                  (g_variant_new("(b)", toShow)),
                                  nullptr);
}

static void ibus_panel_impanel_show_preedit_text(IBusPanelService *panel)
{
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;
    gboolean toShow = 1;

    g_dbus_connection_emit_signal(impanel->conn, nullptr, "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowPreedit", (g_variant_new("(b)", toShow)), nullptr);
}

static void ibus_panel_impanel_start_setup(IBusPanelService *panel)
{
    Q_UNUSED(panel);
}

static void ibus_panel_impanel_state_changed(IBusPanelService *panel)
{
    IBusPanelImpanel *impanel = IBUS_PANEL_IMPANEL(panel);
    if (!impanel->conn)
        return;

    if (impanel->app->keyboardGrabbed()) {
        return;
    }

    IBusEngineDesc *engine_desc = ibus_bus_get_global_engine(impanel->bus);
    if (!engine_desc) {
        return;
    }

    impanel_update_logo_by_engine(impanel, engine_desc);

    g_dbus_connection_emit_signal(impanel->conn, nullptr, "/kimpanel", "org.kde.kimpanel.inputmethod", "Enable", (g_variant_new("(b)", TRUE)), nullptr);

    impanel->engineManager->moveToFirst(engine_desc);
    QStringList engineList = impanel->engineManager->engineOrder();

    gchar **engine_names = g_new0(gchar *, engineList.size() + 1);
    size_t i = 0;
    Q_FOREACH (const QString &name, engineList) {
        engine_names[i] = g_strdup(name.toUtf8().constData());
        i++;
    }

    GVariant *var = g_variant_new_strv(engine_names, engineList.size());
    g_settings_set_value(impanel->settings_general, "engines-order", var);
    g_strfreev(engine_names);
    g_object_unref(engine_desc);
}

static void ibus_panel_impanel_exec_menu(IBusPanelImpanel *impanel, IBusPropList *prop_list)
{
    if (!impanel->conn)
        return;

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));

    int i = 0;
    while (true) {
        IBusProperty *prop = ibus_prop_list_get(prop_list, i);
        if (!prop)
            break;
        QByteArray propstr = ibus_property_to_propstr(prop);
        g_variant_builder_add(&builder, "s", propstr.constData());
        i++;
    }

    g_dbus_connection_emit_signal(impanel->conn, nullptr, "/kimpanel", "org.kde.kimpanel.inputmethod", "ExecMenu", (g_variant_new("(as)", &builder)), nullptr);
}

static void ibus_panel_impanel_exec_im_menu(IBusPanelImpanel *impanel)
{
    if (!impanel->conn)
        return;

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));

    IBusEngineDesc **engines = impanel->engineManager->engines();
    if (engines) {
        int i = 0;
        while (engines[i]) {
            QByteArray propstr = ibus_engine_desc_to_propstr(engines[i]);
            g_variant_builder_add(&builder, "s", propstr.constData());
            i++;
        }
    }

    g_dbus_connection_emit_signal(impanel->conn, nullptr, "/kimpanel", "org.kde.kimpanel.inputmethod", "ExecMenu", (g_variant_new("(as)", &builder)), nullptr);
}

IBusPanelImpanel *ibus_panel_impanel_new(GDBusConnection *connection)
{
    IBusPanelImpanel *panel;
    panel = (IBusPanelImpanel *)g_object_new(IBUS_TYPE_PANEL_IMPANEL, "object-path", IBUS_PATH_PANEL, "connection", connection, NULL);
    return panel;
}
