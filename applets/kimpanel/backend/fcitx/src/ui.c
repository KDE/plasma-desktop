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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ui.h"

#include <X11/Xlocale.h>
#include <X11/Xutil.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <iconv.h>

#include "xim.h"
#include "MyErrorsHandlers.h"
#include "MainWindow.h"
#include "InputWindow.h"
#include "vk.h"
#include "IC.h"
#include "tools.h"
#include "sp.h"
#include "about.h"

Display        *dpy;
int             iScreen;

#ifdef _USE_XFT
XftFont        *xftFont = (XftFont *) NULL;
XftFont        *xftFontEn = (XftFont *) NULL;
XftDraw        *xftDraw = (XftDraw *) NULL;
XftFont        *xftMainWindowFont = (XftFont *) NULL;
XftFont        *xftMainWindowFontEn = (XftFont *) NULL;
XftFont        *xftVKWindowFont = (XftFont *) NULL;
Bool            bUseAA = True;
Bool            bUseBold = True;
int             iMainWindowFontSize = 9;
int             iVKWindowFontSize = 11;
int             iFontSize = 12;
char            strUserLocale[50] = "zh_CN.UTF-8";
#else
XFontSet        fontSet = NULL;
XFontSet        fontSetMainWindow = NULL;
XFontSet        fontSetVKWindow = NULL;
int             iMainWindowFontSize = 12;
int             iVKWindowFontSize = 12;
int             iFontSize = 16;
Bool            bUseBold = True;
char            strUserLocale[50] = "zh_CN.gbk";
#endif

iconv_t         convUTF8;

GC              dimGC = (GC) NULL;
GC              lightGC = (GC) NULL;

Bool            bIsUtf8 = False;

char            strFontName[100] = "*";

#ifdef _USE_XFT
char            strFontEnName[100] = "Courier New";
#else
char            strFontEnName[100] = "Courier";
#endif

extern Window   mainWindow;
extern int      iMainWindowX;
extern int      iMainWindowY;
extern Window   inputWindow;
extern int      iInputWindowX;
extern int      iInputWindowY;
extern Window   VKWindow;
extern int      iVKWindowX;
extern int      iVKWindowY;
extern WINDOW_COLOR mainWindowColor;
extern WINDOW_COLOR inputWindowColor;
extern WINDOW_COLOR VKWindowColor;
extern HIDE_MAINWINDOW hideMainWindow;
extern IC      *CurrentIC;
extern int      MAINWND_WIDTH;
extern Bool     bCompactMainWindow;
extern INT8     iIMIndex;
extern CARD16   connect_id;
extern Bool     bShowVK;
extern Bool     bVK;

//added by yunfan
extern Window   aboutWindow;
extern Atom     about_protocol_atom;
extern Atom     about_kill_atom;

//**********************************

Bool InitX (void)
{
    if ((dpy = XOpenDisplay ((char *) NULL)) == NULL) {
	fprintf (stderr, "Error: FCITX can only run under X\n");
	return False;
    }

    SetMyXErrorHandler ();
    iScreen = DefaultScreen (dpy);

    convUTF8 = iconv_open ("UTF-8", "GB18030");

    return True;
}

