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
#ifndef _UI_H
#define _UI_H

#include <X11/Xlib.h>

#ifdef _USE_XFT
#include <ft2build.h>
#include <X11/Xft/Xft.h>
#endif

#define LIGHT_COLOR	0xffff
#define BACK_COLOR	0x0000
#define DIM_COLOR	0x6666

typedef enum {
    _3D_FLAT = 0,
    _3D_UPPER = 1,
    _3D_LOWER = 2
} _3D_EFFECT;

typedef struct {
    GC              foreGC;
    GC              backGC;
    XColor          backColor;
    XColor          foreColor;
} WINDOW_COLOR;

typedef struct {
    GC              gc;
    XColor          color;
} MESSAGE_COLOR;

typedef struct {
    char           *pcStr;
    int             iPixel;
} StrPixel;

Bool            InitX (void);
void            Draw3DEffect (Window window, int x, int y, int width, int height, _3D_EFFECT effect);
void            InitGC (Window window);
void            CreateFont (void);
void            MyXEventHandler (XEvent * event);
Bool            IsInBox (int x0, int y0, int x1, int y1, int x2, int y2);

/*void		SetLocale (void);*/

#ifdef _USE_XFT
void            OutputString (Window window, XftFont * font, char *str, int x, int y, XColor);
int             StringWidth (char *str, XftFont * font);
int             FontHeight (XftFont * font);
#else
void            OutputString (Window window, XFontSet font, char *str, int x, int y, GC gc);
int             StringWidth (char *str, XFontSet font);
int             FontHeight (XFontSet font);
#endif

/*int             FillImageByXPMData (XImage * pImage, char **apcData);*/
Bool            MouseClick (int *x, int *y, Window window);
/*
void            WaitButtonRelease (XPoint * point);
*/

#endif
