/***************************************************************************
 *   Copyright (C) 2005 by Yunfan                                          *
 *   yunfan_zg@163.com                                                     *
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

#include "about.h"
#include "ui.h"
#include "xim.h"
#include "about_icon.xpm"
#include "version.h"

#include <ctype.h>

#ifdef _USE_XFT
#include <ft2build.h>
#include <X11/Xft/Xft.h>
#endif
#include <iconv.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>

extern Display *dpy;
extern int      iScreen;

extern int      iVKWindowFontSize;

#ifdef _USE_XFT
extern XftFont *xftVKWindowFont;
#else
extern XFontSet fontSetVKWindow;
#endif

Window          aboutWindow;
WINDOW_COLOR    AboutWindowColor = { NULL, NULL, {0, 220 << 8, 220 << 8, 220 << 8}
};
MESSAGE_COLOR   AboutWindowFontColor = { NULL, {0, 0, 0, 0}
};
int             ABOUT_WINDOW_WIDTH;

Atom            about_protocol_atom = 0;
Atom            about_kill_atom = 0;

char            AboutCaption[] = "关于 - FCITX";
char            AboutTitle[] = "小企鹅中文输入法";
char            AboutEmail[] = "yuking_net@sohu.com";
char            AboutCopyRight[] = "(c) 2005, Yuking";
char            strTitle[100];

int             iBackPixel;

extern iconv_t  convUTF8;
extern Bool     bIsUtf8;

Bool CreateAboutWindow (void)
{
    strcpy (strTitle, AboutTitle);
    strcat (strTitle, " ");
    strcat (strTitle, FCITX_VERSION);
    strcat (strTitle, "-");
    strcat (strTitle, USE_XFT);

    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(AboutWindowColor.backColor)))
	iBackPixel = AboutWindowColor.backColor.pixel;
    else
	iBackPixel = WhitePixel (dpy, DefaultScreen (dpy));

#ifdef _USE_XFT
    ABOUT_WINDOW_WIDTH = StringWidth (strTitle, xftVKWindowFont) + 50;
#else
    ABOUT_WINDOW_WIDTH = StringWidth (strTitle, fontSetVKWindow) + 50;
#endif
    aboutWindow =
	XCreateSimpleWindow (dpy, DefaultRootWindow (dpy), (DisplayWidth (dpy, iScreen) - ABOUT_WINDOW_WIDTH) / 2, (DisplayHeight (dpy, iScreen) - ABOUT_WINDOW_HEIGHT) / 2, ABOUT_WINDOW_WIDTH, ABOUT_WINDOW_HEIGHT, 0, WhitePixel (dpy, DefaultScreen (dpy)),
			     iBackPixel);
    if (aboutWindow == (Window) NULL)
	return False;

    InitWindowProperty ();
    XSelectInput (dpy, aboutWindow, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | EnterWindowMask | PointerMotionMask | LeaveWindowMask | VisibilityChangeMask);

    InitAboutWindowColor ();
    setIcon ();

    return True;
}

void InitWindowProperty (void)
{
    XTextProperty   tp;
    char            strOutput[100];
    char           *ps;

    Atom            about_wm_window_type = XInternAtom (dpy, "_NET_WM_WINDOW_TYPE", False);
    Atom            type_toolbar = XInternAtom (dpy, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);

    XSetTransientForHint (dpy, aboutWindow, DefaultRootWindow (dpy));

    XChangeProperty (dpy, aboutWindow, about_wm_window_type, XA_ATOM, 32, PropModeReplace, (void *) &type_toolbar, 1);

    about_protocol_atom = XInternAtom (dpy, "WM_PROTOCOLS", False);
    about_kill_atom = XInternAtom (dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols (dpy, aboutWindow, &about_kill_atom, 1);

    if (bIsUtf8) {
	size_t          l1, l2;
	char           *p;

	p = AboutCaption;
	ps = strOutput;
	l1 = strlen (AboutCaption);
	l2 = 99;
	l1 = iconv (convUTF8, (ICONV_CONST char **) (&p), &l1, &ps, &l2);
	*ps = '\0';
	ps = strOutput;
    }
    else
	ps = AboutCaption;

    tp.value = (void *) ps;
    tp.encoding = XA_STRING;
    tp.format = 16;
    tp.nitems = strlen (ps);
    XSetWMName (dpy, aboutWindow, &tp);
}

void InitAboutWindowColor (void)
{
    XGCValues       values;
    int             iPixel;

    AboutWindowFontColor.gc = XCreateGC (dpy, aboutWindow, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(AboutWindowFontColor.color)))
	iPixel = AboutWindowFontColor.color.pixel;
    else
	iPixel = WhitePixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, AboutWindowFontColor.gc, iPixel);
}

void DisplayAboutWindow (void)
{
    XMapRaised (dpy, aboutWindow);
    XMoveWindow (dpy, aboutWindow, (DisplayWidth (dpy, iScreen) - ABOUT_WINDOW_WIDTH) / 2, (DisplayHeight (dpy, iScreen) - ABOUT_WINDOW_HEIGHT) / 2);
}

void DrawAboutWindow (void)
{
#ifdef _USE_XFT
    OutputString (aboutWindow, xftVKWindowFont, strTitle, (ABOUT_WINDOW_WIDTH - StringWidth (strTitle, xftVKWindowFont)) / 2, iVKWindowFontSize + 6 + 30, AboutWindowFontColor.color);
#else
    OutputString (aboutWindow, fontSetVKWindow, strTitle, (ABOUT_WINDOW_WIDTH - StringWidth (strTitle, fontSetVKWindow)) / 2, iVKWindowFontSize + 6 + 30, AboutWindowFontColor.gc);
#endif

#ifdef _USE_XFT
    OutputString (aboutWindow, xftVKWindowFont, AboutEmail, (ABOUT_WINDOW_WIDTH - StringWidth (AboutEmail, xftVKWindowFont)) / 2, iVKWindowFontSize + 6 + 60, AboutWindowFontColor.color);
#else
    OutputString (aboutWindow, fontSetVKWindow, AboutEmail, (ABOUT_WINDOW_WIDTH - StringWidth (AboutEmail, fontSetVKWindow)) / 2, iVKWindowFontSize + 6 + 60, AboutWindowFontColor.gc);
#endif

#ifdef _USE_XFT
    OutputString (aboutWindow, xftVKWindowFont, AboutCopyRight, (ABOUT_WINDOW_WIDTH - StringWidth (AboutCopyRight, xftVKWindowFont)) / 2, iVKWindowFontSize + 6 + 80, AboutWindowFontColor.color);
#else
    OutputString (aboutWindow, fontSetVKWindow, AboutCopyRight, (ABOUT_WINDOW_WIDTH - StringWidth (AboutCopyRight, fontSetVKWindow)) / 2, iVKWindowFontSize + 6 + 80, AboutWindowFontColor.gc);
#endif
}

// we use logo.xpm as the Application icon
void setIcon (void)
{
    int             rv;
    Pixmap          pixmap = 0;
    Pixmap          mask = 0;
    XpmAttributes   attrib;
    XWMHints       *h = XGetWMHints (dpy, aboutWindow);
    XWMHints        wm_hints;
    Bool            got_hints = h != 0;

    attrib.valuemask = 0;
    rv = XCreatePixmapFromData (dpy, aboutWindow, about_icon_xpm, &pixmap, &mask, &attrib);
    if (rv != XpmSuccess) {
	fprintf (stderr, "Failed to read xpm file: %s\n", XpmGetErrorString (rv));
	return;
    }

    if (!got_hints) {
	h = &wm_hints;
	h->flags = 0;
    }
    h->icon_pixmap = pixmap;
    h->icon_mask = mask;
    h->flags |= IconPixmapHint | IconMaskHint;
    XSetWMHints (dpy, aboutWindow, h);
    if (got_hints)
	XFree ((char *) h);
}