void InitGC (Window window)
{
    XGCValues       values;
    XColor          color;
    int             iPixel;

    if (lightGC)
	XFreeGC (dpy, lightGC);
    lightGC = XCreateGC (dpy, window, 0, &values);
    color.red = LIGHT_COLOR;
    color.green = LIGHT_COLOR;
    color.blue = LIGHT_COLOR;
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &color))
	iPixel = color.pixel;
    else
	iPixel = WhitePixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, lightGC, iPixel);

    if (dimGC)
	XFreeGC (dpy, dimGC);
    dimGC = XCreateGC (dpy, window, 0, &values);
    color.red = DIM_COLOR;
    color.green = DIM_COLOR;
    color.blue = DIM_COLOR;
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &color))
	iPixel = color.pixel;
    else
	iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, dimGC, iPixel);

    if (mainWindowColor.backGC)
	XFreeGC (dpy, mainWindowColor.backGC);
    mainWindowColor.backGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(mainWindowColor.backColor)))
	iPixel = mainWindowColor.backColor.pixel;
    else
	iPixel = WhitePixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, mainWindowColor.backGC, iPixel);

    if (inputWindowColor.foreGC)
	XFreeGC (dpy, inputWindowColor.foreGC);
    inputWindowColor.foreGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(inputWindowColor.foreColor)))
	iPixel = inputWindowColor.foreColor.pixel;
    else
	iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, inputWindowColor.foreGC, iPixel);

    if (inputWindowColor.backGC)
	XFreeGC (dpy, inputWindowColor.backGC);
    inputWindowColor.backGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(inputWindowColor.backColor)))
	iPixel = inputWindowColor.backColor.pixel;
    else
	iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, inputWindowColor.backGC, iPixel);

    if (VKWindowColor.foreGC)
	XFreeGC (dpy, VKWindowColor.foreGC);
    VKWindowColor.foreGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(VKWindowColor.foreColor)))
	iPixel = VKWindowColor.foreColor.pixel;
    else
	iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, VKWindowColor.foreGC, iPixel);

    if (VKWindowColor.backGC)
	XFreeGC (dpy, VKWindowColor.backGC);
    VKWindowColor.backGC = XCreateGC (dpy, window, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(VKWindowColor.backColor)))
	iPixel = VKWindowColor.backColor.pixel;
    else
	iPixel = BlackPixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, VKWindowColor.backGC, iPixel);
}

/*
 * 当locale为UTF-8时，setlocale(LC_CTYPE,"")会导致程序崩溃
 */
/*void SetLocale(void)
{
	char *p;
	
	p=getenv("LC_CTYPE");
	if ( !p ) {
		p=getenv("LC_ALL");
		if ( !p)
			p=getenv("LANG");
	}
	
	//试验发现，当locale为zh_CN.UTF-8时，setlocale必须设置这zh_CN.UTF-8而不能是zh_CN.utf-8
	if ( p ) {
		strcpy(strUserLocale,p);
		p=strUserLocale;
		while ( *p!='.' && *p )
			p++;
		while ( *p ) {
			*p=toupper(*p);
			p++;
		}
		setlocale(LC_CTYPE,strUserLocale);
	}
	else
		setlocale(LC_CTYPE,"");
}
*/

