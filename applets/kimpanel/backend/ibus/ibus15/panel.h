/*
    SPDX-FileCopyrightText: 2011 Ni Hui <shuizhuyuanluo@126.com>
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef __PANEL_IMPANEL_H__
#define __PANEL_IMPANEL_H__

#include <ibus.h>

class App;
#define IBUS_TYPE_PANEL_IMPANEL (ibus_panel_impanel_get_type())
#define IBUS_PANEL_IMPANEL(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), IBUS_TYPE_PANEL_IMPANEL, IBusPanelImpanel))
#define IBUS_PANEL_IMPANEL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), IBUS_TYPE_PANEL_IMPANEL, IBusPanelImpanelClass))
#define IBUS_IS_PANEL_IMPANEL(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), IBUS_TYPE_PANEL_IMPANEL))
#define IBUS_IS_PANEL_IMPANEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), IBUS_TYPE_PANEL_IMPANEL))
#define IBUS_PANEL_IMPANEL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), IBUS_TYPE_PANEL_IMPANEL, IBusPanelImpanelClass))

G_BEGIN_DECLS

typedef struct _IBusPanelImpanel IBusPanelImpanel;

GType ibus_panel_impanel_get_type(void);
#if !IBUS_CHECK_VERSION(1, 3, 99)
IBusPanelImpanel *ibus_panel_impanel_new(IBusConnection *connection);
#else
IBusPanelImpanel *ibus_panel_impanel_new(GDBusConnection *connection);
#endif
void ibus_panel_impanel_set_bus(IBusPanelImpanel *impanel, IBusBus *bus);
void ibus_panel_impanel_set_app(IBusPanelImpanel *impanel, App *app);
void ibus_panel_impanel_accept(IBusPanelImpanel *impanel);
void ibus_panel_impanel_navigate(IBusPanelImpanel *impanel, gboolean start, gboolean forward);
void ibus_panel_impanel_move_next(IBusPanelImpanel *impanel);

G_END_DECLS

#endif
