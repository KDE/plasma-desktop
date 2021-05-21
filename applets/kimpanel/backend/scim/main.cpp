/*
    SPDX-FileCopyrightText: 2009 Wang Hoi <zealot.hoi@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/** @file scim_panel_dbus.cpp
 */

#define KDE_signal signal

#undef QT_NO_STL
#define QT_STL

#define QT_NO_KEYWORDS

#include <config-scim.h>
#include <errno.h>
#include <list>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QThread>

#define Uses_C_STDIO
#define Uses_C_STDLIB
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_SOCKET
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_TRANS_COMMANDS
#define Uses_SCIM_CONFIG
#define Uses_SCIM_CONFIG_MODULE
//#define Uses_SCIM_DEBUG
#define Uses_SCIM_HELPER
#define Uses_SCIM_HELPER_MODULE
#define Uses_SCIM_PANEL_AGENT
#define Uses_STL_MAP

#define ENABLE_DEBUG 9
// scim use exceptions and template, so it doesn't work in hidden visibility
#pragma GCC visibility push(default)
#include <scim.h>
#pragma GCC visibility pop

Q_DECLARE_METATYPE(scim::Property)
Q_DECLARE_METATYPE(scim::PanelFactoryInfo)
Q_DECLARE_METATYPE(scim::HelperInfo)

using namespace scim;

// PanelAgent related functions
static bool initialize_panel_agent(const String &config, const String &display, bool resident);
static bool run_panel_agent(void);
static void start_auto_start_helpers(void);

static void slot_transaction_start(void);
static void slot_transaction_end(void);
static void slot_reload_config(void);
static void slot_turn_on(void);
static void slot_turn_off(void);
static void slot_update_screen(int screen);
static void slot_update_spot_location(int x, int y);
static void slot_update_factory_info(const PanelFactoryInfo &info);
static void slot_show_help(const String &help);
static void slot_show_factory_menu(const std::vector<PanelFactoryInfo> &menu);

static void slot_show_preedit_string(void);
static void slot_show_aux_string(void);
static void slot_show_lookup_table(void);
static void slot_hide_preedit_string(void);
static void slot_hide_aux_string(void);
static void slot_hide_lookup_table(void);
static void slot_update_preedit_string(const String &str, const AttributeList &attrs);
static void slot_update_preedit_caret(int caret);
static void slot_update_aux_string(const String &str, const AttributeList &attrs);
static void slot_update_lookup_table(const LookupTable &table);
static void slot_register_properties(const PropertyList &props);
static void slot_update_property(const Property &prop);

static void slot_register_helper_properties(int id, const PropertyList &props);
static void slot_update_helper_property(int id, const Property &prop);
static void slot_register_helper(int id, const HelperInfo &helper);
static void slot_remove_helper(int id);
static void slot_lock(void);
static void slot_unlock(void);

/////////////////////////////////////////////////////////////////////////////
// Declaration of internal variables.
/////////////////////////////////////////////////////////////////////////////
// static bool               _ui_initialized              = false;

static ConfigModule *_config_module = nullptr;
static ConfigPointer _config;

static std::vector<HelperInfo> _helper_list;

static bool _should_exit = false;

// static bool               _panel_is_on                 = false;

static PanelAgent *_panel_agent = nullptr;

static QList<PanelFactoryInfo> _factory_list;

class PanelAgentThread;
PanelAgentThread *_panel_agent_thread;

class DBusHandler;
DBusHandler *_dbus_handler;

static QMutex _panel_agent_lock;
static QMutex _global_resource_lock;
static QMutex _transaction_lock;

/////////////////////////////////////////////////////////////////////////////
// Implementation of internal functions.
/////////////////////////////////////////////////////////////////////////////

static void ui_config_reload_callback(const ConfigPointer &config)
{
    _config = config;
}

static QString AttrList2String(const AttributeList &attr_list)
{
    QString result;
    Q_FOREACH (const Attribute &attr, attr_list) {
        int type = (int)attr.get_type();
        unsigned int start = attr.get_start();
        unsigned int length = attr.get_length();
        unsigned int value = attr.get_value();
        result += QString("%1:%2:%3:%4;").arg(type).arg(start).arg(length).arg(value);
    }
    return result;
}

static QVariantList LookupTable2VariantList(const LookupTable &lookup_table)
{
    QVariantList result;
    QStringList labels;
    QStringList candidates;
    QStringList attrlist_list;
    int current_page_size = lookup_table.get_current_page_size();

    for (int i = 0; i < current_page_size; ++i) {
        labels << QString::fromStdWString(lookup_table.get_candidate_label(i));
    }
    for (int i = 0; i < current_page_size; ++i) {
        candidates << QString::fromStdWString(lookup_table.get_candidate_in_current_page(i));
        attrlist_list << AttrList2String(lookup_table.get_attributes_in_current_page(i));
    }

    result << labels << candidates << attrlist_list << lookup_table.get_current_page_start()
           << (lookup_table.get_current_page_start() + lookup_table.get_current_page_size() < ((int)lookup_table.number_of_candidates()));

    // result << labels << candidates << attrlist_list;
    return result;
}