#ifdef _USE_XFT
void CreateFont (void)
{
    //为了让界面能够正常显示，必须设置为中文模式
    if (strUserLocale[0])
	setlocale (LC_CTYPE, strUserLocale);

    if (xftFont)
	XftFontClose (dpy, xftFont);
    xftFont = XftFontOpen (dpy, iScreen, XFT_FAMILY, XftTypeString, strFontName, XFT_SIZE, XftTypeDouble, (double) iFontSize, XFT_ANTIALIAS, XftTypeBool, bUseAA, NULL);

    if (xftFontEn)
	XftFontClose (dpy, xftFontEn);
    xftFontEn = XftFontOpen (dpy, iScreen, XFT_FAMILY, XftTypeString, strFontEnName, XFT_SIZE, XftTypeDouble, (double) iFontSize, XFT_ANTIALIAS, XftTypeBool, bUseAA, NULL);

    if (xftMainWindowFont)
	XftFontClose (dpy, xftMainWindowFont);
    xftMainWindowFont =
	XftFontOpen (dpy, iScreen, XFT_FAMILY, XftTypeString, strFontName, XFT_SIZE, XftTypeDouble, (double) iMainWindowFontSize, XFT_ANTIALIAS, XftTypeBool, bUseAA, XFT_WEIGHT, XftTypeInteger, (bUseBold) ? XFT_WEIGHT_BOLD : XFT_WEIGHT_MEDIUM, NULL);

    if (xftMainWindowFontEn)
	XftFontClose (dpy, xftMainWindowFontEn);
    xftMainWindowFontEn =
	XftFontOpen (dpy, iScreen, XFT_FAMILY, XftTypeString, strFontEnName, XFT_SIZE, XftTypeDouble, (double) iMainWindowFontSize, XFT_ANTIALIAS, XftTypeBool, bUseAA, XFT_WEIGHT, XftTypeInteger, (bUseBold) ? XFT_WEIGHT_BOLD : XFT_WEIGHT_MEDIUM, NULL);

    if (xftVKWindowFont)
	XftFontClose (dpy, xftVKWindowFont);
    xftVKWindowFont = XftFontOpen (dpy, iScreen, XFT_FAMILY, XftTypeString, strFontName, XFT_SIZE, XftTypeDouble, (double) iVKWindowFontSize, XFT_ANTIALIAS, XftTypeBool, bUseAA, NULL);

    if (xftDraw)
	XftDrawDestroy (xftDraw);
    xftDraw = XftDrawCreate (dpy, inputWindow, DefaultVisual (dpy, DefaultScreen (dpy)), DefaultColormap (dpy, DefaultScreen (dpy)));

    setlocale (LC_CTYPE, "");
}
#else
void CreateFont (void)
{
    char          **missing_charsets;
    int             num_missing_charsets = 0;
    char           *default_string;
    char            strFont[256];

    //为了让界面能够正常显示，必须设置为中文模式
    if (strUserLocale[0])
	setlocale (LC_CTYPE, strUserLocale);

    if (bUseBold)
	sprintf (strFont, "-*-%s-bold-r-normal--%d-*-*-*-*-*-*-*,-*-%s-bold-r-normal--%d-*-*-*-*-*-*-*", strFontEnName, iMainWindowFontSize, strFontName, iMainWindowFontSize);
    else
	sprintf (strFont, "-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*,-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*", strFontEnName, iMainWindowFontSize, strFontName, iMainWindowFontSize);
    if (fontSetMainWindow)
	XFreeFontSet (dpy, fontSetMainWindow);
    fontSetMainWindow = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
    if (num_missing_charsets > 0) {
	fprintf (stderr, "Error: Cannot Create Chinese Fonts:\n\t%s\nUsing Default ...\n", strFont);
	iMainWindowFontSize = 14;
	if (bUseBold)
	    sprintf (strFont, "-*-*-medium-r-normal--%d-*-*-*-*-*-*-*,-*-*-medium-r-normal--%d-*-*-*-*-*-*-*", iMainWindowFontSize, iMainWindowFontSize);
	else
	    sprintf (strFont, "-*-*-bold-r-normal--%d-*-*-*-*-*-*-*,-*-*-bold-r-normal--%d-*-*-*-*-*-*-*", iMainWindowFontSize, iMainWindowFontSize);
	fontSetMainWindow = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
	if (num_missing_charsets > 0)
	    fprintf (stderr, "Error: Cannot Create Chinese Fonts!\n\n");
    }

    sprintf (strFont, "-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*,-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*", strFontEnName, iFontSize, strFontName, iFontSize);
    if (fontSet)
	XFreeFontSet (dpy, fontSet);
    fontSet = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
    if (num_missing_charsets > 0) {
	fprintf (stderr, "Error: Cannot Create Chinese Fonts:\n\t%s\nUsing Default ...\n", strFont);
	iFontSize = 16;
	sprintf (strFont, "-*-*-medium-r-normal--%d-*-*-*-*-*-*-*,-*-*-medium-r-normal--%d-*-*-*-*-*-*-*", iFontSize, iFontSize);
	fontSet = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
	if (num_missing_charsets > 0)
	    fprintf (stderr, "Error: Cannot Create Chinese Fonts!\n\n");
    }

    sprintf (strFont, "-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*,-*-%s-medium-r-normal--%d-*-*-*-*-*-*-*", strFontEnName, iVKWindowFontSize, strFontName, iVKWindowFontSize);
    if (fontSetVKWindow)
	XFreeFontSet (dpy, fontSetVKWindow);
    fontSetVKWindow = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
    if (num_missing_charsets > 0) {
	fprintf (stderr, "Error: Cannot Create Chinese Fonts:\n\t%s\nUsing Default ...\n", strFont);
	iFontSize = 12;
	sprintf (strFont, "-*-*-medium-r-normal--%d-*-*-*-*-*-*-*,-*-*-medium-r-normal--%d-*-*-*-*-*-*-*", iFontSize, iFontSize);
	fontSetVKWindow = XCreateFontSet (dpy, strFont, &missing_charsets, &num_missing_charsets, &default_string);
	if (num_missing_charsets > 0)
	    fprintf (stderr, "Error: Cannot Create Chinese Fonts!\n\n");
    }

    setlocale (LC_CTYPE, "");
}
#endif

