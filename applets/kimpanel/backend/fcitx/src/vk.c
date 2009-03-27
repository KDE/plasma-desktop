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

#include "vk.h"
#include "vk.xpm"
#include "ui.h"
#include "MainWindow.h"
#include "InputWindow.h"
#include "IC.h"
#include "xim.h"

#include <limits.h>
#include <ctype.h>
#include <X11/xpm.h>

#ifdef _USE_XFT
#include <ft2build.h>
#include <X11/Xft/Xft.h>
#include <iconv.h>
#endif

Window          VKWindow;
WINDOW_COLOR    VKWindowColor = { NULL, NULL, {0, 220 << 8, 220 << 8, 220 << 8}
};
MESSAGE_COLOR   VKWindowAlphaColor = { NULL, {0, 80 << 8, 0, 0}
};
MESSAGE_COLOR   VKWindowFontColor = { NULL, {0, 0, 0, 0}
};
XImage         *pVKLogo = NULL;

int             iVKWindowX = 0;
int             iVKWindowY = 0;
unsigned char   iCurrentVK = 0;
unsigned char   iVKCount = 0;

VKS             vks[VK_MAX];
char            vkTable[VK_NUMBERS + 1] = "`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./";

char            strCharTable[] = "`~1!2@3#4$5%6^7&8*9(0)-_=+[{]}\\|;:'\",<.>/?";	//用于转换上/下档键

Bool            bShiftPressed = False;
Bool            bVKCaps = False;

extern Display *dpy;
extern int      iScreen;

extern int      iVKWindowFontSize;

#ifdef _USE_XFT
extern XftFont *xftVKWindowFont;
#else
extern XFontSet fontSetVKWindow;
#endif

extern char     strStringGet[];
extern Bool     bVK;
extern int      iMainWindowX;
extern int      iMainWindowY;
extern Window   inputWindow;

extern IC      *CurrentIC;
extern CARD16   connect_id;
extern CARD16   icid;
extern XIMS     ims;

extern uint     iHZInputed;

Bool CreateVKWindow (void)
{
    XSetWindowAttributes attrib;
    unsigned long   attribmask;
    int             iBackPixel;

    attrib.override_redirect = True;
    attribmask = CWOverrideRedirect;

    sprintf (strVKWindowXPMBackColor, "@\tc #%02x%02x%02x", VKWindowColor.backColor.red >> 8, VKWindowColor.backColor.green >> 8, VKWindowColor.backColor.blue >> 8);
    sprintf (strVKWindowAlphaXPMColor, "-\tc #%02x%02x%02x", VKWindowAlphaColor.color.red >> 8, VKWindowAlphaColor.color.green >> 8, VKWindowAlphaColor.color.blue >> 8);

    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(VKWindowColor.backColor)))
	iBackPixel = VKWindowColor.backColor.pixel;
    else
	iBackPixel = WhitePixel (dpy, DefaultScreen (dpy));

    VKWindow = XCreateSimpleWindow (dpy, DefaultRootWindow (dpy), 0, 0, VK_WINDOW_WIDTH, VK_WINDOW_HEIGHT, 0, WhitePixel (dpy, DefaultScreen (dpy)), iBackPixel);
    if (VKWindow == (Window) NULL)
	return False;

    XChangeWindowAttributes (dpy, VKWindow, attribmask, &attrib);
    XSelectInput (dpy, VKWindow, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | EnterWindowMask | PointerMotionMask | LeaveWindowMask | VisibilityChangeMask);

    InitVKWindowColor ();
    LoadVKMapFile ();

    return True;
}

void InitVKWindowColor (void)
{
    XGCValues       values;
    int             iPixel;

    if (VKWindowFontColor.gc)
	XFreeGC (dpy, VKWindowFontColor.gc);
    VKWindowFontColor.gc = XCreateGC (dpy, VKWindow, 0, &values);
    if (XAllocColor (dpy, DefaultColormap (dpy, DefaultScreen (dpy)), &(VKWindowFontColor.color)))
	iPixel = VKWindowFontColor.color.pixel;
    else
	iPixel = WhitePixel (dpy, DefaultScreen (dpy));
    XSetForeground (dpy, VKWindowFontColor.gc, iPixel);
}

void DisplayVKWindow (void)
{
    XMapRaised (dpy, VKWindow);
}