static QString Property2String(const Property &prop)
{
    QString result;
    int state = 0;
    if (prop.active())
        state |= SCIM_PROPERTY_ACTIVE;
    if (prop.visible())
        state |= SCIM_PROPERTY_VISIBLE;
    result = QString("%1:%2:%3:%4:%5")
                 .arg(QString::fromUtf8(prop.get_key().c_str()))
                 .arg(QString::fromUtf8(prop.get_label().c_str()))
                 .arg(QString::fromUtf8(prop.get_icon().c_str()))
                 .arg(QString::fromUtf8(prop.get_tip().c_str()))
                 .arg(state);
    return result;
}

#if 0
static QString
PanelFactoryInfo2String(const PanelFactoryInfo &info)
{
    QString result = QString("%1:%2:%3:%4")
                     .arg(QString::fromUtf8(info.uuid.c_str()))
                     .arg(QString::fromUtf8(info.name.c_str()))
                     .arg(QString::fromUtf8(info.icon.c_str()))
                     .arg(QString::fromUtf8(info.lang.c_str()));
    return result;
}
#endif

static QStringList PropertyList2LeafOnlyStringList(const QList<Property> &props)
{
    QStringList list;
    list.clear();
    for (int i = 0; i < props.size(); ++i) {
        bool ok = true;
        for (int j = 0; j < props.size(); ++j) {
            if (props.at(i).is_a_leaf_of(props.at(j))) {
                SCIM_DEBUG_MAIN(1) << "Leaf" << props.at(i).get_key() << " " << props.at(j).get_key() << "\n";
                ok = false;
            }
        }
        if (ok)
            list << Property2String(props.at(i));
    }
    return list;
}

static int dbus_event_type = QEvent::registerEventType(1988);
class DBusEvent : public QEvent
{
public:
    enum SCIM_EVENT_TYPE {
        TURN_ON,
        TURN_OFF,
        UP_SCREEN,
        UP_SPOT_LOC,
        UP_AUX,
        UP_PREEDIT_STR,
        UP_PREEDIT_CARET,
        UP_LOOKUPTABLE,
        UP_LOOKUPTABLE_CURSOR,
        UP_FACTORY_INFO,
        UP_PROPERTY,
        UP_HELPER_PROPERTY,
        REG_PROPERTIES,
        REG_HELPER_PROPERTIES,
        REG_HELPER,
        RM_HELPER,
        RM_PROPERTY,
        SHOW_LOOKUPTABLE,
        HIDE_LOOKUPTABLE,
        SHOW_PREEDIT,
        HIDE_PREEDIT,
        SHOW_AUX,
        HIDE_AUX,
        SHOW_HELP,
        SHOW_FACTORY_MENU,
    };
    DBusEvent(SCIM_EVENT_TYPE t, const QVariantList &arglist = QVariantList())
        : QEvent((QEvent::Type)dbus_event_type)
        , m_evtype(t)
        , m_data(arglist)
    {
    }
    SCIM_EVENT_TYPE scim_event_type() const
    {
        return m_evtype;
    }
    QVariantList data() const
    {
        return m_data;
    }

private:
    SCIM_EVENT_TYPE m_evtype;
    QVariantList m_data;
};
class DBusHandler : public QObject
{
    Q_OBJECT
public:
    explicit DBusHandler(QObject *parent = nullptr)
        : QObject(parent)
    {
        QDBusConnection("scim_panel").connect("", "", "org.kde.impanel", "MovePreeditCaret", this, SLOT(MovePreeditCaret(int)));
        QDBusConnection("scim_panel").connect("", "", "org.kde.impanel", "SelectCandidate", this, SLOT(SelectCandidate(int)));
        QDBusConnection("scim_panel").connect("", "", "org.kde.impanel", "LookupTablePageUp", this, SLOT(LookupTablePageUp()));
        QDBusConnection("scim_panel").connect("", "", "org.kde.impanel", "LookupTablePageDown", this, SLOT(LookupTablePageDown()));
        QDBusConnection("scim_panel").connect("", "", "org.kde.impanel", "TriggerProperty", this, SLOT(TriggerProperty(QString)));
        QDBusConnection("scim_panel").connect("", "", "org.kde.impanel", "PanelCreated", this, SLOT(PanelCreated()));
        QDBusConnection("scim_panel").connect("", "", "org.kde.impanel", "Exit", this, SLOT(Exit()));
        QDBusConnection("scim_panel").connect("", "", "org.kde.impanel", "ReloadConfig", this, SLOT(ReloadConfig()));
        QDBusConnection("scim_panel").connect("", "", "org.kde.impanel", "Configure", this, SLOT(Configure()));

        logo_prop = Property("/Logo", "SCIM", String(SCIM_ICON_DIR) + "/trademark.png", "SCIM Input Method");
        show_help_prop = Property("/StartHelp", "Help", "help-about", "About SCIM…");
        factory_prop_prefix = QString::fromUtf8("/Factory/");
        helper_prop_prefix = QString::fromUtf8("/Helper/");
    }
    ~DBusHandler()
    {
    }