/*
 * 让指定的区域显示三维效果
 * effect:
 *	_3D_UPPER:显示为凸出效果
 *	_3D_LOWER:显示为凹下效果
 */
void Draw3DEffect (Window window, int x, int y, int width, int height, _3D_EFFECT effect)
{
    if (effect == _3D_UPPER) {
	XDrawLine (dpy, window, lightGC, x, y, x + width - 1, y);
	XDrawLine (dpy, window, lightGC, x, y, x, y + height - 1);
	XDrawLine (dpy, window, dimGC, x + width - 1, y, x + width - 1, y + height - 1);
	XDrawLine (dpy, window, dimGC, x, y + height - 1, x + width - 1, y + height - 1);
    }
    else if (effect == _3D_LOWER) {
	XDrawLine (dpy, window, lightGC, x, y, x + width - 1, y);
	XDrawLine (dpy, window, lightGC, x, y, x, y + height - 1);
	XDrawLine (dpy, window, dimGC, x + width - 1, y, x + width - 1, y + height - 1);
	XDrawLine (dpy, window, dimGC, x, y + height - 1, x + width - 1, y + height - 1);

	XDrawLine (dpy, window, dimGC, x + 1, y + 1, x + width - 2, y + 1);
	XDrawLine (dpy, window, dimGC, x + 1, y + 1, x + 1, y + height - 2);
	XDrawLine (dpy, window, lightGC, x + 1, y + height - 2, x + width - 2, y + height - 2);
	XDrawLine (dpy, window, lightGC, x + width - 2, y + 1, x + width - 2, y + height - 2);
    }
    else {			//   _3D_FLAT
	XDrawRectangle (dpy, window, dimGC, x, y, width - 1, height - 1);
	return;
    }
}

/*
 * 有关界面的消息都在这里处理
 */
