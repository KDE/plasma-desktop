/******************************************************************
 
         Copyright 1994, 1995 by Sun Microsystems, Inc.
         Copyright 1993, 1994 by Hewlett-Packard Company
 
Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and Hewlett-Packard not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. and Hewlett-Packard make no representations about
the suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
 
SUN MICROSYSTEMS INC. AND HEWLETT-PACKARD COMPANY DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
SUN MICROSYSTEMS, INC. AND HEWLETT-PACKARD COMPANY BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 
  Author: Hidetoshi Tajima(tajima@Eng.Sun.COM) Sun Microsystems, Inc.
 
******************************************************************/
#ifndef _IC_H
#define _IC_H

#include <stdio.h>
#include <X11/Xlib.h>

#include "IMdkit.h"
#include "Xi18n.h"
/*
#include "ime.h"
*/
typedef struct {
    XRectangle      area;	/* area */
    XRectangle      area_needed;	/* area needed */
    XPoint          spot_location;	/* spot location */
    Colormap        cmap;	/* colormap */
    CARD32          foreground;	/* foreground */
    CARD32          background;	/* background */
    Pixmap          bg_pixmap;	/* background pixmap */
    char           *base_font;	/* base font of fontset */
    CARD32          line_space;	/* line spacing */
    Cursor          cursor;	/* cursor */
} PreeditAttributes;

typedef struct {
    XRectangle      area;	/* area */
    XRectangle      area_needed;	/* area needed */
    Colormap        cmap;	/* colormap */
    CARD32          foreground;	/* foreground */
    CARD32          background;	/* background */
    Pixmap          bg_pixmap;	/* background pixmap */
    char           *base_font;	/* base font of fontset */
    CARD32          line_space;	/* line spacing */
    Cursor          cursor;	/* cursor */
} StatusAttributes;

typedef struct _IC {
    CARD16          id;		/* ic id */
    INT32           input_style;	/* input style */
    Window          client_win;	/* client window */
    Window          focus_win;	/* focus window */
    char           *resource_name;	/* resource name */
    char           *resource_class;	/* resource class */
    PreeditAttributes pre_attr;	/* preedit attributes */
    StatusAttributes sts_attr;	/* status attributes */
    struct _IC     *next;    
} IC;

extern IC      *FindIC (CARD16);
extern void     CreateIC (IMChangeICStruct *);
extern void     DestroyIC (IMChangeICStruct *);
extern void     SetIC (IMChangeICStruct *);
extern void     GetIC (IMChangeICStruct *);

#endif