    void setInitialHelpers(const std::vector<HelperInfo> &_helper_list)
    {
        QList<Property> props;
        Q_FOREACH (const HelperInfo &info, _helper_list) {
            if (((info.option & SCIM_HELPER_STAND_ALONE) != 0) && ((info.option & SCIM_HELPER_AUTO_START) == 0)) {
                props << Property(String(helper_prop_prefix.toUtf8().constData()) + info.uuid, info.name, info.icon, info.description);
            }
        }
        if (!props.isEmpty()) {
            helper_props_map.insert(0, props);
        }
    }
public Q_SLOTS:
    void MovePreeditCaret(int pos)
    {
        SCIM_DEBUG_MAIN(1) << Q_FUNC_INFO << pos << "\n";
        _panel_agent->move_preedit_caret(pos);
    }
    void SelectCandidate(int idx)
    {
        SCIM_DEBUG_MAIN(1) << Q_FUNC_INFO << idx << "\n";
        _panel_agent->select_candidate(idx);
    }
    void LookupTablePageUp()
    {
        SCIM_DEBUG_MAIN(1) << Q_FUNC_INFO << "\n";
        _panel_agent->lookup_table_page_up();
    }
    void LookupTablePageDown()
    {
        SCIM_DEBUG_MAIN(1) << Q_FUNC_INFO << "\n";
        _panel_agent->lookup_table_page_down();
    }
    void TriggerProperty(const QString &key)
    {
        SCIM_DEBUG_MAIN(1) << qPrintable(key) << "\n";
        if (key == show_help_prop.get_key().c_str()) {
            SCIM_DEBUG_MAIN(1) << "about_to_show_help"
                               << "\n";
            _panel_agent->request_help();
            return;
        }
        if (key == logo_prop.get_key().c_str()) {
            SCIM_DEBUG_MAIN(1) << "about_to_show_factory_menu"
                               << "\n";
            _panel_agent->request_factory_menu();
            return;
        }
        if (key.startsWith(factory_prop_prefix)) {
            QString factory_uuid = key;
            factory_uuid.remove(0, factory_prop_prefix.size());
            SCIM_DEBUG_MAIN(1) << "about_to_change_factory" << qPrintable(factory_uuid) << "\n";
            _panel_agent->change_factory(factory_uuid.toUtf8().constData());
            return;
        }
        if (key.startsWith(helper_prop_prefix)) {
            QString helper_uuid = key;
            helper_uuid.remove(0, helper_prop_prefix.size());
            SCIM_DEBUG_MAIN(1) << "about_to_start_helper" << qPrintable(helper_uuid) << "\n";
            _panel_agent->start_helper(helper_uuid.toUtf8().constData());
            return;
        }
        int i = 0;
        for (i = 0; i < panel_props.size(); ++i) {
            if (panel_props.at(i).get_key() == String(key.toUtf8().constData())) {
                break;
            }
        }
        // found one
        QStringList list_result;
        list_result.clear();
        if (i < panel_props.size()) {
            for (int j = 0; j < panel_props.size(); ++j) {
                if (panel_props.at(j).is_a_leaf_of(panel_props.at(i))) {
                    list_result << Property2String(panel_props.at(j));
                }
            }
            if (list_result.isEmpty()) {
                _panel_agent->trigger_property(key.toUtf8().constData());
            } else {
                SCIM_DEBUG_MAIN(1) << "ExecMenu" << qPrintable(key) << "\n";
                QDBusMessage message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "ExecMenu");
                message << list_result;
                QDBusConnection("scim_panel").send(message);
            }
        }
    }
    void PanelCreated()
    {
        QDBusMessage message;
        QStringList list_result;

        list_result.clear();
        list_result << Property2String(logo_prop);
        list_result << PropertyList2LeafOnlyStringList(panel_props);

        Q_FOREACH (const QList<Property> &props, helper_props_map) {
            list_result << PropertyList2LeafOnlyStringList(props);
        }

        list_result << Property2String(show_help_prop);

        message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "RegisterProperties");

        message << list_result;
        QDBusConnection("scim_panel").send(message);
    }
    void Exit()
    {
        SCIM_DEBUG_MAIN(1) << Q_FUNC_INFO << "\n";
        _panel_agent->exit();
        _panel_agent->stop();
        exit(1);
    }
    void ReloadConfig()
    {
        SCIM_DEBUG_MAIN(1) << Q_FUNC_INFO << "\n";
        _panel_agent->reload_config();
        if (!_config.null())
            _config->reload();
    }
    void Configure()
    {
        SCIM_DEBUG_MAIN(1) << Q_FUNC_INFO << "\n";
        QProcess::startDetached("scim-setup");
    }