void MyXEventHandler (XEvent * event)
{
    unsigned char   iPos;

    switch (event->type) {
	//added by yunfan
    case ClientMessage:
	if ((event->xclient.message_type == about_protocol_atom) && ((Atom) event->xclient.data.l[0] == about_kill_atom)) {
	    XUnmapWindow (dpy, aboutWindow);
//X 	    DrawMainWindow ();
	}
	break;
	//*********************
    case Expose:
#if 0
#ifdef _DEBUG
	fprintf (stderr, "XEvent--Expose\n");
#endif
	if (event->xexpose.count > 0)
	    break;
	if (event->xexpose.window == mainWindow)
	    DrawMainWindow ();
	else if (event->xexpose.window == VKWindow)
	    DrawVKWindow ();
	else if (event->xexpose.window == inputWindow)
	    DrawInputWindow ();
	//added by yunfan
	else if (event->xexpose.window == aboutWindow)
	    DrawAboutWindow ();
	//******************************
#endif
	break;
    case DestroyNotify:
	break;
    case ButtonPress:
#if 0
	switch (event->xbutton.button) {
	case Button1:
	    if (event->xbutton.window == inputWindow) {
		int x, y;
		x = event->xbutton.x;
		y = event->xbutton.y;
		MouseClick (&x, &y, inputWindow);
		DrawInputWindow ();
		ConnectIDSetPos(connect_id, x, y);
	    }
	    else if (event->xbutton.window == mainWindow) {
		if (IsInBox (event->xbutton.x, event->xbutton.y, 1, 1, 16, 17)) {
		    iMainWindowX = event->xbutton.x;
		    iMainWindowY = event->xbutton.y;
		    if (!MouseClick (&iMainWindowX, &iMainWindowY, mainWindow)) {
			if (ConnectIDGetState (connect_id) != IS_CHN) {
			    XUnmapWindow (dpy, inputWindow);
			    XUnmapWindow (dpy, VKWindow);
			    SetIMState ((ConnectIDGetState (connect_id) == IS_ENG) ? False : True);
			    DrawMainWindow ();
			}
			else
			    ChangeIMState (connect_id);
		    }
		}
	    }
	    else if (event->xbutton.window == VKWindow) {
		if (!VKMouseKey (event->xbutton.x, event->xbutton.y)) {
		    iVKWindowX = event->xbutton.x;
		    iVKWindowY = event->xbutton.y;
		    MouseClick (&iVKWindowX, &iVKWindowY, VKWindow);
		    DrawVKWindow ();
		}
	    }
	    //added by yunfan
	    else if (event->xbutton.window == aboutWindow) {
		XUnmapWindow (dpy, aboutWindow);
		DrawMainWindow ();
	    }
	    //****************************
	    SaveProfile ();
	    break;
	}
#endif
	break;
    case ButtonRelease:
#if 0
	if (event->xbutton.window == mainWindow) {
	    switch (event->xbutton.button) {
	    case Button1:
		iPos = 20;
		if (!bCompactMainWindow) {
		    if (IsInBox (event->xbutton.x, event->xbutton.y, 20, 1, 38, 19))
			ChangePunc ();
		    else if (IsInBox (event->xbutton.x, event->xbutton.y, 38, 1, 55, 19))
			ChangeCorner ();
		    else if (IsInBox (event->xbutton.x, event->xbutton.y, 56, 1, 73, 19))
			ChangeGBK ();
		    else if (IsInBox (event->xbutton.x, event->xbutton.y, 74, 1, 91, 19))
			ChangeLegend ();
		    else if (IsInBox (event->xbutton.x, event->xbutton.y, 92, 1, 109, 19))
			ChangeGBKT ();

		    iPos = 110;
		}
		if (IsInBox (event->xbutton.x, event->xbutton.y, iPos, 1, iPos + 10, 19))
		    ChangeLock ();
		else {
		    if (bShowVK || !bCompactMainWindow) {
			if (IsInBox (event->xbutton.x, event->xbutton.y, iPos + 11, 1, iPos + 30, 19))
			    SwitchVK ();
			iPos += 20;
		    }

		    if (IsInBox (event->xbutton.x, event->xbutton.y, iPos + 11, 1, MAINWND_WIDTH, 19))
			SwitchIM (-1);
		}
		break;
		//added by yunfan
	    case Button2:
		DisplayAboutWindow ();
		break;
		//********************
	    case Button3:
		if (IsInBox (event->xbutton.x, event->xbutton.y, 1, 1, 16, 17)) {
		    XUnmapWindow (dpy, mainWindow);
		}
		else if (!bVK) {
		    bCompactMainWindow = !bCompactMainWindow;
		    SwitchIM (iIMIndex);
		}
		break;
	    }
	}
#endif
	break;
    case FocusIn:
	if (ConnectIDGetState (connect_id) == IS_CHN)
	    DisplayInputWindow ();
//X 	if (hideMainWindow != HM_HIDE)
//X 	    XMapRaised (dpy, mainWindow);
	break;
    default:
	break;
    }
}

/*
 * 判断鼠标点击处是否处于指定的区域内
 */
Bool IsInBox (int x0, int y0, int x1, int y1, int x2, int y2)
{
    if (x0 >= x1 && x0 <= x2 && y0 >= y1 && y0 <= y2)
	return True;

    return False;
}

