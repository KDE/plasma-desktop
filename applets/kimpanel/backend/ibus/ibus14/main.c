/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011 Ni Hui <shuizhuyuanluo@126.com>
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

#include <ibus.h>
#include <stdlib.h>
#include <locale.h>
#include "panel.h"

#define _UNUSED(x) ((void) x)

static IBusBus *bus = NULL;
static IBusPanelImpanel *impanel = NULL;

/* options */
static gboolean ibus = FALSE;
static gboolean verbose = FALSE;

static const GOptionEntry entries[] =
{
    { "ibus", 'i', 0, G_OPTION_ARG_NONE, &ibus, "component is executed by ibus", NULL },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", NULL },
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL  },
};

static void
ibus_disconnected_cb (IBusBus  *bus,
                      gpointer  user_data)
{
    _UNUSED(bus);
    _UNUSED(user_data);
    ibus_quit ();
}

static void
ibus_impanel_start (void)
{
    ibus_init ();
    bus = ibus_bus_new ();
    if (!ibus_bus_is_connected (bus)) {
        exit (-1);
    }
    g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);
    impanel = ibus_panel_impanel_new (ibus_bus_get_connection (bus));
    ibus_bus_request_name (bus, IBUS_SERVICE_PANEL, 0);
    ibus_panel_impanel_set_bus(impanel, bus);
    ibus_main ();
}

gint
main (gint argc, gchar **argv)
{
    GError *error = NULL;
    GOptionContext *context;

    setlocale (LC_ALL, "");

    context = g_option_context_new ("- ibus impanel component");

    g_option_context_add_main_entries (context, entries, "ibus-impanel");

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("Option parsing failed: %s\n", error->message);
        exit (-1);
    }

    ibus_impanel_start ();

    return 0;
}

