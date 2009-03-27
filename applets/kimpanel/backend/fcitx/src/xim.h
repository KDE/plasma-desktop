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
#ifndef _xim_h
#define _xim_h

#include <stdio.h>
#include <X11/Xlocale.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "IMdkit.h"
#include "Xi18n.h"

#define DEFAULT_IMNAME "fcitx"
#define STRBUFLEN 64

#ifndef ICONV_CONST
#define ICONV_CONST
#endif

#ifndef FALSE
#define FALSE	0
#endif

typedef enum _IME_STATE {
    IS_CLOSED = 0,
    IS_ENG,
    IS_CHN
} IME_STATE;

typedef struct _POSITION {
    int	x;
    int	y;
} position;

typedef struct _CONNECT_ID {
    struct _CONNECT_ID	*next;
    CARD16		connect_id;
    IME_STATE		imState;
    Bool		bTrackCursor:1;	//if in 'over the spot' mode
    position		pos;
} CONNECT_ID;

Bool            InitXIM (Window, char *);
void            SendHZtoClient (IMForwardEventStruct * call_data, char *strHZ);
void            EnterChineseMode (Bool bState);
void            CreateConnectID (IMOpenStruct * call_data);
void            DestroyConnectID (CARD16 connect_id);
void            SetConnectID (CARD16 connect_id, IME_STATE imState);
IME_STATE       ConnectIDGetState (CARD16 connect_id);

/* 用于lumaqq支持
Bool            ConnectIDGetReset (CARD16 connect_id);
void            ConnectIDSetReset (CARD16 connect_id, Bool bReset);
void            ConnectIDResetReset (void);
*/
void            ConnectIDSetPos (CARD16 connect_id, int x, int y);
position	*ConnectIDGetPos (CARD16 connect_id);
void            ConnectIDSetTrackCursor (CARD16 connect_id, Bool bTrack);
Bool            ConnectIDGetTrackCursor (CARD16 connect_id);
void            SetIMState (Bool bState);
void            MyIMForwardEvent (CARD16 connectId, CARD16 icId, int keycode);

/* char           *ConnectIDGetLocale(CARD16 connect_id); */
#ifndef __USE_GNU
extern char    *strcasestr (__const char *__haystack, __const char *__needle);
#endif

#endif