void DrawVKWindow (void)
{
    int             i;
    int             iPos;
    int             rv;
    XImage         *mask;
    XpmAttributes   attrib;

    attrib.valuemask = 0;
    if (!pVKLogo) {
	rv = XpmCreateImageFromData (dpy, vk_xpm, &pVKLogo, &mask, &attrib);
	if (rv != XpmSuccess)
	    fprintf (stderr, "Failed to read xpm file: VK::VKLogo\n");
    }
    XPutImage (dpy, VKWindow, VKWindowColor.backGC, pVKLogo, 0, 0, 0, 0, VK_WINDOW_WIDTH, VK_WINDOW_HEIGHT);

    /* 显示字符 */
    /* 名称 */
#ifdef _USE_XFT
    OutputString (VKWindow, xftVKWindowFont, vks[iCurrentVK].strName, (VK_WINDOW_WIDTH - StringWidth (vks[iCurrentVK].strName, xftVKWindowFont)) / 2, iVKWindowFontSize + 6, VKWindowFontColor.color);
#else
    OutputString (VKWindow, fontSetVKWindow, vks[iCurrentVK].strName, (VK_WINDOW_WIDTH - StringWidth (vks[iCurrentVK].strName, fontSetVKWindow)) / 2, iVKWindowFontSize + 6, VKWindowFontColor.gc);
#endif

    /* 第一排 */
    iPos = 13;
    for (i = 0; i < 13; i++) {
#ifdef _USE_XFT
	OutputString (VKWindow, xftVKWindowFont, vks[iCurrentVK].strSymbol[i][1], iPos, 39, VKWindowFontColor.color);
	OutputString (VKWindow, xftVKWindowFont, vks[iCurrentVK].strSymbol[i][0], iPos - 5, 52, VKWindowFontColor.color);
	iPos += 24;
#else
	OutputString (VKWindow, fontSetVKWindow, vks[iCurrentVK].strSymbol[i][1], iPos, 39, VKWindowFontColor.gc);
	OutputString (VKWindow, fontSetVKWindow, vks[iCurrentVK].strSymbol[i][0], iPos - 5, 52, VKWindowFontColor.gc);
	iPos += 24;
#endif
    }
    /* 第二排 */
    iPos = 48;
    for (i = 13; i < 26; i++) {
#ifdef _USE_XFT
	OutputString (VKWindow, xftVKWindowFont, vks[iCurrentVK].strSymbol[i][1], iPos, 67, VKWindowFontColor.color);
	OutputString (VKWindow, xftVKWindowFont, vks[iCurrentVK].strSymbol[i][0], iPos - 5, 80, VKWindowFontColor.color);
	iPos += 24;
#else
	OutputString (VKWindow, fontSetVKWindow, vks[iCurrentVK].strSymbol[i][1], iPos, 67, VKWindowFontColor.gc);
	OutputString (VKWindow, fontSetVKWindow, vks[iCurrentVK].strSymbol[i][0], iPos - 5, 80, VKWindowFontColor.gc);
	iPos += 24;
#endif
    }
    /* 第三排 */
    iPos = 55;
    for (i = 26; i < 37; i++) {
#ifdef _USE_XFT
	OutputString (VKWindow, xftVKWindowFont, vks[iCurrentVK].strSymbol[i][1], iPos, 95, VKWindowFontColor.color);
	OutputString (VKWindow, xftVKWindowFont, vks[iCurrentVK].strSymbol[i][0], iPos - 5, 108, VKWindowFontColor.color);
	iPos += 24;
#else
	OutputString (VKWindow, fontSetVKWindow, vks[iCurrentVK].strSymbol[i][1], iPos, 95, VKWindowFontColor.gc);
	OutputString (VKWindow, fontSetVKWindow, vks[iCurrentVK].strSymbol[i][0], iPos - 5, 108, VKWindowFontColor.gc);
	iPos += 24;
#endif
    }
    if (bVKCaps)
	Draw3DEffect (VKWindow, 5, 85, 38, 25, _3D_LOWER);

    /* 第四排 */
    iPos = 72;
    for (i = 37; i < 47; i++) {
#ifdef _USE_XFT
	OutputString (VKWindow, xftVKWindowFont, vks[iCurrentVK].strSymbol[i][1], iPos, 123, VKWindowFontColor.color);
	OutputString (VKWindow, xftVKWindowFont, vks[iCurrentVK].strSymbol[i][0], iPos - 5, 136, VKWindowFontColor.color);
	iPos += 24;
#else
	OutputString (VKWindow, fontSetVKWindow, vks[iCurrentVK].strSymbol[i][1], iPos, 123, VKWindowFontColor.gc);
	OutputString (VKWindow, fontSetVKWindow, vks[iCurrentVK].strSymbol[i][0], iPos - 5, 136, VKWindowFontColor.gc);
	iPos += 24;
#endif
    }
    if (bShiftPressed)
	Draw3DEffect (VKWindow, 5, 113, 56, 25, _3D_LOWER);
}

