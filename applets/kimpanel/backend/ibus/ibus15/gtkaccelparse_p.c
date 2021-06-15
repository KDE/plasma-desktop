/*
    GDK - The GIMP Drawing Kit
    SPDX-FileCopyrightText: 1995-1997 Peter Mattis
    SPDX-FileCopyrightText: 1995-1997 Spencer Kimball
    SPDX-FileCopyrightText: 1995-1997 Josh MacDonald
    SPDX-FileCopyrightText: 2005, 2006, 2007, 2009 GNOME Foundation

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include "gdkkeysyms_p.h"
#include "gtkaccelparse_p.h"
#include "gdkkeynames_p.h"

static inline gboolean
is_alt (const gchar *string)
{
    // clang-format off
    return ((string[0] == '<') &&
          (string[1] == 'a' || string[1] == 'A') &&
          (string[2] == 'l' || string[2] == 'L') &&
          (string[3] == 't' || string[3] == 'T') &&
          (string[4] == '>'));
    // clang-format on
}

static inline gboolean
is_ctl (const gchar *string)
{
    // clang-format off
    return ((string[0] == '<') &&
          (string[1] == 'c' || string[1] == 'C') &&
          (string[2] == 't' || string[2] == 'T') &&
          (string[3] == 'l' || string[3] == 'L') &&
          (string[4] == '>'));
    // clang-format on
}

static inline gboolean
is_modx (const gchar *string)
{
    // clang-format off
    return ((string[0] == '<') &&
          (string[1] == 'm' || string[1] == 'M') &&
          (string[2] == 'o' || string[2] == 'O') &&
          (string[3] == 'd' || string[3] == 'D') &&
          (string[4] >= '1' && string[4] <= '5') &&
          (string[5] == '>'));
    // clang-format on
}

static inline gboolean
is_ctrl (const gchar *string)
{
    // clang-format off
    return ((string[0] == '<') &&
          (string[1] == 'c' || string[1] == 'C') &&
          (string[2] == 't' || string[2] == 'T') &&
          (string[3] == 'r' || string[3] == 'R') &&
          (string[4] == 'l' || string[4] == 'L') &&
          (string[5] == '>'));
    // clang-format on
}

static inline gboolean
is_shft (const gchar *string)
{
    // clang-format off
  return ((string[0] == '<') &&
          (string[1] == 's' || string[1] == 'S') &&
          (string[2] == 'h' || string[2] == 'H') &&
          (string[3] == 'f' || string[3] == 'F') &&
          (string[4] == 't' || string[4] == 'T') &&
          (string[5] == '>'));
    // clang-format on
}

static inline gboolean
is_shift (const gchar *string)
{
    // clang-format off
    return ((string[0] == '<') &&
          (string[1] == 's' || string[1] == 'S') &&
          (string[2] == 'h' || string[2] == 'H') &&
          (string[3] == 'i' || string[3] == 'I') &&
          (string[4] == 'f' || string[4] == 'F') &&
          (string[5] == 't' || string[5] == 'T') &&
          (string[6] == '>'));
    // clang-format on
}

static inline gboolean
is_control (const gchar *string)
{
    // clang-format off
    return  ((string[0] == '<') &&
          (string[1] == 'c' || string[1] == 'C') &&
          (string[2] == 'o' || string[2] == 'O') &&
          (string[3] == 'n' || string[3] == 'N') &&
          (string[4] == 't' || string[4] == 'T') &&
          (string[5] == 'r' || string[5] == 'R') &&
          (string[6] == 'o' || string[6] == 'O') &&
          (string[7] == 'l' || string[7] == 'L') &&
          (string[8] == '>'));
    // clang-format on
}

static inline gboolean
is_release (const gchar *string)
{
    // clang-format off
    return  ((string[0] == '<') &&
          (string[1] == 'r' || string[1] == 'R') &&
          (string[2] == 'e' || string[2] == 'E') &&
          (string[3] == 'l' || string[3] == 'L') &&
          (string[4] == 'e' || string[4] == 'E') &&
          (string[5] == 'a' || string[5] == 'A') &&
          (string[6] == 's' || string[6] == 'S') &&
          (string[7] == 'e' || string[7] == 'E') &&
          (string[8] == '>'));
    // clang-format on
}

static inline gboolean
is_meta (const gchar *string)
{
    // clang-format off
    return  ((string[0] == '<') &&
          (string[1] == 'm' || string[1] == 'M') &&
          (string[2] == 'e' || string[2] == 'E') &&
          (string[3] == 't' || string[3] == 'T') &&
          (string[4] == 'a' || string[4] == 'A') &&
          (string[5] == '>'));
    // clang-format on
}

static inline gboolean
is_super (const gchar *string)
{
    // clang-format off
    return  ((string[0] == '<') &&
          (string[1] == 's' || string[1] == 'S') &&
          (string[2] == 'u' || string[2] == 'U') &&
          (string[3] == 'p' || string[3] == 'P') &&
          (string[4] == 'e' || string[4] == 'E') &&
          (string[5] == 'r' || string[5] == 'R') &&
          (string[6] == '>'));
    // clang-format on
}

static inline gboolean
is_hyper (const gchar *string)
{
    // clang-format off
    return  ((string[0] == '<') &&
          (string[1] == 'h' || string[1] == 'H') &&
          (string[2] == 'y' || string[2] == 'Y') &&
          (string[3] == 'p' || string[3] == 'P') &&
          (string[4] == 'e' || string[4] == 'E') &&
          (string[5] == 'r' || string[5] == 'R') &&
          (string[6] == '>'));
    // clang-format on
}

static inline gboolean
is_keycode (const gchar *string)
{
    // clang-format off
    return (string[0] == '0' &&
          string[1] == 'x' &&
          g_ascii_isxdigit (string[2]) &&
          g_ascii_isxdigit (string[3]));
    // clang-format on
}

void
_gtk_accelerator_parse (const gchar     *accelerator,
                        guint           *accelerator_key,
                        GdkModifierType *accelerator_mods)
{
  guint keyval;
  GdkModifierType mods;
  gint len;
  gboolean error;

  if (accelerator_key)
    *accelerator_key = 0;
  if (accelerator_mods)
    *accelerator_mods = 0;

  g_return_if_fail (accelerator != NULL);

  error = FALSE;
  keyval = 0;
  mods = 0;
  len = strlen (accelerator);
  while (len)
    {
      if (*accelerator == '<')
        {
          if (len >= 9 && is_release (accelerator))
            {
              accelerator += 9;
              len -= 9;
              mods |= GDK_RELEASE_MASK;
            }
          else if (len >= 9 && is_control (accelerator))
            {
              accelerator += 9;
              len -= 9;
              mods |= GDK_CONTROL_MASK;
            }
          else if (len >= 7 && is_shift (accelerator))
            {
              accelerator += 7;
              len -= 7;
              mods |= GDK_SHIFT_MASK;
            }
          else if (len >= 6 && is_shft (accelerator))
            {
              accelerator += 6;
              len -= 6;
              mods |= GDK_SHIFT_MASK;
            }
          else if (len >= 6 && is_ctrl (accelerator))
            {
              accelerator += 6;
              len -= 6;
              mods |= GDK_CONTROL_MASK;
            }
          else if (len >= 6 && is_modx (accelerator))
            {
              static const guint mod_vals[] = {
                GDK_MOD1_MASK, GDK_MOD2_MASK, GDK_MOD3_MASK,
                GDK_MOD4_MASK, GDK_MOD5_MASK
              };

              len -= 6;
              accelerator += 4;
              mods |= mod_vals[*accelerator - '1'];
              accelerator += 2;
            }
          else if (len >= 5 && is_ctl (accelerator))
            {
              accelerator += 5;
              len -= 5;
              mods |= GDK_CONTROL_MASK;
            }
          else if (len >= 5 && is_alt (accelerator))
            {
              accelerator += 5;
              len -= 5;
              mods |= GDK_MOD1_MASK;
            }
          else if (len >= 6 && is_meta (accelerator))
            {
              accelerator += 6;
              len -= 6;
              // mods |= GDK_META_MASK;
            }
          else if (len >= 7 && is_hyper (accelerator))
            {
              accelerator += 7;
              len -= 7;
              mods |= GDK_MOD4_MASK;
            }
          else if (len >= 7 && is_super (accelerator))
            {
              accelerator += 7;
              len -= 7;
              mods |= GDK_MOD4_MASK;
            }
          else
            {
              gchar last_ch;

              last_ch = *accelerator;
              while (last_ch && last_ch != '>')
                {
                  last_ch = *accelerator;
                  accelerator += 1;
                  len -= 1;
                }
            }
        }
      else
        {
          if (len >= 4 && is_keycode (accelerator))
            {
               char keystring[5];
               gchar *endptr;

               memcpy (keystring, accelerator, 4);
               keystring [4] = '\000';

               strtol (keystring, &endptr, 16);

               if (endptr == NULL || *endptr != '\000')
                 {
                   error = TRUE;
                   goto out;
                 }
               else
                 {
                   /* There was a keycode in the string, but
                    * we cannot store it, so we have an error */
                   error = TRUE;
                   goto out;
                 }
            }
      else
        {
          keyval = _gdk_keyval_from_name (accelerator);
          if (keyval == GDK_KEY_VoidSymbol)
            {
              error = TRUE;
              goto out;
        }
        }

          accelerator += len;
          len -= len;
        }
    }

out:
  if (error)
    keyval = mods = 0;

  if (accelerator_key)
    *accelerator_key = keyval;
  if (accelerator_mods)
    *accelerator_mods = mods;
}