protected:
    bool event(QEvent *e) override
    {
        QStringList list_result;
        QList<Property> prop_list;
        if (e->type() == dbus_event_type) {
            DBusEvent *ev = (DBusEvent *)e;

            QDBusMessage message;

            switch (ev->scim_event_type()) {
            case DBusEvent::TURN_ON:
                /*
                                list_result.clear();
                                list_result << Property2String(logo_prop);
                                list_result << PropertyList2LeafOnlyStringList(panel_props);
                                Q_FOREACH(const QList<Property> &props, helper_props_map.values()) {
                                    list_result << PropertyList2LeafOnlyStringList(props);
                                }
                                list_result << Property2String(show_help_prop);

                                message = QDBusMessage::createSignal("/kimpanel",
                                    "org.kde.kimpanel.inputmethod",
                                    "RegisterProperties");

                                message << list_result;
                                QDBusConnection("scim_panel").send(message);
                */
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "Enable");
                message << true;
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::TURN_OFF:
                logo_prop.set_icon(String(SCIM_ICON_DIR) + "/trademark.png");

                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateProperty");
                message << Property2String(logo_prop);
                QDBusConnection("scim_panel").send(message);
                /*
                                list_result.clear();
                                list_result << Property2String(logo_prop);
                                Q_FOREACH (const QList<Property> &prop_list, helper_props_map.values()) {
                                    list_result << PropertyList2LeafOnlyStringList(prop_list);
                                }
                                list_result << Property2String(show_help_prop);

                                message = QDBusMessage::createSignal("/kimpanel",
                                    "org.kde.kimpanel.inputmethod",
                                    "RegisterProperties");

                                message << list_result;

                                QDBusConnection("scim_panel").send(message);
                */
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "Enable");
                message << false;
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::SHOW_HELP:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "ExecDialog");
                message << Property2String(Property("/Help", "Help", "", ev->data().at(0).toString().toUtf8().constData()));
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::SHOW_FACTORY_MENU:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "ExecMenu");
                // X                 _factory_list.clear();
                list_result.clear();
                Q_FOREACH (const QVariant &v, ev->data()) {
                    PanelFactoryInfo factory_info = v.value<PanelFactoryInfo>();
                    // X                     _factory_list << factory_info;
                    list_result << Property2String(Property(String(factory_prop_prefix.toUtf8().constData()) + factory_info.uuid,
                                                            factory_info.name,
                                                            factory_info.icon,
                                                            factory_info.lang));
                }
                message << list_result;
                SCIM_DEBUG_MAIN(1) << qPrintable(list_result.join(";")) << "\n";
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::SHOW_PREEDIT:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "ShowPreedit");
                message << true;
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::SHOW_AUX:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "ShowAux");
                message << true;
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::SHOW_LOOKUPTABLE:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "ShowLookupTable");
                message << true;
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::HIDE_PREEDIT:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "ShowPreedit");
                message << false;
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::HIDE_AUX:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "ShowAux");
                message << false;
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::HIDE_LOOKUPTABLE:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "ShowLookupTable");
                message << false;
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::UP_PROPERTY:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateProperty");
                message << Property2String(ev->data().at(0).value<Property>());
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::UP_LOOKUPTABLE:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateLookupTable");
                message << ev->data().at(0).toStringList();
                message << ev->data().at(1).toStringList();
                message << ev->data().at(2).toStringList();
                message << ev->data().at(3).toBool();
                message << ev->data().at(4).toBool();
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::UP_LOOKUPTABLE_CURSOR:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateLookupTableCursor");
                message << ev->data().at(0).toInt();
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::UP_PREEDIT_CARET:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "UpdatePreeditCaret");
                message << ev->data().at(0).toInt();
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::UP_PREEDIT_STR:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "UpdatePreeditText");
                message << ev->data().at(0).toString();
                message << ev->data().at(1).toString();
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::UP_AUX:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateAux");
                message << ev->data().at(0).toString();
                message << ev->data().at(1).toString();
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::UP_SPOT_LOC:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateSpotLocation");
                message << ev->data().at(0).toInt();
                message << ev->data().at(1).toInt();
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::UP_SCREEN:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateScreen");
                message << ev->data().at(0).toInt();
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::UP_FACTORY_INFO:
                SCIM_DEBUG_MAIN(1) << "update factory info\n";

                logo_prop.set_icon(ev->data().at(0).value<PanelFactoryInfo>().icon);
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateProperty");
                message << Property2String(logo_prop);
                QDBusConnection("scim_panel").send(message);
                break;
            case DBusEvent::REG_HELPER:
                break;
            case DBusEvent::REG_HELPER_PROPERTIES:
                helper_props_map.clear();
                prop_list.clear();
                Q_FOREACH (const QVariant &v, ev->data().at(1).toList()) {
                    Property prop = v.value<Property>();
                    prop_list << prop;
                }
                helper_props_map.insert(ev->data().at(0).toInt(), prop_list);

                list_result.clear();
                list_result << Property2String(logo_prop);
                list_result << PropertyList2LeafOnlyStringList(panel_props);
                Q_FOREACH (const QList<Property> &props, helper_props_map) {
                    list_result << PropertyList2LeafOnlyStringList(props);
                }
                list_result << Property2String(show_help_prop);

                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "RegisterProperties");

                message << list_result;
                QDBusConnection("scim_panel").send(message);

                break;
            case DBusEvent::REG_PROPERTIES:
                panel_props.clear();
                Q_FOREACH (const QVariant &v, ev->data()) {
                    Property prop = v.value<Property>();
                    panel_props << prop;
                    SCIM_DEBUG_MAIN(1) << "REG_PROPERTIES" << qPrintable(Property2String(v.value<Property>())) << "\n";
                }

                list_result.clear();
                list_result << Property2String(logo_prop);
                list_result << PropertyList2LeafOnlyStringList(panel_props);
                SCIM_DEBUG_MAIN(1) << "SIMPLIFY_PROPS " << panel_props.size() << " " << list_result.size() - 1 << "\n";
                Q_FOREACH (const QList<Property> &props, helper_props_map) {
                    list_result << PropertyList2LeafOnlyStringList(props);
                }
                list_result << Property2String(show_help_prop);

                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "RegisterProperties");

                message << list_result;
                QDBusConnection("scim_panel").send(message);

                break;
            case DBusEvent::RM_HELPER:
                helper_props_map.remove(ev->data().at(0).toInt());
                list_result.clear();
                list_result << Property2String(logo_prop);
                list_result << PropertyList2LeafOnlyStringList(panel_props);
                Q_FOREACH (const QList<Property> &props, helper_props_map) {
                    list_result << PropertyList2LeafOnlyStringList(props);
                }
                list_result << Property2String(show_help_prop);

                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "RegisterProperties");

                message << list_result;
                QDBusConnection("scim_panel").send(message);

                break;
            case DBusEvent::RM_PROPERTY:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "RemoveProperty");
                // message << ev->data().at(0).V();
                QDBusConnection("scim_panel").send(message);
                break;
            default:
                message = QDBusMessage::createSignal("/kimpanel", "org.kde.kimpanel.inputmethod", "ImplementMe");
                // message << ev->data().at(0).toInt();
                SCIM_DEBUG_MAIN(1) << "Implement me:" << ev->scim_event_type() << "\n";
                QDBusConnection("scim_panel").send(message);
                break;
            }

            return true;
        }
        return QObject::event(e);
    }

