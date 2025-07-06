/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tabletevents.h"
#include "qwayland-tablet-v2.h"
#include <QQuickWindow>
#include <QWaylandClientExtensionTemplate>
#include <qguiapplication.h>
#include <qtwaylandclientversion.h>

class TabletPadDial : public QObject, public QtWayland::zwp_tablet_pad_dial_v2
{
public:
    TabletPadDial(TabletEvents *events, ::zwp_tablet_pad_dial_v2 *t)
        : QObject(events)
        , QtWayland::zwp_tablet_pad_dial_v2(t)
        , m_events(events)
    {
    }

    ~TabletPadDial()
    {
        destroy();
    }

    void zwp_tablet_pad_dial_v2_delta(int32_t value120) override
    {
        Q_EMIT m_events->dialDelta(value120);
    }

    TabletEvents *const m_events;
};

class TabletPadGroup : public QObject, public QtWayland::zwp_tablet_pad_group_v2
{
public:
    TabletPadGroup(TabletEvents *events, ::zwp_tablet_pad_group_v2 *t)
        : QObject(events)
        , QtWayland::zwp_tablet_pad_group_v2(t)
        , m_events(events)
    {
    }

    ~TabletPadGroup()
    {
        destroy();
    }

    void zwp_tablet_pad_group_v2_dial(zwp_tablet_pad_dial_v2 *dial) override
    {
        new TabletPadDial(m_events, dial);
    }

    TabletEvents *const m_events;
};

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

    void zwp_tablet_pad_v2_button(uint32_t /*time*/, uint32_t button, uint32_t state) override
    {
        Q_EMIT m_events->padButtonReceived(m_path, button, state);
    }

    void zwp_tablet_pad_v2_group(zwp_tablet_pad_group_v2 *pad_group) override
    {
        new TabletPadGroup(m_events, pad_group);
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
        , m_events(events)
    {
    }

    ~Tool()
    {
        destroy();
    }

    void zwp_tablet_tool_v2_hardware_serial(uint32_t hardware_serial_hi, uint32_t hardware_serial_lo) override
    {
        m_hardware_serial_hi = hardware_serial_hi;
        m_hardware_serial_lo = hardware_serial_lo;
    }

    void zwp_tablet_tool_v2_button(uint32_t /*serial*/, uint32_t button, uint32_t state) override
    {
        Q_EMIT m_events->toolButtonReceived(m_hardware_serial_hi, m_hardware_serial_lo, button, state);
    }

    void zwp_tablet_tool_v2_motion(wl_fixed_t x, wl_fixed_t y) override
    {
        m_tool_x = x;
        m_tool_y = y;
        const double pressure = m_pressure / 65535.0;
        Q_EMIT m_events->toolMotion(m_hardware_serial_hi,
                                    m_hardware_serial_lo,
                                    wl_fixed_to_double(m_tool_x),
                                    wl_fixed_to_double(m_tool_y),
                                    pressure,
                                    wl_fixed_to_double(m_tilt_x),
                                    wl_fixed_to_double(m_tilt_y));
    }

    void zwp_tablet_tool_v2_pressure(uint32_t pressure) override
    {
        m_pressure = pressure;
    }

    void zwp_tablet_tool_v2_tilt(wl_fixed_t tilt_x, wl_fixed_t tilt_y) override
    {
        m_tilt_x = tilt_x;
        m_tilt_y = tilt_y;
    }

    void zwp_tablet_tool_v2_down(uint32_t serial) override
    {
        Q_UNUSED(serial)
        Q_EMIT m_events->toolDown(m_hardware_serial_hi, m_hardware_serial_lo, wl_fixed_to_double(m_tool_x), wl_fixed_to_double(m_tool_y));
    }

    void zwp_tablet_tool_v2_up() override
    {
        Q_EMIT m_events->toolUp(m_hardware_serial_hi, m_hardware_serial_lo, wl_fixed_to_double(m_tool_x), wl_fixed_to_double(m_tool_y));
    }

    uint32_t m_hardware_serial_hi = 0;
    uint32_t m_hardware_serial_lo = 0;
    uint32_t m_tool_x = 0;
    uint32_t m_tool_y = 0;
    uint32_t m_pressure = 0;
    uint32_t m_tilt_x = 0;
    uint32_t m_tilt_y = 0;
    TabletEvents *const m_events;
};

class TabletManager : public QWaylandClientExtensionTemplate<TabletManager>, public QtWayland::zwp_tablet_manager_v2
{
public:
    TabletManager(TabletEvents *q)
        : QWaylandClientExtensionTemplate<TabletManager>(2)
        , q(q)
    {
        setParent(q);
        initialize();
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
    auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!waylandApp) {
        return;
    }
    auto seat = waylandApp->seat();

    auto tabletClient = new TabletManager(this);
    auto _seat = tabletClient->get_tablet_seat(seat);
    new TabletSeat(this, _seat);
}

#include "moc_tabletevents.cpp"
