/*
 *  Copyright (C) 2011 Ni Hui <shuizhuyuanluo@126.com>
 *  Copyright (C) 2014 Weng Xuetian <wengxt@gmail.com>
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
#ifndef __PANEL_IMPANEL_H__
#define __PANEL_IMPANEL_H__

#include <ibus.h>

class App;
#define IBUS_TYPE_PANEL_IMPANEL        \
    (ibus_panel_impanel_get_type ())
#define IBUS_PANEL_IMPANEL(obj)            \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_PANEL_IMPANEL, IBusPanelImpanel))
#define IBUS_PANEL_IMPANEL_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_PANEL_IMPANEL, IBusPanelImpanelClass))
#define IBUS_IS_PANEL_IMPANEL(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_PANEL_IMPANEL))
#define IBUS_IS_PANEL_IMPANEL_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_PANEL_IMPANEL))
#define IBUS_PANEL_IMPANEL_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_PANEL_IMPANEL, IBusPanelImpanelClass))

G_BEGIN_DECLS

typedef struct _IBusPanelImpanel IBusPanelImpanel;

GType               ibus_panel_impanel_get_type     (void);
#if !IBUS_CHECK_VERSION(1,3,99)
IBusPanelImpanel   *ibus_panel_impanel_new          (IBusConnection     *connection);
#else
IBusPanelImpanel   *ibus_panel_impanel_new          (GDBusConnection    *connection);
#endif
void                ibus_panel_impanel_set_bus      (IBusPanelImpanel   *impanel,
                                                     IBusBus            *bus);
void                ibus_panel_impanel_set_app      (IBusPanelImpanel   *impanel,
                                                     App                *app);
void                ibus_panel_impanel_accept     (IBusPanelImpanel   *impanel);
void                ibus_panel_impanel_navigate     (IBusPanelImpanel   *impanel, gboolean start, gboolean forward);
void                ibus_panel_impanel_move_next     (IBusPanelImpanel   *impanel);

G_END_DECLS

#endif