private:
    QList<Property> panel_props;
    QMap<int, QList<Property>> helper_props_map;
    Property logo_prop;
    Property show_help_prop;
    QString factory_prop_prefix;
    QString helper_prop_prefix;
    QStringList cached_panel_props;
};

class PanelAgentThread : public QThread
{
    Q_OBJECT
public:
    explicit PanelAgentThread(QObject *parent = nullptr)
        : QThread(parent)
    {
    }
    ~PanelAgentThread()
    {
    }
    void run() override
    {
        if (!_panel_agent->run())
            std::cerr << "Failed to run Panel.\n";
        _global_resource_lock.lock();
        _should_exit = true;
        _global_resource_lock.unlock();
    }
};

//////////////////////////////////////////////////////////////////////
// Start of PanelAgent Functions
//////////////////////////////////////////////////////////////////////
static bool initialize_panel_agent(const String &config, const String &display, bool resident)
{
    _panel_agent = new PanelAgent();

    if (!_panel_agent->initialize(config, display, resident))
        return false;

    _panel_agent->signal_connect_transaction_start(slot(slot_transaction_start));
    _panel_agent->signal_connect_transaction_end(slot(slot_transaction_end));
    _panel_agent->signal_connect_reload_config(slot(slot_reload_config));
    _panel_agent->signal_connect_turn_on(slot(slot_turn_on));
    _panel_agent->signal_connect_turn_off(slot(slot_turn_off));
    _panel_agent->signal_connect_update_screen(slot(slot_update_screen));
    _panel_agent->signal_connect_update_spot_location(slot(slot_update_spot_location));
    _panel_agent->signal_connect_update_factory_info(slot(slot_update_factory_info));
    _panel_agent->signal_connect_show_help(slot(slot_show_help));
    _panel_agent->signal_connect_show_factory_menu(slot(slot_show_factory_menu));
    _panel_agent->signal_connect_show_preedit_string(slot(slot_show_preedit_string));
    _panel_agent->signal_connect_show_aux_string(slot(slot_show_aux_string));
    _panel_agent->signal_connect_show_lookup_table(slot(slot_show_lookup_table));
    _panel_agent->signal_connect_hide_preedit_string(slot(slot_hide_preedit_string));
    _panel_agent->signal_connect_hide_aux_string(slot(slot_hide_aux_string));
    _panel_agent->signal_connect_hide_lookup_table(slot(slot_hide_lookup_table));
    _panel_agent->signal_connect_update_preedit_string(slot(slot_update_preedit_string));
    _panel_agent->signal_connect_update_preedit_caret(slot(slot_update_preedit_caret));
    _panel_agent->signal_connect_update_aux_string(slot(slot_update_aux_string));
    _panel_agent->signal_connect_update_lookup_table(slot(slot_update_lookup_table));
    _panel_agent->signal_connect_register_properties(slot(slot_register_properties));
    _panel_agent->signal_connect_update_property(slot(slot_update_property));
    _panel_agent->signal_connect_register_helper_properties(slot(slot_register_helper_properties));
    _panel_agent->signal_connect_update_helper_property(slot(slot_update_helper_property));
    _panel_agent->signal_connect_register_helper(slot(slot_register_helper));
    _panel_agent->signal_connect_remove_helper(slot(slot_remove_helper));
    _panel_agent->signal_connect_lock(slot(slot_lock));
    _panel_agent->signal_connect_unlock(slot(slot_unlock));

    _panel_agent->get_helper_list(_helper_list);

    return true;
}