/*
 * 处理相关鼠标键
 */
Bool VKMouseKey (int x, int y)
{
    int             iIndex;
    IMForwardEventStruct forwardEvent;
    char            strKey[3];
    char           *pstr;

    if (IsInBox (x, y, 1, 1, VK_WINDOW_WIDTH, 16))
	ChangVK ();
    else {
	if (!icid || !connect_id)
	    return False;

	strKey[1] = '\0';
	pstr = strKey;
	if (y >= 28 && y <= 55) {	//第一行
	    if (x < 4 || x > 348)
		return False;

	    x -= 4;
	    if (x >= 313 && x <= 344) {	//backspace
		MyIMForwardEvent (connect_id, icid, 22);
		return True;
	    }
	    else {
		iIndex = x / 24;
		if (iIndex > 12)	//避免出现错误
		    iIndex = 12;
		pstr = vks[iCurrentVK].strSymbol[iIndex][bShiftPressed ^ bVKCaps];
		if (bShiftPressed) {
		    bShiftPressed = False;
		    DrawVKWindow ();
		}
	    }
	}
	else if (y >= 56 && y <= 83) {	//第二行
	    if (x < 4 || x > 350)
		return False;

	    if (x >= 4 && x < 38) {	//Tab
		MyIMForwardEvent (connect_id, icid, 23);
		return True;
	    }
	    else {
		iIndex = 13 + (x - 38) / 24;
		pstr = vks[iCurrentVK].strSymbol[iIndex][bShiftPressed ^ bVKCaps];
		if (bShiftPressed) {
		    bShiftPressed = False;
		    DrawVKWindow ();
		}
	    }
	}
	else if (y >= 84 && y <= 111) {	//第三行
	    if (x < 4 || x > 350)
		return False;

	    if (x >= 4 && x < 44) {	//Caps
		//改变大写键状态
		bVKCaps = !bVKCaps;
		pstr = (char *) NULL;
		DrawVKWindow ();
	    }
	    else if (x > 308 && x <= 350)	//Return
		strKey[0] = '\n';
	    else {
		iIndex = 26 + (x - 44) / 24;
		pstr = vks[iCurrentVK].strSymbol[iIndex][bShiftPressed ^ bVKCaps];
		if (bShiftPressed) {
		    bShiftPressed = False;
		    DrawVKWindow ();
		}
	    }
	}
	else if (y >= 112 && y <= 139) {	//第四行
	    if (x < 4 || x > 302)
		return False;

	    if (x >= 4 && x < 62) {	//SHIFT
		//改变SHIFT键状态
		bShiftPressed = !bShiftPressed;
		pstr = (char *) NULL;
		DrawVKWindow ();
	    }
	    else {
		iIndex = 37 + (x - 62) / 24;
		pstr = vks[iCurrentVK].strSymbol[iIndex][bShiftPressed ^ bVKCaps];
		if (bShiftPressed) {
		    bShiftPressed = False;
		    DrawVKWindow ();
		}
	    }
	}
	else if (y >= 140 && y <= 162) {	//第五行         
	    if (x >= 4 && x < 38) {	//Ins
		//改变INS键状态
		MyIMForwardEvent (connect_id, icid, 106);
		return True;
	    }
	    else if (x >= 61 && x < 98) {	//DEL
		MyIMForwardEvent (connect_id, icid, 107);
		return True;
	    }
	    else if (x >= 99 && x < 270)	//空格
		strcpy (strKey, "\xa1\xa1");
	    else if (x >= 312 && x <= 350) {	//ESC
		SwitchVK ();
		pstr = (char *) NULL;
	    }
	    else
		return False;
	}

	if (pstr) {
	    memset (&forwardEvent, 0, sizeof (IMForwardEventStruct));
	    forwardEvent.connect_id = connect_id;
	    forwardEvent.icid = icid;
	    SendHZtoClient (&forwardEvent, pstr);
	    iHZInputed += (int) (strlen (pstr) / 2);	//粗略统计字数
	}
    }

    return True;
}

/*
 * 读取虚拟键盘映射文件
 */
