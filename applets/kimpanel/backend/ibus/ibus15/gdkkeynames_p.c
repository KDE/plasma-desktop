/*
    GDK - The GIMP Drawing Kit
    SPDX-FileCopyrightText: 1995-1997 Peter Mattis
    SPDX-FileCopyrightText: 1995-1997 Spencer Kimball
    SPDX-FileCopyrightText: 1995-1997 Josh MacDonald

    SPDX-License-Identifier: LGPL-2.0-or-later
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