static bool run_panel_agent(void)
{
    SCIM_DEBUG_MAIN(1) << "run_panel_agent ()\n";

    _panel_agent_thread = NULL;

    if (_panel_agent && _panel_agent->valid()) {
        _panel_agent_thread = new PanelAgentThread();
        _panel_agent_thread->start();
    }

    return (_panel_agent_thread != NULL);
}

static void start_auto_start_helpers(void)
{
    SCIM_DEBUG_MAIN(1) << "start_auto_start_helpers () begin\n";

    // Add Helper object items.
    for (size_t i = 0; i < _helper_list.size(); ++i) {
        SCIM_DEBUG_MAIN(1) << "--" << _helper_list[i].uuid << "--" << _helper_list[i].name << "--\n";
        if ((_helper_list[i].option & SCIM_HELPER_AUTO_START)) {
            _panel_agent->start_helper(_helper_list[i].uuid);
        }
    }
    SCIM_DEBUG_MAIN(1) << "start_auto_start_helpers () end\n";
}

static void slot_transaction_start(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_transaction_start ()\n";
    _transaction_lock.lock();
}

static void slot_transaction_end(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_transaction_end ()\n";
    _transaction_lock.unlock();
}

static void slot_reload_config(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_reload_config ()\n";
    if (!_config.null())
        _config->reload();
    else
        SCIM_DEBUG_MAIN(1) << "config is null\n";
}

static void slot_turn_on(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_turn_on ()" << _dbus_handler << "\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::TURN_ON));
}

static void slot_turn_off(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_turn_off ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::TURN_OFF));

    /*
     */
}

static void slot_update_screen(int num)
{
    SCIM_DEBUG_MAIN(1) << "slot_update_screen ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::UP_SCREEN, QVariantList() << num));
}

static void slot_update_factory_info(const PanelFactoryInfo &info)
{
    SCIM_DEBUG_MAIN(1) << "slot_update_factory_info ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::UP_FACTORY_INFO, QVariantList() << QVariant::fromValue(info)));
}

static void slot_show_help(const String &help)
{
    SCIM_DEBUG_MAIN(1) << "slot_show_help ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::SHOW_HELP, QVariantList() << QString::fromUtf8(help.c_str())));
}

static void slot_show_factory_menu(const std::vector<PanelFactoryInfo> &factories)
{
    SCIM_DEBUG_MAIN(1) << "slot_show_factory_menu ()\n";
    QVariantList list;
    Q_FOREACH (const PanelFactoryInfo &info, factories) {
        list << QVariant::fromValue(info);
    }
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::SHOW_FACTORY_MENU, list));
}

static void slot_update_spot_location(int x, int y)
{
    SCIM_DEBUG_MAIN(1) << "slot_update_spot_location ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::UP_SPOT_LOC, QVariantList() << x << y));
}

static void slot_show_preedit_string(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_show_preedit_string ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::SHOW_PREEDIT));
}

static void slot_show_aux_string(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_show_aux_string ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::SHOW_AUX));
}

static void slot_show_lookup_table(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_show_lookup_table ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::SHOW_LOOKUPTABLE));
}

static void slot_hide_preedit_string(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_hide_preedit_string ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::HIDE_PREEDIT));
}

static void slot_hide_aux_string(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_hide_aux_string ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::HIDE_AUX));
}

