/***************************************************************************
 *   Copyright (C) 2002~2005 by Yuking                                     *
 *   yuking_net@sohu.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/**
 * @file   KeyList.h
 * @author Yuking yuking_net@sohu.com
 * @date   2008-1-16
 * 
 * @brief  º¸≈Ã…®√Ë¬Î¡–±Ì
 * 
 * 
 */

#ifndef _KEYLIST_H
#define _KETLIST_H

#include <stdio.h>
#include <X11/Xlib.h>

#include "xim.h"
#include "IMdkit.h"

typedef enum _KEYCODE_LIST {
    L_CTRL = 37,
    R_CTRL = 109,
    L_SHIFT = 50,
    R_SHIFT = 62,
    L_SUPER = 115,
    R_SUPER = 116
} KEY_CODE;

enum {
    K_LCTRL = 227,
    K_LSHIFT = 225,
    K_LALT = 233,
    K_RCTRL = 228,
    K_RSHIFT = 226,
    K_RALT = 234,
    K_INSERT = 99,
    K_HOME = 80,
    K_PGUP = 85,
    K_END = 87,
    K_PGDN = 86
};

typedef struct _KEY_LIST {
    char           *strKey;
    int             code;
} KEY_LIST;
// typedef struct _KEY_LIST KEYCODE_LIST;

typedef enum _KEY_STATE {
    KEY_NONE = 0,
    KEY_SHIFT_COMP = 1,
    KEY_CAPSLOCK = 2,
    KEY_CTRL_COMP = 4,
    KEY_CTRL_SHIFT_COMP = 5,
    KEY_ALT_COMP = 8,
    KEY_ALT_SHIFT_COMP = 9,
    KEY_CTRL_ALT_COMP = 12,
    KEY_CTRL_ALT_SHIFT_COMP = 13,
    KEY_NUMLOCK = 16,
    KEY_SUPER_COMP =64,
    KEY_SCROLLLOCK = 128,
    KEY_MOUSE_PRESSED = 256
} KEY_STATE;

