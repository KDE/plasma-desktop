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
 * @file   MainWindow.c
 * @author Yuking yuking_net@sohu.com
 * @date   2008-1-16
 * 
 * @brief  主窗口
 * 
 * 
 */

#include "MainWindow.h"

#include <stdio.h>
#include <ctype.h>
#include <X11/xpm.h>

#include "IC.h"
#include "ui.h"
#include "ime.h"
#include "tools.h"

//下面的顺序不能改变
#include "logo.xpm"
#include "fullcorner.xpm"
#include "halfcorner.xpm"
#include "chnPunc.xpm"
#include "engPunc.xpm"
#include "gbk-0.xpm"
#include "gbk-1.xpm"
#include "lx-0.xpm"
#include "lx-1.xpm"
#include "lock-0.xpm"
#include "lock-1.xpm"
#include "vk-1.xpm"
#include "ft-0.xpm"
#include "ft-1.xpm"

#include "ui.h"
#include "vk.h"

Window          mainWindow;
int             MAINWND_WIDTH = _MAINWND_WIDTH;

int             iMainWindowX = MAINWND_STARTX;
int             iMainWindowY = MAINWND_STARTY;

WINDOW_COLOR    mainWindowColor = { NULL, NULL, {0, 240 << 8, 255 << 8, 240 << 8} };
MESSAGE_COLOR   mainWindowLineColor = { NULL, {0, 150 << 8, 220 << 8, 150 << 8} };	//线条色
MESSAGE_COLOR   IMNameColor[3] = {	//输入法名称的颜色
    {NULL, {0, 170 << 8, 170 << 8, 170 << 8}},
    {NULL, {0, 150 << 8, 200 << 8, 150 << 8}},
    {NULL, {0, 0, 0, 255 << 8}}
};

Bool            _3DEffectMainWindow = False;
XImage         *pLogo = NULL;

XImage         *pCorner[2] = { NULL, NULL };
char          **CornerLogo[2] = { halfcorner_xpm, fullcorner_xpm };
XImage         *pPunc[2] = { NULL, NULL };
char          **PuncLogo[2] = { engPunc_xpm, chnPunc_xpm };
XImage         *pGBK[2] = { NULL, NULL };
char          **GBKLogo[2] = { gbk_0_xpm, gbk_1_xpm };
XImage         *pLX[2] = { NULL, NULL };
char          **LXLogo[2] = { lx_0_xpm, lx_1_xpm };
XImage         *pLock[2] = { NULL, NULL };
char          **LockLogo[2] = { lock_0_xpm, lock_1_xpm };

XImage         *pGBKT[2] = { NULL, NULL };
char          **GBKTLogo[2] = { ft_0_xpm, ft_1_xpm };

XImage         *pVK = NULL;

HIDE_MAINWINDOW hideMainWindow = HM_SHOW;

Bool            bCompactMainWindow = False;
Bool            bShowVK = False;
Bool		bMainWindow_Hiden = False;

char           *strFullCorner = "全角模式";

extern Display *dpy;
extern GC       dimGC;
extern int      i3DEffect;
extern IC      *CurrentIC;
extern Bool     bCorner;
extern Bool     bLocked;
extern Bool     bChnPunc;
extern INT8     iIMIndex;
extern Bool     bUseGBK;
extern Bool     bSP;
extern Bool     bUseLegend;
extern IM      *im;
extern CARD16   connect_id;

extern Bool     bUseGBKT;

#ifdef _USE_XFT
extern XftFont *xftMainWindowFont;
extern XftFont *xftMainWindowFontEn;
#else
extern XFontSet fontSetMainWindow;
#endif

extern VKS      vks[];
extern unsigned char iCurrentVK;
extern Bool     bVK;

Bool CreateMainWindow (void)
{
    XSetWindowAttributes attrib;
    unsigned long   attribmask;
    int             iBackPixel;

    attrib.override_redirect = True;
    attribmask = CWOverrideRedirect;

    sprintf (strMainWindowXPMBackColor, ". 	c #%02x%02x%02x", mainWindowColor.backColor.red >> 8, mainWindowColor.backColor.green >> 8, mainWindowColor.backColor.blue >> 8);

    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(mainWindowColor.backColor)))
	iBackPixel = mainWindowColor.backColor.pixel;
    else
	iBackPixel = WhitePixel (dpy, DefaultScreen (dpy));

    mainWindow = XCreateSimpleWindow (dpy, DefaultRootWindow (dpy), iMainWindowX, iMainWindowY, MAINWND_WIDTH, MAINWND_HEIGHT, 0, WhitePixel (dpy, DefaultScreen (dpy)), iBackPixel);
    if (mainWindow == (Window) NULL)
	return False;

    XChangeWindowAttributes (dpy, mainWindow, attribmask, &attrib);
    XSelectInput (dpy, mainWindow, ExposureMask | ButtonPressMask | ButtonReleaseMask  | PointerMotionMask);
    
    InitMainWindowColor ();

    return True;
}