#ifdef _USE_XFT
int StringWidth (char *str, XftFont * font)
{
    XGlyphInfo      extents;
    char            str1[100];
    char           *ps;
    size_t          l1, l2;
    int             il;

    if (!font)
	return 0;

    il = l1 = strlen (str);
    l2 = 99;
    ps = str1;

    l1 = iconv (convUTF8, (ICONV_CONST char **) &str, &l1, &ps, &l2);
    *ps = '\0';
    XftTextExtentsUtf8 (dpy, font, (FcChar8 *) str1, strlen (str1), &extents);
    if (font == xftMainWindowFont)
	return extents.width;

    return extents.xOff;
}

int FontHeight (XftFont * font)
{
    XGlyphInfo      extents;
    char            str1[] = "Ay中";
    char            str2[10];
    char           *ps1, *ps2;
    size_t          l1, l2;

    if (!font)
	return 0;

    l1 = strlen (str1);
    l2 = 9;
    ps2 = str2;
    ps1 = str1;

    l1 = iconv (convUTF8, (ICONV_CONST char **) &ps1, &l1, &ps2, &l2);
    *ps2 = '\0';

    XftTextExtentsUtf8 (dpy, font, (FcChar8 *) str2, strlen (str2), &extents);

    return extents.height;
}
#else
int StringWidth (char *str, XFontSet font)
{
    XRectangle      InkBox, LogicalBox;

    if (!font || !str)
	return 0;

    XmbTextExtents (font, str, strlen (str), &InkBox, &LogicalBox);

    return LogicalBox.width;
}

int FontHeight (XFontSet font)
{
    XRectangle      InkBox, LogicalBox;
    char            str[] = "Ay中";

    if (!font)
	return 0;

    XmbTextExtents (font, str, strlen (str), &InkBox, &LogicalBox);

    return LogicalBox.height;
}
#endif

/*
 * 以指定的颜色在窗口的指定位置输出字串
 */
#ifdef _USE_XFT
void OutputString (Window window, XftFont * font, char *str, int x, int y, XColor color)
{
    char            strOutput[100] = "\0";
    size_t          l1, l2;
    char           *ps;
    XftColor        xftColor;
    XRenderColor    renderColor;

    if (!font || !str)
	return;

    //使用UTF8串
    l1 = strlen (str);
    l2 = 99;
    ps = strOutput;
    l1 = iconv (convUTF8, (ICONV_CONST char **) &str, &l1, &ps, &l2);
    *ps = '\0';

    renderColor.red = color.red;
    renderColor.green = color.green;
    renderColor.blue = color.blue;
    renderColor.alpha = 0xFFFF;

    XftColorAllocValue (dpy, DefaultVisual (dpy, DefaultScreen (dpy)), DefaultColormap (dpy, DefaultScreen (dpy)), &renderColor, &xftColor);
    XftDrawChange (xftDraw, window);
    XftDrawStringUtf8 (xftDraw, &xftColor, font, x, y, (FcChar8 *) strOutput, strlen (strOutput));

    XftColorFree (dpy, DefaultVisual (dpy, DefaultScreen (dpy)), DefaultColormap (dpy, DefaultScreen (dpy)), &xftColor);
}
#else
void OutputString (Window window, XFontSet font, char *str, int x, int y, GC gc)
{
    if (!font || !str)
	return;

    XmbDrawString (dpy, window, font, gc, x, y, str, strlen (str));
}
#endif

/* *************下列函数取自于 rfinput-2.0 ************************ */
/*
 * 从xpm图形文件中图形数据填入到XImage变量中
 */
