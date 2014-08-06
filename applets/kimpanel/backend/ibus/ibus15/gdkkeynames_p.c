/* GDK - The GIMP Drawing Kit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#include <glib.h>
#include "gdkkeynames_p.h"
#include "gdkkeysyms_p.h"

/* Key handling not part of the keymap */

#include "keyname-table.h"

#include <glib/gprintf.h>
#include <stdlib.h>
#include <string.h>

#define GDK_NUM_KEYS G_N_ELEMENTS (gdk_keys_by_keyval)

static int
_gdk_keys_name_compare (const void *pkey, const void *pbase)
{
  return strcmp ((const char *) pkey, 
		 (const char *) (keynames + ((const gdk_key *) pbase)->offset));
}

guint
_gdk_keyval_from_name (const gchar *keyval_name)
{
  gdk_key *found;

  g_return_val_if_fail (keyval_name != NULL, 0);
  
  found = bsearch (keyval_name, gdk_keys_by_name,
		   GDK_NUM_KEYS, sizeof (gdk_key),
		   _gdk_keys_name_compare);
  if (found != NULL)
    return found->keyval;
  else
    return GDK_KEY_VoidSymbol;
}
