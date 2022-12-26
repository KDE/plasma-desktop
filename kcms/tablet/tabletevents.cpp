/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tabletevents.h"
#include "qwayland-tablet-unstable-v2.h"
#include <QQuickWindow>
#include <QWaylandClientExtensionTemplate>
#include <qguiapplication.h>
#include <qpa/qplatformnativeinterface.h>
#include <qtwaylandclientversion.h>

extern "C" {
#include <libwacom/libwacom.h>
}

class TabletPad : public QObject, public QtWayland::zwp_tablet_pad_v2
{
public:
    TabletPad(TabletEvents *events, ::zwp_tablet_pad_v2 *t)
        : QObject(events)
        , QtWayland::zwp_tablet_pad_v2(t)
        , m_events(events)
    {
    }

    ~TabletPad()
    {
        destroy();
    }

    void zwp_tablet_pad_v2_path(const QString &path) override
    {
        m_path = path;
    }

    void zwp_tablet_pad_v2_buttons(uint32_t buttons) override
    {
        m_buttons = buttons;
    }

    void zwp_tablet_pad_v2_done() override
    {
        m_events->padButtonsChanged(m_path, m_buttons);
    }

    void zwp_tablet_pad_v2_button(uint32_t /*time*/, uint32_t button, uint32_t state) override
    {
        Q_EMIT m_events->padButtonReceived(m_path, button, state);
    }

    TabletEvents *const m_events;
    QString m_path;
    uint m_buttons = 0;
};

class Tool : public QObject, public QtWayland::zwp_tablet_tool_v2
{
public:
    Tool(TabletEvents *const events, ::zwp_tablet_tool_v2 *t)
        : QObject(events)
        , QtWayland::zwp_tablet_tool_v2(t)
        , m_db(libwacom_database_new())
        , m_events(events)
    {
    }

    ~Tool()
    {
        libwacom_database_destroy(m_db);
        destroy();
    }

    void zwp_tablet_tool_v2_hardware_serial(uint32_t hardware_serial_hi, uint32_t hardware_serial_lo) override
    {
        m_hardware_serial_hi = hardware_serial_hi;
        m_hardware_serial_lo = hardware_serial_lo;
    }

    void zwp_tablet_tool_v2_hardware_id_wacom(uint32_t hardware_id_hi, uint32_t hardware_id_lo) override
    {
        Q_UNUSED(hardware_id_hi) // higher bits is always zero
        const WacomStylus *stylus = libwacom_stylus_get_for_id(m_db, hardware_id_lo);

        if (stylus) {
            auto name = libwacom_stylus_get_name(stylus);

            qDebug() << "name: " << name << ", hardware id: " << hardware_id_lo;
            m_stylusButtonNumber = libwacom_stylus_get_num_buttons(stylus);
            Q_EMIT m_events->stylusButtonNumberChanged(m_stylusButtonNumber);
        }
    }

    void zwp_tablet_tool_v2_button(uint32_t /*serial*/, uint32_t button, uint32_t state) override
    {
        Q_EMIT m_events->toolButtonReceived(m_hardware_serial_hi, m_hardware_serial_lo, button, state);
    }

    void zwp_tablet_tool_v2_proximity_in(uint32_t serial, struct ::zwp_tablet_v2 *tablet, struct ::wl_surface *surface)
    {
    }

    uint32_t m_hardware_serial_hi = 0;
    uint32_t m_hardware_serial_lo = 0;
    int m_stylusButtonNumber = 3;
    WacomDeviceDatabase *m_db;
    TabletEvents *const m_events;
};

class TabletManager : public QWaylandClientExtensionTemplate<TabletManager>, public QtWayland::zwp_tablet_manager_v2
{
public:
    TabletManager(TabletEvents *q)
        : QWaylandClientExtensionTemplate<TabletManager>(ZWP_TABLET_MANAGER_V2_GET_TABLET_SEAT_SINCE_VERSION)
        , q(q)
    {
        setParent(q);
#if QTWAYLANDCLIENT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        initialize();
#else
        // QWaylandClientExtensionTemplate invokes this with a QueuedConnection but we want it called immediately
        QMetaObject::invokeMethod(this, "addRegistryListener", Qt::DirectConnection);
#endif
        Q_ASSERT(isInitialized());
    }

    ~TabletManager()
    {
        destroy();
    }

    TabletEvents *const q;
};

class TabletSeat : public QObject, public QtWayland::zwp_tablet_seat_v2
{
public:
    TabletSeat(TabletEvents *events, ::zwp_tablet_seat_v2 *seat)
        : QObject(events)
        , QtWayland::zwp_tablet_seat_v2(seat)
        , m_events(events)
    {
    }

    ~TabletSeat()
    {
        destroy();
    }

    void zwp_tablet_seat_v2_tool_added(struct ::zwp_tablet_tool_v2 *id) override
    {
        new Tool(m_events, id);
    }

    void zwp_tablet_seat_v2_pad_added(struct ::zwp_tablet_pad_v2 *id) override
    {
        new TabletPad(m_events, id);
    }

    TabletEvents *const m_events;
};

TabletEvents::TabletEvents(QQuickItem *parent)
    : QQuickItem(parent)
{
    auto seat = static_cast<wl_seat *>(QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("wl_seat"));

    auto tabletClient = new TabletManager(this);
    auto _seat = tabletClient->get_tablet_seat(seat);
    new TabletSeat(this, _seat);
}