/*int FillImageByXPMData (XImage * pImage, char **apcData)
{
    int             iWidth, iHeight;
    int             iColorCount;
    int             iCharsPerPixel;
    char           *pcNext;
    int             i, j, k;
    StrPixel       *pPixels = NULL;
    char            acColor[3];
    XColor          AColor;
    int             iRV;

    iWidth = strtol (apcData[0], &pcNext, 10);
    iHeight = strtol (pcNext, &pcNext, 10);
    iColorCount = strtol (pcNext, &pcNext, 10);
    iCharsPerPixel = strtol (pcNext, &pcNext, 10);

    // Make the color array
    if (NULL == (pPixels = (StrPixel *) malloc (sizeof (StrPixel) * iColorCount)))
	goto ErrorExit;

    for (i = 0; i < iColorCount; i++)
	pPixels[i].pcStr = NULL;

    //File the Array
    for (i = 0; i < iColorCount; i++) {
	if (NULL == (pPixels[i].pcStr = (char *) malloc (iCharsPerPixel + 1)))
	    goto ErrorExit;

	strncpy (pPixels[i].pcStr, apcData[i + 1], iCharsPerPixel);
	pPixels[i].pcStr[iCharsPerPixel] = 0;

	acColor[0] = apcData[i + 1][strlen (apcData[i + 1]) - 6];
	acColor[1] = apcData[i + 1][strlen (apcData[i + 1]) - 5];
	acColor[2] = 0;
	AColor.red = strtol (acColor, NULL, 16) * 0xffff / 0xff;
	acColor[0] = apcData[i + 1][strlen (apcData[i + 1]) - 4];
	acColor[1] = apcData[i + 1][strlen (apcData[i + 1]) - 3];
	acColor[2] = 0;
	AColor.green = strtol (acColor, NULL, 16) * 0xffff / 0xff;
	acColor[0] = apcData[i + 1][strlen (apcData[i + 1]) - 2];
	acColor[1] = apcData[i + 1][strlen (apcData[i + 1]) - 1];
	acColor[2] = 0;
	AColor.blue = strtol (acColor, NULL, 16) * 0xffff / 0xff;

	iRV = XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &AColor);
	pPixels[i].iPixel = AColor.pixel;
    }

    // Fill the image
    for (i = 0; i < iHeight; i++) {
	for (j = 0; j < iWidth; j++) {
	    for (k = 0; k < iColorCount; k++) {
		if (!strncmp (pPixels[k].pcStr, apcData[i + 1 + iColorCount] + j * iCharsPerPixel, iCharsPerPixel)) {
		    XPutPixel (pImage, j, i, pPixels[k].iPixel);
		    break;
		}
	    }
	}
    }
    // Free the Array
    if (NULL != pPixels) {
	for (i = 0; i < iColorCount; i++) {
	    if (NULL != pPixels[i].pcStr)
		free (pPixels[i].pcStr);
	}
	free (pPixels);
    }
    return 1;
  ErrorExit:
    if (NULL != pPixels) {
	for (i = 0; i < iColorCount; i++) {
	    if (NULL != pPixels[i].pcStr)
		free (pPixels[i].pcStr);
	}
	free (pPixels);
    }
    return 0;
}
*/

Bool MouseClick (int *x, int *y, Window window)
{
    XEvent          evtGrabbed;
    Bool            bMoved = False;

    // To motion the  window
    while (1) {
	XMaskEvent (dpy, PointerMotionMask | ButtonReleaseMask | ButtonPressMask, &evtGrabbed);
	if (ButtonRelease == evtGrabbed.xany.type) {
	    if (Button1 == evtGrabbed.xbutton.button)
		break;
	}
	else if (MotionNotify == evtGrabbed.xany.type) {
	    static Time     LastTime;

	    if (evtGrabbed.xmotion.time - LastTime < 20)
		continue;

	    XMoveWindow (dpy, window, evtGrabbed.xmotion.x_root - *x, evtGrabbed.xmotion.y_root - *y);
	    XRaiseWindow (dpy, window);

	    bMoved = True;
	    LastTime = evtGrabbed.xmotion.time;
	}
    }

    *x = evtGrabbed.xmotion.x_root - *x;
    *y = evtGrabbed.xmotion.y_root - *y;

    return bMoved;
}

/*void WaitButtonRelease (XPoint * point)
{
    XEvent          ev;

    while (1) {
	XNextEvent (dpy, &ev);
	switch (ev.type) {
	case ButtonRelease:
	    point->x = ev.xbutton.x;
	    point->y = ev.xbutton.y;
	    return;
	default:
	    break;
	}
    }
}
*/
/* ****************************************************************** */
