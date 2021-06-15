/*
    GDK - The GIMP Drawing Kit
    SPDX-FileCopyrightText: 1995-1997 Peter Mattis
    SPDX-FileCopyrightText: 1995-1997 Spencer Kimball
    SPDX-FileCopyrightText: 1995-1997 Josh MacDonald
    SPDX-FileCopyrightText: 2005, 2006, 2007, 2009 GNOME Foundation

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef GTKACCELPARSE_P_H
#define GTKACCELPARSE_P_H

#include <glib.h>

G_BEGIN_DECLS

typedef enum {
    GDK_SHIFT_MASK = 1 << 0,
    GDK_LOCK_MASK = 1 << 1,
    GDK_CONTROL_MASK = 1 << 2,
    GDK_MOD1_MASK = 1 << 3,
    GDK_MOD2_MASK = 1 << 4,
    GDK_MOD3_MASK = 1 << 5,
    GDK_MOD4_MASK = 1 << 6,
    GDK_MOD5_MASK = 1 << 7,
    GDK_BUTTON1_MASK = 1 << 8,
    GDK_BUTTON2_MASK = 1 << 9,
    GDK_BUTTON3_MASK = 1 << 10,
    GDK_BUTTON4_MASK = 1 << 11,
    GDK_BUTTON5_MASK = 1 << 12,

    GDK_MODIFIER_RESERVED_13_MASK = 1 << 13,
    GDK_MODIFIER_RESERVED_14_MASK = 1 << 14,
    GDK_MODIFIER_RESERVED_15_MASK = 1 << 15,
    GDK_MODIFIER_RESERVED_16_MASK = 1 << 16,
    GDK_MODIFIER_RESERVED_17_MASK = 1 << 17,
    GDK_MODIFIER_RESERVED_18_MASK = 1 << 18,
    GDK_MODIFIER_RESERVED_19_MASK = 1 << 19,
    GDK_MODIFIER_RESERVED_20_MASK = 1 << 20,
    GDK_MODIFIER_RESERVED_21_MASK = 1 << 21,
    GDK_MODIFIER_RESERVED_22_MASK = 1 << 22,
    GDK_MODIFIER_RESERVED_23_MASK = 1 << 23,
    GDK_MODIFIER_RESERVED_24_MASK = 1 << 24,
    GDK_MODIFIER_RESERVED_25_MASK = 1 << 25,

    /* The next few modifiers are used by XKB, so we skip to the end.
     * Bits 15 - 25 are currently unused. Bit 29 is used internally.
     */

    GDK_SUPER_MASK = 1 << 26,
    GDK_HYPER_MASK = 1 << 27,
    GDK_META_MASK = 1 << 28,

    GDK_MODIFIER_RESERVED_29_MASK = 1 << 29,

    GDK_RELEASE_MASK = 1 << 30,

    /* Combination of GDK_SHIFT_MASK..GDK_BUTTON5_MASK + GDK_SUPER_MASK
       + GDK_HYPER_MASK + GDK_META_MASK + GDK_RELEASE_MASK */
    GDK_MODIFIER_MASK = 0x5c001fff
} GdkModifierType;

void _gtk_accelerator_parse(const gchar *accelerator, guint *accelerator_key, GdkModifierType *accelerator_mods);

G_END_DECLS

#endif // GDKACCELPARSE_P_H