static void slot_hide_lookup_table(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_hide_lookup_table ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::HIDE_LOOKUPTABLE));
}

static void slot_update_preedit_string(const String &str, const AttributeList &attrs)
{
    SCIM_DEBUG_MAIN(1) << "slot_update_preedit_string ()" << str << "\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::UP_PREEDIT_STR, QVariantList() << QString::fromUtf8(str.c_str()) << AttrList2String(attrs)));
}

static void slot_update_preedit_caret(int caret)
{
    SCIM_DEBUG_MAIN(1) << "slot_update_preedit_caret ()" << caret << "\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::UP_PREEDIT_CARET, QVariantList() << caret));
}

static void slot_update_aux_string(const String &str, const AttributeList &attrs)
{
    SCIM_DEBUG_MAIN(1) << "slot_update_aux_string ()" << str << "\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::UP_AUX, QVariantList() << QString::fromUtf8(str.c_str()) << AttrList2String(attrs)));
}

static void slot_update_lookup_table(const LookupTable &table)
{
    SCIM_DEBUG_MAIN(1) << "slot_update_lookup_table ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::UP_LOOKUPTABLE, LookupTable2VariantList(table)));

    qApp->postEvent(
        _dbus_handler,
        new DBusEvent(DBusEvent::UP_LOOKUPTABLE_CURSOR, QVariantList() << (table.is_cursor_visible() ? table.get_cursor_pos_in_current_page() : -1)));
}

static void slot_register_properties(const PropertyList &props)
{
    SCIM_DEBUG_MAIN(1) << "slot_register_properties ()\n";
    QVariantList list;
    Q_FOREACH (const Property &prop, props) {
        list << QVariant::fromValue(prop);
    }
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::REG_PROPERTIES, list));
}

static void slot_update_property(const Property &prop)
{
    SCIM_DEBUG_MAIN(1) << "slot_update_property ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::UP_PROPERTY, QVariantList() << QVariant::fromValue(prop)));
}

static void slot_register_helper_properties(int id, const PropertyList &props)
{
    SCIM_DEBUG_MAIN(1) << "slot_register_helper_properties ()\n";
    QVariantList list;
    Q_FOREACH (const Property &prop, props) {
        list << QVariant::fromValue(prop);
    }
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::REG_HELPER_PROPERTIES, QVariantList() << id << list));
}

static void slot_update_helper_property(int id, const Property &prop)
{
    SCIM_DEBUG_MAIN(1) << "slot_update_helper_property ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::UP_HELPER_PROPERTY, QVariantList() << id << QVariant::fromValue(prop)));
}

static void slot_register_helper(int id, const HelperInfo &helper)
{
    SCIM_DEBUG_MAIN(1) << "slot_register_helper ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::REG_HELPER, QVariantList() << id << QVariant::fromValue(helper)));
}

static void slot_remove_helper(int id)
{
    SCIM_DEBUG_MAIN(1) << "slot_remove_helper ()\n";
    qApp->postEvent(_dbus_handler, new DBusEvent(DBusEvent::RM_HELPER, QVariantList() << id));
}

static void slot_lock(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_lock ()\n";
    //_panel_agent_lock.lock();
}

static void slot_unlock(void)
{
    SCIM_DEBUG_MAIN(1) << "slot_unlock ()\n";
    //_panel_agent_lock.unlock();
}
//////////////////////////////////////////////////////////////////////
// End of PanelAgent-Functions
//////////////////////////////////////////////////////////////////////

static void signalhandler(int sig)
{
    Q_UNUSED(sig)
    SCIM_DEBUG_MAIN(1) << "In signal handler...\n";

    if (_panel_agent)
        _panel_agent->stop();
}