void LoadVKMapFile (void)
{
    int             i, j;
    FILE           *fp;
    char            strPath[PATH_MAX];
    char           *pstr;

    for (j = 0; j < VK_MAX; j++) {
	for (i = 0; i < VK_NUMBERS; i++) {
	    vks[j].strSymbol[i][0][0] = '\0';
	    vks[j].strSymbol[i][1][0] = '\0';
	}
	vks[j].strName[0] = '\0';
    }

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");
    strcat (strPath, VK_FILE);

    if (access (strPath, 0)) {
	strcpy (strPath, PKGDATADIR "/data/");
	strcat (strPath, VK_FILE);
    }
    fp = fopen (strPath, "rt");

    if (!fp)
	return;

    iVKCount = 0;

    for (;;) {
	if (!fgets (strPath, PATH_MAX, fp))
	    break;
	pstr = strPath;
	while (*pstr == ' ' || *pstr == '\t')
	    pstr++;
	if (pstr[0] == '#')
	    continue;

	i = strlen (pstr) - 1;
	if (pstr[i] == '\n')
	    pstr[i] = '\0';
	if (!strlen (pstr))
	    continue;

	if (!strcmp (pstr, "[VK]"))
	    iVKCount++;
	else if (!strncmp (pstr, "NAME=", 5))
	    strcpy (vks[iVKCount - 1].strName, pstr + 5);
	else {
	    if (pstr[1] != '=' && !iVKCount)
		continue;

	    for (i = 0; i < VK_NUMBERS; i++) {
		if (vkTable[i] == tolower (pstr[0])) {
		    pstr += 2;
		    while (*pstr == ' ' || *pstr == '\t')
			pstr++;

		    if (!(*pstr))
			break;

		    j = 0;
		    while (*pstr && (*pstr != ' ' && *pstr != '\t'))
			vks[iVKCount - 1].strSymbol[i][0][j++] = *pstr++;
		    vks[iVKCount - 1].strSymbol[i][0][j] = '\0';

		    j = 0;
		    while (*pstr == ' ' || *pstr == '\t')
			pstr++;
		    if (*pstr) {
			while (*pstr && (*pstr != ' ' && *pstr != '\t'))
			    vks[iVKCount - 1].strSymbol[i][1][j++] = *pstr++;
			vks[iVKCount - 1].strSymbol[i][1][j] = '\0';
		    }

		    break;
		}
	    }
	}
    }

    fclose (fp);
}

/*
 * 根据字符查找符号
 */
char           *VKGetSymbol (char cChar)
{
    int             i;

    for (i = 0; i < VK_NUMBERS; i++) {
	if (MyToUpper (vkTable[i]) == cChar)
	    return vks[iCurrentVK].strSymbol[i][1];
	if (MyToLower (vkTable[i]) == cChar)
	    return vks[iCurrentVK].strSymbol[i][0];
    }

    return NULL;
}

/*
 * 上/下档键字符转换，以取代toupper和tolower
 */
int MyToUpper (int iChar)
{
    char           *pstr;

    pstr = strCharTable;
    while (*pstr) {
	if (*pstr == iChar)
	    return *(pstr + 1);
	pstr += 2;
    }

    return toupper (iChar);
}

int MyToLower (int iChar)
{
    char           *pstr;

    pstr = strCharTable + 1;
    for (;;) {
	if (*pstr == iChar)
	    return *(pstr - 1);
	if (!(*(pstr + 1)))
	    break;
	pstr += 2;
    }

    return tolower (iChar);
}

void ChangVK (void)
{
    iCurrentVK++;
    if (iCurrentVK == iVKCount)
	iCurrentVK = 0;

    bVKCaps = False;
    bShiftPressed = False;

    DrawVKWindow ();
    SwitchIM (-2);
    DrawMainWindow ();
}

INPUT_RETURN_VALUE DoVKInput (int iKey)
{
    char           *pstr;

    pstr = VKGetSymbol (iKey);
    if (!pstr)
	return IRV_DONOT_PROCESS_CLEAN;
    else {
	strcpy (strStringGet, pstr);
	return IRV_GET_CANDWORDS;
    }
}

void SwitchVK (void)
{
    if (!iVKCount)
	return;

    bVK = !bVK;
    if (bVK) {
	int             x, y;

	x = iMainWindowX;
	if ((x + VK_WINDOW_WIDTH) >= DisplayWidth (dpy, iScreen))
	    x = DisplayWidth (dpy, iScreen) - VK_WINDOW_WIDTH - 1;
	if (x < 0)
	    x = 0;

	y = iMainWindowY + MAINWND_HEIGHT + 2;
	if ((y + VK_WINDOW_HEIGHT) >= DisplayHeight (dpy, iScreen))
	    y = iMainWindowY - VK_WINDOW_HEIGHT - 2;
	if (y < 0)
	    y = 0;

	XMoveWindow (dpy, VKWindow, x, y);
	DisplayVKWindow ();
	XUnmapWindow (dpy, inputWindow);

	if (ConnectIDGetState (connect_id) == IS_CLOSED)
	    SetIMState (True);
    }
    else
	XUnmapWindow (dpy, VKWindow);

    SwitchIM (-2);
    DrawMainWindow ();
}