typedef enum _KEY {
    TAB = 9,
    ENTER = 13,
    ESC = 27,
    DELETE = 255,

    LSHIFT = 9225,
    RSHIFT = 9226,
    LCTRL = 9227,
    RCTRL = 9228,

    LEFT = 8081,
    UP = 8082,
    RIGHT = 8083,
    DOWN = 8084,

    INSERT = 8099,
    HOME = 8080,
    PGUP = 8085,
    END = 8087,
    PGDN = 8086,

    CTRL_CTRL = 300,
    CTRL_LSHIFT,
    CTRL_LALT,
    CTRL_RSHIFT,
    CTRL_RALT,
    SHIFT_LCTRL,
    SHIFT_SHIFT,
    SHIFT_LALT,
    SHIFT_RCTRL,
    SHIFT_RALT,
    ALT_LCTRL,
    ALT_LSHIFT,
    ALT_ALT,
    ALT_RCTRL,
    ALT_RSHIFT = 314,

    CTRL_A = 1065,
    CTRL_B,
    CTRL_C,
    CTRL_D,
    CTRL_E,
    CTRL_F,
    CTRL_G,
    CTRL_H,
    CTRL_I,
    CTRL_J,
    CTRL_K,
    CTRL_L,
    CTRL_M,
    CTRL_N,
    CTRL_O,
    CTRL_P,
    CTRL_Q,
    CTRL_R,
    CTRL_S,
    CTRL_T,
    CTRL_U,
    CTRL_V,
    CTRL_W,
    CTRL_X,
    CTRL_Y,
    CTRL_Z,
    CTRL_0 = 1048,
    CTRL_1,
    CTRL_2,
    CTRL_3,
    CTRL_4,
    CTRL_5,
    CTRL_6,
    CTRL_7,
    CTRL_8,
    CTRL_9,
    CTRL_SPACE = 1032,
    CTRL_INSERT = 10099,
    CTRL_DELETE = 1255,

    SHIFT_SPACE = 2032,
    SHIFT_TAB = 11032,
    SHIFT_INSERT = 11099,
    SHIFT_DELETE = 2255,

    ALT_A = 3065,
    ALT_B,
    ALT_C,
    ALT_D,
    ALT_E,
    ALT_F,
    ALT_G,
    ALT_H,
    ALT_I,
    ALT_J,
    ALT_K,
    ALT_L,
    ALT_M,
    ALT_N,
    ALT_O,
    ALT_P,
    ALT_Q,
    ALT_R,
    ALT_S,
    ALT_T,
    ALT_U,
    ALT_V,
    ALT_W,
    ALT_X,
    ALT_Y,
    ALT_Z,
    ALT_0 = 3048,
    ALT_1,
    ALT_2,
    ALT_3,
    ALT_4,
    ALT_5,
    ALT_6,
    ALT_7,
    ALT_8,
    ALT_9,
    ALT_SPACE = 3032,
    ALT_INSERT = 12099,
    ALT_DELETE = 3255,

    CTRL_SHIFT_A = 4065,
    CTRL_SHIFT_B,
    CTRL_SHIFT_C,
    CTRL_SHIFT_D,
    CTRL_SHIFT_E,
    CTRL_SHIFT_F,
    CTRL_SHIFT_G,
    CTRL_SHIFT_H,
    CTRL_SHIFT_I,
    CTRL_SHIFT_J,
    CTRL_SHIFT_K,
    CTRL_SHIFT_L,
    CTRL_SHIFT_M,
    CTRL_SHIFT_N,
    CTRL_SHIFT_O,
    CTRL_SHIFT_P,
    CTRL_SHIFT_Q,
    CTRL_SHIFT_R,
    CTRL_SHIFT_S,
    CTRL_SHIFT_T,
    CTRL_SHIFT_U,
    CTRL_SHIFT_V,
    CTRL_SHIFT_W,
    CTRL_SHIFT_X,
    CTRL_SHIFT_Y,
    CTRL_SHIFT_Z,
    CTRL_SHIFT_0 = 4048,
    CTRL_SHIFT_1,
    CTRL_SHIFT_2,
    CTRL_SHIFT_3,
    CTRL_SHIFT_4,
    CTRL_SHIFT_5,
    CTRL_SHIFT_6,
    CTRL_SHIFT_7,
    CTRL_SHIFT_8,
    CTRL_SHIFT_9,

    CTRL_ALT_A = 5065,
    CTRL_ALT_B,
    CTRL_ALT_C,
    CTRL_ALT_D,
    CTRL_ALT_E,
    CTRL_ALT_F,
    CTRL_ALT_G,
    CTRL_ALT_H,
    CTRL_ALT_I,
    CTRL_ALT_J,
    CTRL_ALT_K,
    CTRL_ALT_L,
    CTRL_ALT_M,
    CTRL_ALT_N,
    CTRL_ALT_O,
    CTRL_ALT_P,
    CTRL_ALT_Q,
    CTRL_ALT_R,
    CTRL_ALT_S,
    CTRL_ALT_T,
    CTRL_ALT_U,
    CTRL_ALT_V,
    CTRL_ALT_W,
    CTRL_ALT_X,
    CTRL_ALT_Y,
    CTRL_ALT_Z,
    CTRL_ALT_0 = 5048,
    CTRL_ALT_1,
    CTRL_ALT_2,
    CTRL_ALT_3,
    CTRL_ALT_4,
    CTRL_ALT_5,
    CTRL_ALT_6,
    CTRL_ALT_7,
    CTRL_ALT_8,
    CTRL_ALT_9,

    ALT_SHIFT_A = 6065,
    ALT_SHIFT_B,
    ALT_SHIFT_C,
    ALT_SHIFT_D,
    ALT_SHIFT_E,
    ALT_SHIFT_F,
    ALT_SHIFT_G,
    ALT_SHIFT_H,
    ALT_SHIFT_I,
    ALT_SHIFT_J,
    ALT_SHIFT_K,
    ALT_SHIFT_L,
    ALT_SHIFT_M,
    ALT_SHIFT_N,
    ALT_SHIFT_O,
    ALT_SHIFT_P,
    ALT_SHIFT_Q,
    ALT_SHIFT_R,
    ALT_SHIFT_S,
    ALT_SHIFT_T,
    ALT_SHIFT_U,
    ALT_SHIFT_V,
    ALT_SHIFT_W,
    ALT_SHIFT_X,
    ALT_SHIFT_Y,
    ALT_SHIFT_Z,
    ALT_SHIFT_0 = 6048,
    ALT_SHIFT_1,
    ALT_SHIFT_2,
    ALT_SHIFT_3,
    ALT_SHIFT_4,
    ALT_SHIFT_5,
    ALT_SHIFT_6,
    ALT_SHIFT_7,
    ALT_SHIFT_8,
    ALT_SHIFT_9,

    CTRL_ALT_SHIFT_A = 7065,
    CTRL_ALT_SHIFT_B,
    CTRL_ALT_SHIFT_C,
    CTRL_ALT_SHIFT_D,
    CTRL_ALT_SHIFT_E,
    CTRL_ALT_SHIFT_F,
    CTRL_ALT_SHIFT_G,
    CTRL_ALT_SHIFT_H,
    CTRL_ALT_SHIFT_I,
    CTRL_ALT_SHIFT_J,
    CTRL_ALT_SHIFT_K,
    CTRL_ALT_SHIFT_L,
    CTRL_ALT_SHIFT_M,
    CTRL_ALT_SHIFT_N,
    CTRL_ALT_SHIFT_O,
    CTRL_ALT_SHIFT_P,
    CTRL_ALT_SHIFT_Q,
    CTRL_ALT_SHIFT_R,
    CTRL_ALT_SHIFT_S,
    CTRL_ALT_SHIFT_T,
    CTRL_ALT_SHIFT_U,
    CTRL_ALT_SHIFT_V,
    CTRL_ALT_SHIFT_W,
    CTRL_ALT_SHIFT_X,
    CTRL_ALT_SHIFT_Y,
    CTRL_ALT_SHIFT_Z,
    CTRL_ALT_SHIFT_0 = 7048,
    CTRL_ALT_SHIFT_1,
    CTRL_ALT_SHIFT_2,
    CTRL_ALT_SHIFT_3,
    CTRL_ALT_SHIFT_4,
    CTRL_ALT_SHIFT_5,
    CTRL_ALT_SHIFT_6,
    CTRL_ALT_SHIFT_7,
    CTRL_ALT_SHIFT_8,
    CTRL_ALT_SHIFT_9
} KEY;

int             GetKey (unsigned char iKeyCode, int iKeyState, int iCount);
int             ParseKey (char *strKey);
int             GetKeyList (char *strKey);
//int             GetKeyCodeList (char *strKey);

#endif