int main(int argc, char *argv[])
{
    std::vector<String> config_list;

    int i;

    bool daemon = false;

    int new_argc = 0;
    char **new_argv = new char *[40];

    String config_name("simple");
    String display_name;
    bool should_resident = true;

    //    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    // Display version info
    //    std::cerr << "GTK Panel of SCIM " << SCIM_VERSION << "\n\n";

    // get modules list
    scim_get_config_module_list(config_list);

    // Add a dummy config module, it's not really a module!
    config_list.push_back("dummy");

    // Use socket Config module as default if available.
    if (!config_list.empty()) {
        if (std::find(config_list.begin(), config_list.end(), config_name) == config_list.end())
            config_name = config_list[0];
    }

    // DebugOutput::set_output("/home/ora/a.log");
    // DebugOutput::enable_debug (SCIM_DEBUG_AllMask);

    // parse command options
    i = 0;
    while (i < argc) {
        if (++i >= argc)
            break;

        if (String("-l") == argv[i] || String("--list") == argv[i]) {
            std::vector<String>::iterator it;

            std::cout << "\n";
            std::cout << "Available Config module:\n";
            for (it = config_list.begin(); it != config_list.end(); ++it)
                std::cout << "    " << *it << "\n";

            return 0;
        }

        if (String("-c") == argv[i] || String("--config") == argv[i]) {
            if (++i >= argc) {
                std::cerr << "no argument for option " << argv[i - 1] << "\n";
                return -1;
            }
            config_name = argv[i];
            continue;
        }

        if (String("-h") == argv[i] || String("--help") == argv[i]) {
            std::cout << "Usage: " << argv[0] << " [option]...\n\n"
                      << "The options are: \n"
                      << "  --display DISPLAY    Run on display DISPLAY.\n"
                      << "  -l, --list           List all of available config modules.\n"
                      << "  -c, --config NAME    Uses specified Config module.\n"
                      << "  -d, --daemon         Run " << argv[0] << " as a daemon.\n"
                      << "  -ns, --no-stay       Quit if no connected client.\n"
#if ENABLE_DEBUG
                      << "  -v, --verbose LEVEL  Enable debug info, to specific LEVEL.\n"
                      << "  -o, --output FILE    Output debug information into FILE.\n"
#endif
                      << "  -h, --help           Show this help message.\n";
            return 0;
        }

        if (String("-d") == argv[i] || String("--daemon") == argv[i]) {
            daemon = true;
            continue;
        }

        if (String("-ns") == argv[i] || String("--no-stay") == argv[i]) {
            should_resident = false;
            continue;
        }

        if (String("-v") == argv[i] || String("--verbose") == argv[i]) {
            if (++i >= argc) {
                std::cerr << "no argument for option " << argv[i - 1] << "\n";
                return -1;
            }
            DebugOutput::set_verbose_level(atoi(argv[i]));
            continue;
        }

        if (String("-o") == argv[i] || String("--output") == argv[i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv[i - 1] << "\n";
                return -1;
            }
            DebugOutput::set_output(argv[i]);
            continue;
        }

        if (String("--display") == argv[i]) {
            if (++i >= argc) {
                std::cerr << "No argument for option " << argv[i - 1] << "\n";
                return -1;
            }
            display_name = argv[i];
            continue;
        }

        if (String("--") == argv[i])
            break;

        std::cerr << "Invalid command line option: " << argv[i] << "\n";
        return -1;
    } // End of command line parsing.

    new_argv[new_argc++] = argv[0];

    // Store the rest argvs into new_argv.
    for (++i; i < argc && new_argc < 40; ++i) {
        new_argv[new_argc++] = argv[i];
    }

    // Make up DISPLAY env.
    if (display_name.length()) {
        new_argv[new_argc++] = strdup("--display");
        new_argv[new_argc++] = const_cast<char *>(display_name.c_str());

        setenv("DISPLAY", display_name.c_str(), 1);
    }

    new_argv[new_argc] = 0;

    if (!config_name.length()) {
        std::cerr << "No Config module is available!\n";
        return -1;
    }

    if (config_name != "dummy") {
        // load config module
        _config_module = new ConfigModule(config_name);

        if (!_config_module || !_config_module->valid()) {
            std::cerr << "Can not load " << config_name << " Config module.\n";
            return -1;
        }

        // create config instance
        _config = _config_module->create_config();
    } else {
        _config = new DummyConfig();
    }

    if (_config.null()) {
        std::cerr << "Failed to create Config instance from " << config_name << " Config module.\n";
        return -1;
    }

    KDE_signal(SIGQUIT, signalhandler);
    KDE_signal(SIGTERM, signalhandler);
    KDE_signal(SIGINT, signalhandler);
    KDE_signal(SIGHUP, signalhandler);

    const QByteArray display = qgetenv("DISPLAY");
    if (display.constData())
        display_name = String(display.constData());

    if (!initialize_panel_agent(config_name, display_name, should_resident)) {
        std::cerr << "Failed to initialize Panel Agent!\n";
        return -1;
    }

    if (daemon)
        scim_daemon();

    // connect the configuration reload signal.
    _config->signal_connect_reload(slot(ui_config_reload_callback));

    qRegisterMetaType<scim::Property>("scim::Property");
    qRegisterMetaType<PanelFactoryInfo>("PanelFactoryInfo");
    qRegisterMetaType<HelperInfo>("HelperInfo");

    QDBusConnection::connectToBus(QDBusConnection::SessionBus, "scim_panel");
    _dbus_handler = new DBusHandler();
    _dbus_handler->setInitialHelpers(_helper_list);

    if (!run_panel_agent()) {
        std::cerr << "Failed to run Socket Server!\n";
        return -1;
    }

    start_auto_start_helpers();

    QCoreApplication app(argc, argv);
    app.exec();

    _panel_agent_thread->wait();
    _config.reset();

    std::cerr << "Successfully exited.\n";

    return 0;
}

#include "main.moc"

#undef QT_NO_KEYWORDS

/*
vim:ts=4:nowrap:expandtab
*/