void DisplayMainWindow (void)
{
#ifdef _DEBUG
    fprintf (stderr, "DISPLAY MainWindow\n");
#endif

    if (!bMainWindow_Hiden)
	XMapRaised (dpy, mainWindow);
}

void DrawMainWindow (void)
{
    INT8            iIndex = 0;
    INT16           iPos;

    int             rv;
    XImage         *mask;
    XpmAttributes   attrib;

#ifdef _USE_XFT
    char            strTemp[MAX_IM_NAME + 1];
    char           *p1, *p2;
    Bool            bEn;
#endif
    char           *strGBKT;

    if ( bMainWindow_Hiden )
    	return;
	
    iIndex = IS_CLOSED;
    attrib.valuemask = 0;
#ifdef _DEBUG
    fprintf (stderr, "DRAW MainWindow\n");
#endif

    if (hideMainWindow == HM_SHOW || (hideMainWindow == HM_AUTO && (ConnectIDGetState (connect_id) != IS_CLOSED))) {
	if (!pLogo) {
	    rv = XpmCreateImageFromData (dpy, logo_xpm, &pLogo, &mask, &attrib);
	    if (rv != XpmSuccess)
		fprintf (stderr, "Failed to read xpm file: Logo\n");
	}
	if (!pVK) {
	    rv = XpmCreateImageFromData (dpy, vk_1_xpm, &pVK, &mask, &attrib);
	    if (rv != XpmSuccess)
		fprintf (stderr, "Failed to read xpm file: VK\n");
	}

	XPutImage (dpy, mainWindow, mainWindowColor.backGC, pLogo, 0, 0, 2, 2, 15, 16);
	XDrawRectangle (dpy, mainWindow, mainWindowLineColor.gc, 0, 0, MAINWND_WIDTH - 1, MAINWND_HEIGHT - 1);

	iPos = 20;
	if (!bCompactMainWindow) {
	    if (!pPunc[bChnPunc]) {
		rv = XpmCreateImageFromData (dpy, PuncLogo[bChnPunc], &pPunc[bChnPunc], &mask, &attrib);
		if (rv != XpmSuccess)
		    fprintf (stderr, "Failed to read xpm file: Punc\n");
	    }
	    XPutImage (dpy, mainWindow, mainWindowColor.backGC, pPunc[bChnPunc], 0, 0, iPos, 2, 15, 16);
	    iPos += 18;

	    if (!pCorner[bCorner]) {
		rv = XpmCreateImageFromData (dpy, CornerLogo[bCorner], &pCorner[bCorner], &mask, &attrib);
		if (rv != XpmSuccess)
		    fprintf (stderr, "Failed to read xpm file: Corner\n");
	    }
	    XPutImage (dpy, mainWindow, mainWindowColor.backGC, pCorner[bCorner], 0, 0, iPos, 2, 15, 16);
	    iPos += 18;

	    if (!pGBK[bUseGBK]) {
		rv = XpmCreateImageFromData (dpy, GBKLogo[bUseGBK], &pGBK[bUseGBK], &mask, &attrib);
		if (rv != XpmSuccess)
		    fprintf (stderr, "Failed to read xpm file: GBKLogo\n");
	    }
	    XPutImage (dpy, mainWindow, mainWindowColor.backGC, pGBK[bUseGBK], 0, 0, iPos, 2, 15, 16);
	    iPos += 18;

	    if (!pLX[bUseLegend]) {
		rv = XpmCreateImageFromData (dpy, LXLogo[bUseLegend], &pLX[bUseLegend], &mask, &attrib);
		if (rv != XpmSuccess)
		    fprintf (stderr, "Failed to read xpm file: LXLogo\n");
	    }
	    XPutImage (dpy, mainWindow, mainWindowColor.backGC, pLX[bUseLegend], 0, 0, iPos, 2, 15, 16);
	    iPos += 18;

	    if (!pGBKT[bUseGBKT]) {
		rv = XpmCreateImageFromData (dpy, GBKTLogo[bUseGBKT], &pGBKT[bUseGBKT], &mask, &attrib);
		if (rv != XpmSuccess)
		    fprintf (stderr, "Failed to read xpm file: GBKTLogo\n");
	    }
	    XPutImage (dpy, mainWindow, mainWindowColor.backGC, pGBKT[bUseGBKT], 0, 0, iPos, 2, 15, 16);
	    iPos += 18;
	}

	if (!pLock[bLocked]) {
	    rv = XpmCreateImageFromData (dpy, LockLogo[bLocked], &pLock[bLocked], &mask, &attrib);
	    if (rv != XpmSuccess)
		fprintf (stderr, "Failed to read xpm file: LockLogo\n");
	}
	XPutImage (dpy, mainWindow, mainWindowColor.backGC, pLock[bLocked], 0, 0, iPos, 2, 15, 16);
	iPos += 11;

	if (bShowVK || !bCompactMainWindow) {
	    XPutImage (dpy, mainWindow, mainWindowColor.backGC, pVK, 0, 0, iPos, 2, 19, 16);
	    iPos += 23;
	}

	iIndex = ConnectIDGetState (connect_id);
	XClearArea (dpy, mainWindow, iPos, 2, MAINWND_WIDTH - iPos - 2, MAINWND_HEIGHT - 4, False);
#ifdef _USE_XFT
	iPos += 2;

	if (bVK)
	    p1 = vks[iCurrentVK].strName;
	else if (bCorner)
	    p1 = strFullCorner;
	else
	    p1 = im[iIMIndex].strName;

	while (*p1) {
	    p2 = strTemp;
	    if (isprint (*p1))	//使用中文字体
		bEn = True;
	    else {
		*p2++ = *p1++;
		*p2++ = *p1++;
		bEn = False;
	    }
	    while (*p1) {
		if (isprint (*p1)) {
		    if (!bEn)
			break;
		    *p2++ = *p1++;
		}
		else {
		    if (bEn)
			break;
		    *p2++ = *p1++;
		    *p2++ = *p1++;
		}
	    }
	    *p2 = '\0';

	    strGBKT = bUseGBKT ? ConvertGBKSimple2Tradition (strTemp) : strTemp;
	    OutputString (mainWindow, (bEn) ? xftMainWindowFontEn : xftMainWindowFont, strGBKT, iPos, (MAINWND_HEIGHT + FontHeight (xftMainWindowFont)) / 2 - 1, IMNameColor[iIndex].color);
	    iPos += StringWidth (strGBKT, (bEn) ? xftMainWindowFontEn : xftMainWindowFont);

	    if (bUseGBKT)
		free (strGBKT);
	}
#else
	strGBKT = bUseGBKT ? ConvertGBKSimple2Tradition ((bVK) ? vks[iCurrentVK].strName : im[iIMIndex].strName) : ((bVK) ? vks[iCurrentVK].strName : im[iIMIndex].strName);
	OutputString (mainWindow, fontSetMainWindow, strGBKT, iPos, (MAINWND_HEIGHT + FontHeight (fontSetMainWindow)) / 2 - 1, IMNameColor[iIndex].gc);
	if (bUseGBKT)
	    free (strGBKT);
#endif

	if (_3DEffectMainWindow) {
	    Draw3DEffect (mainWindow, 1, 1, MAINWND_WIDTH - 2, MAINWND_HEIGHT - 2, _3D_UPPER);
	    if (!bCompactMainWindow) {
		Draw3DEffect (mainWindow, 1, 1, 18, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 19, 1, 18, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 37, 1, 18, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 55, 1, 18, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 73, 1, 18, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 91, 1, 18, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 109, 1, 11, 18, _3D_UPPER);
		Draw3DEffect (mainWindow, 120, 1, 22, 18, _3D_UPPER);
	    }
	}
	else {
	    iPos = 19;
	    XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, 18, 4, 18, MAINWND_HEIGHT - 4);
	    if (!bCompactMainWindow) {
		XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, 36, 4, 36, MAINWND_HEIGHT - 4);
		XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, 54, 4, 54, MAINWND_HEIGHT - 4);
		XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, 72, 4, 72, MAINWND_HEIGHT - 4);
		XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, 89, 4, 89, MAINWND_HEIGHT - 4);
		XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, 107, 4, 107, MAINWND_HEIGHT - 4);
		iPos = 108;
	    }
	    iPos += 11;
	    if (bShowVK || !bCompactMainWindow) {
		XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, iPos, 4, iPos, MAINWND_HEIGHT - 4);
		iPos += 21;
	    }
	    XDrawLine (dpy, mainWindow, mainWindowLineColor.gc, iPos, 4, iPos, MAINWND_HEIGHT - 4);
	}
    }
    else
	XUnmapWindow (dpy, mainWindow);
}

void InitMainWindowColor (void)
{
    XGCValues       values;
    int             iPixel;
    int             i;

    if (mainWindowLineColor.gc)
	XFreeGC (dpy, mainWindowLineColor.gc);
    mainWindowLineColor.gc = XCreateGC (dpy, mainWindow, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(mainWindowLineColor.color)))
	iPixel = mainWindowLineColor.color.pixel;
    else
	iPixel = WhitePixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, mainWindowLineColor.gc, iPixel);

    for (i = 0; i < 3; i++) {
	if (IMNameColor[i].gc)
	    XFreeGC (dpy, IMNameColor[i].gc);
	IMNameColor[i].gc = XCreateGC (dpy, mainWindow, 0, &values);
	if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(IMNameColor[i].color)))
	    iPixel = IMNameColor[i].color.pixel;
	else
	    iPixel = WhitePixel (dpy, DefaultScreen (dpy));
	XSetForeground (dpy, IMNameColor[i].gc, iPixel);
    }
}
