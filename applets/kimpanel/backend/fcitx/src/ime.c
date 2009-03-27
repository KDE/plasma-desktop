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
/*
 * @file   ime.c
 * @author Yuking yuking_net@sohu.com
 * @date   2008-1-16
 * 
 * @brief  按键和输入法通用功能处理
 * 
 * 
 */

#include <ctype.h>
#include <time.h>

#ifdef _USE_XFT
#include <ft2build.h>
#include <X11/Xft/Xft.h>
#endif

#include "xim.h"
#include "ime.h"
#include "InputWindow.h"
#include "MainWindow.h"
#include "IC.h"
#include "punc.h"
#include "py.h"
#include "sp.h"
#include "qw.h"
#include "table.h"
#include "tools.h"
#include "ui.h"
#include "vk.h"
#include "QuickPhrase.h"
#include "AutoEng.h"
#include "extra.h"
#include "DBus.h"

IM             *im = NULL;
INT8            iIMCount = 0;
INT8            iState = 0;

IMForwardEventStruct current_call_data;

int             iMaxCandWord = 5;
int             iCandPageCount;
int             iCurrentCandPage;
int             iCandWordCount;

int             iLegendCandWordCount;
int             iLegendCandPageCount;
int             iCurrentLegendCandPage;

int             iCodeInputCount;

// *************************************************************
char            strCodeInput[MAX_USER_INPUT + 1];
char            strStringGet[MAX_USER_INPUT + 1];	//保存输入法返回的需要送到客户程序中的字串

// *************************************************************

ENTER_TO_DO     enterToDo = K_ENTER_SEND;

Bool            bCorner = False;	//全半角切换
Bool            bChnPunc = True;	//中英文标点切换
Bool            bUseGBK = False;	//是否支持GBK
Bool            bIsDoInputOnly = False;	//表明是否只由输入法来处理键盘
Bool            bLastIsNumber = False;	//上一次输入是不是阿拉伯数字
char		cLastIsAutoConvert = 0;	//上一次输入是不是符合数字后自动转换的符号，如'.'/','，0表示不是这样的符号
INT8            iInCap = 0;		//是不是处于大写后的英文状态,0--不，1--按下大写键，2--按下分号键

/*
   Bool            bAutoHideInputWindow = False;	//是否自动隐藏输入条
   */
Bool            bEngPuncAfterNumber = True;	//数字后面输出半角符号(只对'.'/','有效)
Bool            bPhraseTips = True;
INT8            lastIsSingleHZ = 0;
Bool            bUseGBKT = False;

SEMICOLON_TO_DO semicolonToDo = K_SEMICOLON_QUICKPHRASE;
Bool            bEngAfterCap = True;
Bool            bConvertPunc = True;
Bool            bDisablePagingInLegend = True;

Bool            bVK = False;

int             i2ndSelectKey = 0;	//第二个候选词选择键，为扫描码
int             i3rdSelectKey = 0;	//第三个候选词选择键，为扫描码

Time            lastKeyPressedTime;
unsigned int    iTimeInterval = 250;

KEY_RELEASED    keyReleased = KR_OTHER;
Bool            bDoubleSwitchKey = False;
KEY_CODE        switchKey = L_CTRL;

//热键定义
HOTKEYS         hkTrigger[HOT_KEY_COUNT] = { CTRL_SPACE, 0 };
HOTKEYS         hkGBK[HOT_KEY_COUNT] = { CTRL_M, 0 };
HOTKEYS         hkLegend[HOT_KEY_COUNT] = { CTRL_L, 0 };
HOTKEYS         hkCorner[HOT_KEY_COUNT] = { SHIFT_SPACE, 0 };	//全半角切换
HOTKEYS         hkPunc[HOT_KEY_COUNT] = { ALT_SPACE, 0 };	//中文标点
HOTKEYS         hkNextPage[HOT_KEY_COUNT] = { '.', 0 };	//下一页
HOTKEYS         hkPrevPage[HOT_KEY_COUNT] = { ',', 0 };	//上一页
HOTKEYS         hkTrack[HOT_KEY_COUNT] = { CTRL_K, 0 };
HOTKEYS         hkGBT[HOT_KEY_COUNT] = { CTRL_ALT_F, 0 };
HOTKEYS         hkHideMainWindow[HOT_KEY_COUNT] = { CTRL_ALT_H, 0 };

Bool            bUseLegend = False;
Bool            bIsInLegend = False;

INT8            iIMIndex = 0;
Bool            bUsePinyin = True;
Bool            bUseSP = True;
Bool            bUseQW = True;
Bool            bUseTable = True;
Bool            bLocked = False;

// dgod extern im
char		strExternIM[PATH_MAX];

Bool            bPointAfterNumber = True;

/* 计算打字速度 */
time_t          timeStart;
Bool            bStartRecordType;
Bool            bShowUserSpeed = True;
Bool            bShowVersion = True;
uint            iHZInputed = 0;

char            strNameOfPinyin[41] = "智能拼音";
char            strNameOfShuangpin[41] = "智能双拼";
char            strNameOfQuwei[41] = "区位";

Bool		bCursorAuto=False;

extern XIMS     ims;
extern IC      *CurrentIC;
extern Display *dpy;
extern ChnPunc *chnPunc;

extern MESSAGE  messageUp[];
extern uint     uMessageUp;
extern MESSAGE  messageDown[];
extern uint     uMessageDown;
extern Bool     bShowPrev;
extern Bool     bShowNext;
extern Bool     bShowCursor;
extern Bool     bTrackCursor;

extern Window   inputWindow;
extern HIDE_MAINWINDOW hideMainWindow;
extern XIMTriggerKey *Trigger_Keys;
extern Window   mainWindow;
extern int      iCursorPos;

extern Window   VKWindow;
extern VKS      vks[];
extern unsigned char iCurrentVK;
extern Bool     bVK;

extern int      MAINWND_WIDTH;
extern Bool     bCompactMainWindow;
extern Bool     bShowVK;

extern INT8     iTableChanged;
extern INT8     iNewPYPhraseCount;
extern INT8     iOrderCount;
extern INT8     iNewFreqCount;
extern INT16    iTableOrderChanged;

extern TABLE   *table;
extern INT8     iTableCount;

extern Bool     bTrigger;

extern int      iInputWindowX;
extern int      iInputWindowY;
extern int      iTempInputWindowX;
extern int      iTempInputWindowY;

extern Bool     bShowInputWindowTriggering;
extern Bool	bMainWindow_Hiden;

#ifdef _USE_XFT
extern XftFont *xftMainWindowFont;
#else
extern XFontSet fontSetMainWindow;
#endif
/*******************************************************/
extern Bool            bSetFocus;

void ResetInput (void)
{
    iCandPageCount = 0;
    iCurrentCandPage = 0;
    iCandWordCount = 0;
    iLegendCandWordCount = 0;
    iCurrentLegendCandPage = 0;
    iLegendCandPageCount = 0;
    iCursorPos = 0;

    strCodeInput[0] = '\0';
    iCodeInputCount = 0;

    bIsDoInputOnly = False;

    bShowPrev = False;
    bShowNext = False;

    bIsInLegend = False;
    iInCap = 0;

    if (!IsIM (strNameOfPinyin))
        bShowCursor = False;

    if (im[iIMIndex].ResetIM)
        im[iIMIndex].ResetIM ();

    uMessageUp = 0;
    uMessageDown = 0;

    updateMessages();
}

void CloseIM (IMForwardEventStruct * call_data)
{
    XUnmapWindow (dpy, inputWindow);
    XUnmapWindow (dpy, VKWindow);

    IMPreeditEnd (ims, (XPointer) call_data);
    SetConnectID (call_data->connect_id, IS_CLOSED);
    bVK = False;
    SwitchIM (-2);
//    DrawMainWindow ();
    updateMessages();
}

void ChangeIMState (CARD16 _connect_id)
{
    if (ConnectIDGetState (_connect_id) == IS_ENG) {
        SetConnectID (_connect_id, IS_CHN);
        iState = IS_CHN;
//X         if (bVK)
//X             DisplayVKWindow ();
//X         else
//X             DisplayInputWindow ();

        if (ConnectIDGetTrackCursor (_connect_id)) {
//X             XMoveWindow (dpy, inputWindow, iTempInputWindowX, iTempInputWindowY);
            fprintf(stderr,"SpotLocation: %d,%d\n",iTempInputWindowX,iTempInputWindowY);
            KIMUpdateSpotLocation(iTempInputWindowX,iTempInputWindowY);
        } else {
//X             XMoveWindow (dpy, inputWindow, iInputWindowX, iInputWindowY);
//            KIMUpdateSpotLocation(iInputWindowX,iInputWindowY);
            fprintf(stderr,"SpotLocation: %d,%d\n",iInputWindowX,iInputWindowY);
            KIMUpdateSpotLocation(iInputWindowX,iInputWindowY);
        }
    }
    else {
        SetConnectID (_connect_id, IS_ENG);
        iState = IS_ENG;
        ResetInput ();
        ResetInputWindow ();

        XUnmapWindow (dpy, inputWindow);
//X         XUnmapWindow (dpy, VKWindow);
    }

//X     if (hideMainWindow != HM_HIDE)
//X         DrawMainWindow ();
    updateProperty(&state_prop);
}

/*
 * 转换strStringGet中的标点为中文标点
 */
void ConvertPunc (void)
{
    char            strTemp[MAX_USER_INPUT + 1] = "\0";
    char           *s1, *s2, *pPunc;

    s1 = strTemp;
    s2 = strStringGet;

    while (*s2) {
        pPunc = GetPunc (*s2);
        if (pPunc) {
            strcat (s1, pPunc);
            s1 += strlen (pPunc);
        }
        else
            *s1++ = *s2;
        s2++;
    }
    *s2 = '\0';

    strcpy (strStringGet, strTemp);
}


void ProcessKey (IMForwardEventStruct * call_data)
{
    current_call_data = *call_data;
    KeySym          keysym;
    XKeyEvent      *kev;
    int             keyCount;
    INPUT_RETURN_VALUE retVal;
    int             iKeyState;
    char            strbuf[STRBUFLEN];
    int             iKey;
    char           *pstr;
    int             iLen;

    kev = (XKeyEvent *) & call_data->event;
    memset (strbuf, 0, STRBUFLEN);
    keyCount = XLookupString (kev, strbuf, STRBUFLEN, &keysym, NULL);

    iKeyState = kev->state - (kev->state & KEY_NUMLOCK) - (kev->state & KEY_CAPSLOCK) - (kev->state & KEY_SCROLLLOCK);
    iKey = GetKey (keysym, iKeyState, keyCount);

    if (!iKey) {
        IMForwardEvent (ims, (XPointer) call_data);
        return;
    }

    /*
     * 原意是为了解决xine-ui中候选字自动选中的问题
     * xine-ui每秒钟产生一个左SHIFT键的释放事件
     * 但这段代码对新的xine-ui已经不起作用了
     */
    if (kev->same_screen && (kev->keycode == switchKey || kev->keycode == i2ndSelectKey || kev->keycode == i3rdSelectKey)) {
        IMForwardEvent (ims, (XPointer) call_data);
        return;
    }

    retVal = IRV_TO_PROCESS;

#ifdef _DEBUG
    printf ("KeyRelease=%d  iKeyState=%d  KEYCODE=%d  iKey=%d\n", (call_data->event.type == KeyRelease), iKeyState, kev->keycode, iKey);
#endif

    /*
     * 有时候，当某个程序获得焦点后的第一个按键被解释成一个KeyRelease事件，此处试图解决该问题。原因不清楚。
     */
    if ( bSetFocus ) {
        call_data->event.type = KeyPress;
        bSetFocus = False;
    }
    /* ************************************************************* */

    if (call_data->event.type == KeyRelease) {	
        if (ConnectIDGetState (call_data->connect_id) != IS_CLOSED) {
            if ((kev->time - lastKeyPressedTime) < 500 && (!bIsDoInputOnly)) {
                if (!bLocked && iKeyState == KEY_CTRL_SHIFT_COMP && (iKey == 225 || iKey == 227)) {
                    if (ConnectIDGetState (call_data->connect_id) == IS_CHN) {
                        SwitchIM (-1);
                    } else if (IsHotKey (iKey, hkTrigger))
                        CloseIM (call_data);
                }
                else if (!bLocked && iKey == CTRL_LSHIFT) {
                    if (ConnectIDGetState (call_data->connect_id) == IS_CHN)
                        SwitchIM (-1);
                    else if (IsHotKey (iKey, hkTrigger))
                        CloseIM (call_data);
                }
                else if (kev->keycode == switchKey && keyReleased == KR_CTRL && !bDoubleSwitchKey) {
                    /* 按下临时英文键会把已经输入的英文送到客户程序中，但实际使用发现，这种方式不具人性化
                     * if (iCodeInputCount) {
                     strcpy (strStringGet, strCodeInput);
                     retVal = IRV_ENG;
                     }
                     else
                     retVal = IRV_TO_PROCESS;
                     */
                    keyReleased = KR_OTHER;
                    ChangeIMState (call_data->connect_id);
                    retVal = IRV_DONOT_PROCESS;
                }
                else if ((kev->keycode == i2ndSelectKey && keyReleased == KR_2ND_SELECTKEY) || (iKey == (i2ndSelectKey ^ 0xFF) && keyReleased == KR_2ND_SELECTKEY_OTHER)) {
                    if (!bIsInLegend) {
                        pstr = im[iIMIndex].GetCandWord (1);
                        if (pstr) {
                            strcpy (strStringGet, pstr);
                            if (bIsInLegend)
                                retVal = IRV_GET_LEGEND;
                            else
                                retVal = IRV_GET_CANDWORDS;
                        }
                        else if (iCandWordCount)
                            retVal = IRV_DISPLAY_CANDWORDS;
                        else
                            retVal = IRV_TO_PROCESS;
                    }
                    else {
                        strcpy (strStringGet, " ");
                        uMessageDown = 0;
                        retVal = IRV_GET_CANDWORDS;
                    }

                    keyReleased = KR_OTHER;
                }
                else if ((iKey == (i3rdSelectKey ^ 0xFF) && keyReleased == KR_3RD_SELECTKEY_OTHER) || (kev->keycode == i3rdSelectKey && keyReleased == KR_3RD_SELECTKEY)) {
                    if (!bIsInLegend) {
                        pstr = im[iIMIndex].GetCandWord (2);
                        if (pstr) {
                            strcpy (strStringGet, pstr);
                            if (bIsInLegend)
                                retVal = IRV_GET_LEGEND;
                            else
                                retVal = IRV_GET_CANDWORDS;
                        }
                        else if (iCandWordCount)
                            retVal = IRV_DISPLAY_CANDWORDS;
                    }
                    else {
                        strcpy (strStringGet, "　");
                        uMessageDown = 0;
                        retVal = IRV_GET_CANDWORDS;
                    }

                    keyReleased = KR_OTHER;
                }
                else if (kev->keycode == i3rdSelectKey && keyReleased == KR_3RD_SELECTKEY) {
                    if (!bIsInLegend) {
                        pstr = im[iIMIndex].GetCandWord (2);
                        if (pstr) {
                            strcpy (strStringGet, pstr);
                            if (bIsInLegend)
                                retVal = IRV_GET_LEGEND;
                            else
                                retVal = IRV_GET_CANDWORDS;
                        }
                        else if (iCandWordCount)
                            retVal = IRV_DISPLAY_CANDWORDS;
                    }
                    else {
                        strcpy (strStringGet, "　");
                        uMessageDown = 0;
                        retVal = IRV_GET_CANDWORDS;
                    }

                    keyReleased = KR_OTHER;
                }
            }
        }
        else
            call_data->event.type = KeyPress;
    }

    if (retVal == IRV_TO_PROCESS) {
        if (call_data->event.type == KeyPress) {
            if (kev->keycode != switchKey)
                keyReleased = KR_OTHER;
            else {
                if ((keyReleased == KR_CTRL) && (kev->time - lastKeyPressedTime < iTimeInterval) && bDoubleSwitchKey) {
                    SendHZtoClient(call_data, strCodeInput);
                    ChangeIMState (call_data->connect_id);
                }
            }

            lastKeyPressedTime = kev->time;
            if (kev->keycode == switchKey) {
                keyReleased = KR_CTRL;
                retVal = IRV_DO_NOTHING;
            }
            else if (IsHotKey (iKey, hkTrigger)) {
                if (ConnectIDGetState (call_data->connect_id) == IS_ENG) {
                    SetConnectID (call_data->connect_id, IS_CHN);

                    if (bShowInputWindowTriggering && !bCorner) {
//X                         DrawInputWindow ();
//X                         DisplayInputWindow ();
                    }

                    if (ConnectIDGetTrackCursor (call_data->connect_id)) {
                        //X             XMoveWindow (dpy, inputWindow, iTempInputWindowX, iTempInputWindowY);
                        fprintf(stderr,"SpotLocation: %d,%d\n",iTempInputWindowX,iTempInputWindowY);
                        KIMUpdateSpotLocation(iTempInputWindowX,iTempInputWindowY);
                    } else {
                        //X             XMoveWindow (dpy, inputWindow, iInputWindowX, iInputWindowY);
                        //            KIMUpdateSpotLocation(iInputWindowX,iInputWindowY);
                        fprintf(stderr,"SpotLocation: %d,%d\n",iInputWindowX,iInputWindowY);
                        KIMUpdateSpotLocation(iInputWindowX,iInputWindowY);
                    }
                    EnterChineseMode (False);
                    //                    DrawMainWindow ();
                }
                else
                    CloseIM (call_data);

                retVal = IRV_DO_NOTHING;
            }
        }

        if (retVal == IRV_TO_PROCESS) {
            if (call_data->event.type == KeyPress) {
                if (ConnectIDGetState (call_data->connect_id) == IS_CHN) {
                    if (bVK)
                        retVal = DoVKInput (iKey);
                    else {
                        if (iKeyState == KEY_NONE) {
                            if (kev->keycode == i2ndSelectKey)
                                keyReleased = KR_2ND_SELECTKEY;
                            else if (kev->keycode == i3rdSelectKey)
                                keyReleased = KR_3RD_SELECTKEY;
                            else if (iKey == (i2ndSelectKey ^ 0xFF)) {
                                if (iCandWordCount >= 2) {
                                    keyReleased = KR_2ND_SELECTKEY_OTHER;
                                    return;
                                }
                            }
                            else if (iKey == (i3rdSelectKey ^ 0xFF)) {
                                if (iCandWordCount >= 2) {
                                    keyReleased = KR_3RD_SELECTKEY_OTHER;
                                    return;
                                }
                            }
                        }

                        if (iKey == CTRL_LSHIFT || iKey == SHIFT_LCTRL) {
                            if (bLocked)
                                retVal = IRV_TO_PROCESS;
                        }
                        else {
                            //调用输入法模块
                            if (bCorner && (iKey >= 32 && iKey <= 126)) {
                                //有人报 空格 的全角不对，正确的是0xa1 0xa1
                                //但查资料却说全角符号总是以0xa3开始。
                                //由于0xa3 0xa0可能会显示乱码，因此采用0xa1 0xa1的方式
                                if (iKey == ' ')
                                    sprintf (strStringGet, "%c%c", 0xa1, 0xa1);
                                else
                                    sprintf (strStringGet, "%c%c", 0xa3, 0xa0 + iKey - 32);
                                retVal = IRV_GET_CANDWORDS;
                            }
                            else {
                                if (!iInCap) {
                                    char            strTemp[MAX_USER_INPUT];

                                    retVal = im[iIMIndex].DoInput (iKey);
                                    if (!bCursorAuto && !IsIM (strNameOfPinyin) && !IsIM (strNameOfShuangpin))
                                        iCursorPos = iCodeInputCount;

                                    //为了实现自动英文转换
                                    strcpy (strTemp, strCodeInput);
                                    if (retVal == IRV_TO_PROCESS) {
                                        strTemp[strlen (strTemp) + 1] = '\0';
                                        strTemp[strlen (strTemp)] = iKey;
                                    }

                                    if (SwitchToEng (strTemp)) {
                                        iInCap = 3;
                                        if (retVal != IRV_TO_PROCESS) {
                                            iCodeInputCount--;
                                            retVal = IRV_TO_PROCESS;
                                        }
                                    }

                                    if (iKey!= (XK_BackSpace & 0x00FF))
                                        cLastIsAutoConvert = 0;
                                }
                                else if (iInCap == 2 && semicolonToDo == K_SEMICOLON_QUICKPHRASE)
                                    retVal = QuickPhraseDoInput (iKey);

                                if (!bIsDoInputOnly && retVal == IRV_TO_PROCESS) {
                                    if (!iInCap && iKey >= 'A' && iKey <= 'Z' && bEngAfterCap && !(kev->state & KEY_CAPSLOCK)) {
                                        iInCap = 1;
                                        if (!bIsInLegend && iCandWordCount) {
                                            pstr = im[iIMIndex].GetCandWord (0);
                                            iCandWordCount = 0;
                                            if (pstr) {
                                                SendHZtoClient (call_data, pstr);
                                                strcpy (strStringGet, pstr);
                                                //粗略统计字数
                                                iHZInputed += (int) (strlen (strStringGet) / 2);
                                                iCodeInputCount = 0;
                                            }
                                        }
                                    }
                                    else if (iKey == ';' && semicolonToDo != K_SEMICOLON_NOCHANGE && !iCodeInputCount) {
                                        if (iInCap != 2)
                                            iInCap = 2;
                                        else
                                            iKey = ' ';	//使用第2个分号输入中文分号
                                    }
                                    else if (!iInCap) {
                                        if (IsHotKey (iKey, hkPrevPage))
                                            retVal = im[iIMIndex].GetCandWords (SM_PREV);
                                        else if (IsHotKey (iKey, hkNextPage))
                                            retVal = im[iIMIndex].GetCandWords (SM_NEXT);
                                    }

                                    if (retVal == IRV_TO_PROCESS) {
                                        if (iInCap) {
                                            if ((iKey == ' ') && (iCodeInputCount == 0)) {
                                                strcpy (strStringGet, "；");
                                                retVal = IRV_ENG;
                                                uMessageUp = uMessageDown = 0;
                                                iInCap = 0;
                                            }
                                            else {
                                                if (isprint (iKey) && iKey < 128) {
                                                    if (iCodeInputCount == MAX_USER_INPUT)
                                                        retVal = IRV_DO_NOTHING;
                                                    else {
                                                        if (!(iInCap == 2 && !iCodeInputCount && iKey == ';')) {
                                                            strCodeInput[iCodeInputCount++] = iKey;
                                                            strCodeInput[iCodeInputCount] = '\0';
                                                            bShowCursor = True;
                                                            iCursorPos = iCodeInputCount;
                                                            if (semicolonToDo == K_SEMICOLON_QUICKPHRASE && iInCap == 2)
                                                                retVal = QuickPhraseGetCandWords (SM_FIRST);
                                                            else
                                                                retVal = IRV_DISPLAY_MESSAGE;
                                                        }
                                                        else
                                                            retVal = IRV_DISPLAY_MESSAGE;
                                                    }
                                                }
                                                else if (iKey == (XK_BackSpace & 0x00FF) || iKey == CTRL_H) {
                                                    if (iCodeInputCount)
                                                        iCodeInputCount--;
                                                    strCodeInput[iCodeInputCount] = '\0';
                                                    iCursorPos = iCodeInputCount;
                                                    if (!iCodeInputCount)
                                                        retVal = IRV_CLEAN;
                                                    else if (semicolonToDo == K_SEMICOLON_QUICKPHRASE && iInCap == 2)
                                                        retVal = QuickPhraseGetCandWords (SM_FIRST);
                                                    else
                                                        retVal = IRV_DISPLAY_MESSAGE;
                                                }

                                                uMessageUp = 1;
                                                if (iInCap == 2) {
                                                    if (semicolonToDo == K_SEMICOLON_ENG) {
                                                        strcpy (messageUp[0].strMsg, "英文输入 ");
                                                        iCursorPos += 9;
                                                    }
                                                    else {
                                                        strcpy (messageUp[0].strMsg, "自定义输入 ");
                                                        iCursorPos += 11;
                                                    }

                                                    if (iCodeInputCount) {
                                                        uMessageUp = 2;
                                                        //strcat (messageUp[0].strMsg, "  ");
                                                        strcpy (messageUp[1].strMsg, strCodeInput);
                                                        messageUp[1].type = MSG_INPUT;
                                                        iCursorPos += iCodeInputCount;
                                                    }

                                                    if (retVal != IRV_DISPLAY_CANDWORDS) {
                                                        if (iCodeInputCount)
                                                            strcpy (messageDown[0].strMsg, "按 Enter 输入英文");
                                                        else
                                                            strcpy (messageDown[0].strMsg, "空格输入；Enter输入;");
                                                        uMessageDown = 1;
                                                        messageDown[0].type = MSG_TIPS;
                                                    }
                                                    messageUp[0].type = MSG_TIPS;
                                                }
                                                else {						    
                                                    uMessageDown = 1;
                                                    messageDown[0].type = MSG_TIPS;
                                                    strcpy (messageUp[0].strMsg, strCodeInput);
                                                    strcpy (messageDown[0].strMsg, "按 Enter 输入英文");
                                                    messageUp[0].type = MSG_INPUT;
                                                }
                                            }
                                        }
                                        else if ((bLastIsNumber && bEngPuncAfterNumber) && (iKey == '.' || iKey == ',' || iKey == ':') && !iCandWordCount) {
                                            cLastIsAutoConvert = iKey;
                                            bLastIsNumber = False;
                                            retVal = IRV_TO_PROCESS;					    
                                        }
                                        else {				    
                                            if (bChnPunc) {
                                                char           *pPunc;

                                                pstr = NULL;
                                                pPunc = GetPunc (iKey);
                                                if (pPunc) {
                                                    strStringGet[0] = '\0';
                                                    if (!bIsInLegend)
                                                        pstr = im[iIMIndex].GetCandWord (0);
                                                    if (pstr)
                                                        strcpy (strStringGet, pstr);
                                                    strcat (strStringGet, pPunc);
                                                    uMessageDown = uMessageUp = 0;

                                                    retVal = IRV_PUNC;
                                                }
                                                else if ((iKey == (XK_BackSpace & 0x00FF) || iKey == CTRL_H) && cLastIsAutoConvert ) {
                                                    char *pPunc;

                                                    IMForwardEvent (ims, (XPointer) call_data);
                                                    pPunc = GetPunc(cLastIsAutoConvert);
                                                    if ( pPunc )
                                                        SendHZtoClient(call_data, pPunc);

                                                    retVal = IRV_DO_NOTHING;
                                                }
                                                else if (isprint (iKey) && iKey < 128) {
                                                    if (iKey >= '0' && iKey <= '9')
                                                        bLastIsNumber = True;
                                                    else {
                                                        bLastIsNumber = False;
                                                        if (iKey == ' ')
                                                            retVal = IRV_DONOT_PROCESS_CLEAN;	//为了与mozilla兼容
                                                        else {
                                                            strStringGet[0] = '\0';
                                                            if (!bIsInLegend)
                                                                pstr = im[iIMIndex].GetCandWord (0);
                                                            if (pstr)
                                                                strcpy (strStringGet, pstr);
                                                            iLen = strlen (strStringGet);
                                                            uMessageDown = uMessageUp = 0;
                                                            strStringGet[iLen] = iKey;
                                                            strStringGet[iLen + 1] = '\0';
                                                            retVal = IRV_ENG;
                                                        }
                                                    }
                                                }
                                            }
                                            cLastIsAutoConvert = 0;
                                        }
                                    }
                                }

                                if (retVal == IRV_TO_PROCESS) {
                                    if (iKey == ESC) {
                                        if (iCodeInputCount || iInCap || bIsInLegend)
                                            retVal = IRV_CLEAN;
                                        else
                                            retVal = IRV_DONOT_PROCESS;
                                    }
                                    else if (iKey == CTRL_5) {
                                        LoadConfig (False);

                                        //X                                         InitGC (inputWindow);
                                        //X                                         InitMainWindowColor ();
                                        //X                                         InitInputWindowColor ();
                                        //X                                         InitVKWindowColor ();

                                        SetIM ();
                                        //X                                         CreateFont ();
                                        //X                                         CalculateInputWindowHeight ();

                                        FreeQuickPhrase ();
                                        LoadQuickPhrase ();

                                        FreeAutoEng ();
                                        LoadAutoEng ();

                                        FreePunc ();
                                        LoadPuncDict ();
                                        SwitchIM(-2);
                                        //                                      DrawMainWindow();

                                        retVal = IRV_DO_NOTHING;
                                    }
                                    else if (iKey == ENTER) {
                                        if (iInCap) {
                                            if (!iCodeInputCount)
                                                strcpy (strStringGet, ";");
                                            else
                                                strcpy (strStringGet, strCodeInput);
                                            retVal = IRV_PUNC;
                                            uMessageUp = uMessageDown = 0;
                                            iInCap = 0;
                                        }
                                        else if (!iCodeInputCount)
                                            retVal = IRV_DONOT_PROCESS;
                                        else {
                                            switch (enterToDo) {
                                            case K_ENTER_NOTHING:
                                                retVal = IRV_DO_NOTHING;
                                                break;
                                            case K_ENTER_CLEAN:
                                                retVal = IRV_CLEAN;
                                                break;
                                            case K_ENTER_SEND:
                                                uMessageDown = uMessageUp = 0;
                                                strcpy (strStringGet, strCodeInput);
                                                retVal = IRV_ENG;
                                                break;
                                            }
                                        }
                                    }
                                    else if (isprint (iKey) && iKey < 128)
                                        retVal = IRV_DONOT_PROCESS_CLEAN;
                                    else
                                        retVal = IRV_DONOT_PROCESS;
                                }
                            }
                        }
                    }
                }

                if (retVal == IRV_TO_PROCESS || retVal == IRV_DONOT_PROCESS) {
                    if (IsHotKey (iKey, hkCorner))
                        retVal = ChangeCorner ();
                    else if (IsHotKey (iKey, hkPunc))
                        retVal = ChangePunc ();
                    else if (IsHotKey (iKey, hkGBK))
                        retVal = ChangeGBK ();
                    else if (IsHotKey (iKey, hkLegend))
                        retVal = ChangeLegend ();
                    else if (IsHotKey (iKey, hkTrack))
                        retVal = ChangeTrack ();
                    else if (IsHotKey (iKey, hkGBT))
                        retVal = ChangeGBKT ();
                    else if (IsHotKey (iKey, hkHideMainWindow)) {
                        if (bMainWindow_Hiden) {
                            bMainWindow_Hiden = False;
                            //			    DisplayMainWindow();
                            //			    DrawMainWindow();
                        }
                        else {
                            bMainWindow_Hiden = True;
                            //			    XUnmapWindow(dpy,mainWindow);
                        }
                        retVal = IRV_DO_NOTHING;			
                    }
                }
            }
            else
                retVal = IRV_DONOT_PROCESS;
        }
    }

    switch (retVal) {
    case IRV_DO_NOTHING:
        break;
    case IRV_TO_PROCESS:
    case IRV_DONOT_PROCESS:
    case IRV_DONOT_PROCESS_CLEAN:
        IMForwardEvent (ims, (XPointer) call_data);

        if (retVal != IRV_DONOT_PROCESS_CLEAN)
            return;
    case IRV_CLEAN:
        ResetInput ();
        updateMessages();
        //	ResetInputWindow ();
        //	XUnmapWindow (dpy, inputWindow);

        return;
    case IRV_DISPLAY_CANDWORDS:
        bShowNext = bShowPrev = False;
        if (bIsInLegend) {
            if (iCurrentLegendCandPage > 0)
                bShowPrev = True;
            if (iCurrentLegendCandPage < iLegendCandPageCount)
                bShowNext = True;
        }
        else {
            if (iCurrentCandPage > 0)
                bShowPrev = True;
            if (iCurrentCandPage < iCandPageCount)
                bShowNext = True;
        }

        fprintf(stderr,"IRV_DISPLAY_CANDWORDS: Up: %d Down: %d\n",uMessageUp,uMessageDown);
        updateMessages();
        //	DrawInputWindow ();
        //	DisplayInputWindow ();

        break;
    case IRV_DISPLAY_LAST:
        bShowNext = bShowPrev = False;
        uMessageUp = 1;
        messageUp[0].strMsg[0] = strCodeInput[0];
        messageUp[0].strMsg[1] = '\0';
        messageUp[0].type = MSG_INPUT;
        uMessageDown = 1;
        strcpy (messageDown[0].strMsg, strStringGet);
        messageDown[0].type = MSG_TIPS;

        fprintf(stderr,"IRV_DISPLAY_LAST Up: %d Down: %d\n",uMessageUp,uMessageDown);
        updateMessages();
        //	DrawInputWindow ();
        //	DisplayInputWindow ();
        break;
    case IRV_DISPLAY_MESSAGE:
        bShowNext = False;
        bShowPrev = False;

        fprintf(stderr,"Up: %d Down: %d\n",uMessageUp,uMessageDown);
        updateMessages();
        //	DrawInputWindow ();
        //	DisplayInputWindow ();
        break;
    case IRV_GET_LEGEND:
        SendHZtoClient (call_data, strStringGet);
        iHZInputed += (int) (strlen (strStringGet) / 2);	//粗略统计字数
        if (iLegendCandWordCount) {
            bShowNext = bShowPrev = False;
            if (iCurrentLegendCandPage > 0)
                bShowPrev = True;
            if (iCurrentLegendCandPage < iLegendCandPageCount)
                bShowNext = True;
            bLastIsNumber = False;
            iCodeInputCount = 0;
            //	    DrawInputWindow ();
            //	    DisplayInputWindow ();
            fprintf(stderr,"IRV_GET_LEGEND Up: %d Down: %d\n",uMessageUp,uMessageDown);
            updateMessages();
        }
        else {
            ResetInput ();
            updateMessages();
//            XUnmapWindow (dpy, inputWindow);
        }

        break;
    case IRV_GET_CANDWORDS:
        SendHZtoClient (call_data, strStringGet);
        bLastIsNumber = False;
        if (bPhraseTips && im[iIMIndex].PhraseTips && !bVK)
            DoPhraseTips ();
        iHZInputed += (int) (strlen (strStringGet) / 2);	//粗略统计字数
        ResetInput ();

#if 0
        if (bVK || (!uMessageDown && (!bPhraseTips || (bPhraseTips && !lastIsSingleHZ))))
            XUnmapWindow (dpy, inputWindow);
        else
            DrawInputWindow ();
#endif

        fprintf(stderr,"IRV_GET_CANDWORDS Up: %d Down: %d\n",uMessageUp,uMessageDown);
        updateMessages();
        lastIsSingleHZ = 0;

        break;
    case IRV_ENG:
        fprintf(stderr,"IRV_ENG\n");
        //如果处于中文标点模式，应该将其中的标点转换为全角
        if (bChnPunc && bConvertPunc)
            ConvertPunc ();
    case IRV_PUNC:
        fprintf(stderr,"IRV_PUNC\n");
        iHZInputed += (int) (strlen (strStringGet) / 2);	//粗略统计字数
        ResetInput ();
        if (!uMessageDown)
            XUnmapWindow (dpy, inputWindow);
    case IRV_GET_CANDWORDS_NEXT:
        fprintf(stderr,"IRV_GET_CANDWORDS_NEXT\n");
        SendHZtoClient (call_data, strStringGet);
        bLastIsNumber = False;
        lastIsSingleHZ = 0;

        if (retVal == IRV_GET_CANDWORDS_NEXT || lastIsSingleHZ == -1) {
            iHZInputed += (int) (strlen (strStringGet) / 2);	//粗略统计字数
            //	    DrawInputWindow ();
            updateMessages();
            fprintf(stderr,"IRV_GET_CANDWORDS_NEXT Up: %d Down: %d\n",uMessageUp,uMessageDown);
        }

        break;
    default:
        ;
    }

    //计算打字速度的功能
    if (retVal == IRV_DISPLAY_MESSAGE || retVal == IRV_DISPLAY_CANDWORDS || retVal == IRV_PUNC) {
        if (!bStartRecordType) {
            bStartRecordType = True;
            timeStart = time (NULL);
        }
    }
}

Bool IsHotKey (int iKey, HOTKEYS * hotkey)
{
    if (iKey == hotkey[0] || iKey == hotkey[1])
        return True;
    return False;
}

INPUT_RETURN_VALUE ChangeCorner (void)
{
    ResetInput ();
    ResetInputWindow ();

    bCorner = !bCorner;
    //    DrawMainWindow ();

    SaveProfile ();

    updateProperty(&corner_prop);

    return IRV_DO_NOTHING;
}

INPUT_RETURN_VALUE ChangePunc (void)
{
    bChnPunc = !bChnPunc;
    //    DrawMainWindow ();
    SaveProfile ();

    updateProperty(&punc_prop);

    return IRV_DO_NOTHING;
}

INPUT_RETURN_VALUE ChangeGBK (void)
{
    bUseGBK = !bUseGBK;
    ResetInput ();
    ResetInputWindow ();

    //    DrawMainWindow ();
    XUnmapWindow (dpy, inputWindow);

    SaveProfile ();

    updateProperty(&gbk_prop);

    return IRV_CLEAN;
}

INPUT_RETURN_VALUE ChangeGBKT (void)
{
    bUseGBKT = !bUseGBKT;
    ResetInput ();
    ResetInputWindow ();

    //X     DrawMainWindow ();
    //X     XUnmapWindow (dpy, inputWindow);

    SaveProfile ();

    updateProperty(&gbkt_prop);

    return IRV_CLEAN;
}

INPUT_RETURN_VALUE ChangeLegend (void)
{
    bUseLegend = !bUseLegend;
    ResetInput ();
    //    ResetInputWindow ();

    //X     DrawMainWindow ();
    //X     XUnmapWindow (dpy, inputWindow);

    SaveProfile ();

    updateProperty(&legend_prop);

    return IRV_CLEAN;
}

INPUT_RETURN_VALUE ChangeTrack (void)
{
    bTrackCursor = !bTrackCursor;
    SaveProfile ();

    return IRV_DO_NOTHING;;
}

void ChangeLock (void)
{
    bLocked = !bLocked;
    //    DrawMainWindow ();

    SaveProfile ();
}

void SwitchIM (INT8 index)
{
    INT8            iLastIM;

    if (index != (INT8) - 2 && bVK)
        return;

    iLastIM = (iIMIndex >= iIMCount) ? (iIMCount - 1) : iIMIndex;
    if (index == (INT8) - 1) {
        if (iIMIndex == (iIMCount - 1))
            iIMIndex = 0;
        else
            iIMIndex++;
    }
    else if (index != (INT8) - 2) {
        if (index >= iIMCount)
            iIMIndex = iIMCount - 1;
        else {
            iIMIndex = index;
        }
    }

#if 0
#ifdef _USE_XFT
    MAINWND_WIDTH = ((bCompactMainWindow) ? _MAINWND_WIDTH_COMPACT : _MAINWND_WIDTH) + StringWidth ((bVK) ? vks[iCurrentVK].strName : im[iIMIndex].strName, xftMainWindowFont) + 4;
#else
    MAINWND_WIDTH = ((bCompactMainWindow) ? _MAINWND_WIDTH_COMPACT : _MAINWND_WIDTH) + StringWidth ((bVK) ? vks[iCurrentVK].strName : im[iIMIndex].strName, fontSetMainWindow) + 4;
#endif
    if (!bShowVK && bCompactMainWindow)
        MAINWND_WIDTH -= 24;

    XResizeWindow (dpy, mainWindow, MAINWND_WIDTH, MAINWND_HEIGHT);

    DrawMainWindow ();
#endif

    if (iIMCount == 1)
        return;

    if (index != (INT8) - 2) {
        if (im[iLastIM].Destroy)
            im[iLastIM].Destroy ();
        if (im[iIMIndex].Init)
            im[iIMIndex].Init ();
    }

    ResetInput ();
    //    XUnmapWindow (dpy, inputWindow);

    SaveProfile ();

    // logo_prop handled specially
    if ((index == (INT8)-2) || (index == (INT8)-2)) {
        logo_prop.label = "Fcitx";
        iState = IS_ENG;
    } else {
        logo_prop.label = g2u(im[iIMIndex].strName);
        iState = IS_CHN;
    }
    updateProperty(&logo_prop);
    updateProperty(&state_prop);
}

void DoPhraseTips (void)
{
    if (!bPhraseTips)
        return;

    if (im[iIMIndex].PhraseTips ())
        lastIsSingleHZ = -1;
    else
        lastIsSingleHZ = 0;
}

void RegisterNewIM (char *strName, void (*ResetIM) (void),
        INPUT_RETURN_VALUE (*DoInput) (int), INPUT_RETURN_VALUE (*GetCandWords) (SEARCH_MODE), char *(*GetCandWord) (int), char *(*GetLegendCandWord) (int), Bool (*PhraseTips) (void), void (*Init) (void), void (*Destroy) (void))
{
#ifdef _DEBUG
    printf ("REGISTER %s\n", strName);
#endif
    strcpy (im[iIMCount].strName, strName);
    im[iIMCount].ResetIM = ResetIM;
    im[iIMCount].DoInput = DoInput;
    im[iIMCount].GetCandWords = GetCandWords;
    im[iIMCount].GetCandWord = GetCandWord;
    im[iIMCount].GetLegendCandWord = GetLegendCandWord;
    im[iIMCount].PhraseTips = PhraseTips;
    im[iIMCount].Init = Init;
    im[iIMCount].Destroy = Destroy;

    iIMCount++;
}

Bool IsIM (char *strName)
{
    if (strstr (im[iIMIndex].strName, strName))
        return True;

    return False;
}

void SaveIM (void)
{
    if (iTableChanged || iTableOrderChanged)
        SaveTableDict ();
    if (iNewPYPhraseCount)
        SavePYUserPhrase ();
    if (iOrderCount)
        SavePYIndex ();
    if (iNewFreqCount)
        SavePYFreq ();
}

void SetIM (void)
{
    INT8            i;

    if (im)
        free (im);

    if (bUseTable)
        LoadTableInfo ();

    iIMCount = iTableCount;
    if (bUsePinyin)
        iIMCount++;
    if (bUseSP)
        iIMCount++;
    if (bUseQW)
        iIMCount++;

    iIMCount += EIM_MAX;
    if (!iIMCount)
        iIMCount = 1;

    im = (IM *) malloc (sizeof (IM) * iIMCount);
    iIMCount = 0;

    /* 加入输入法 */
    if (bUsePinyin || ((!bUseSP && (!bUseTable || !iTableCount)) && !iIMCount))	//至少应该有一种输入法
        RegisterNewIM (strNameOfPinyin, ResetPYStatus, DoPYInput, PYGetCandWords, PYGetCandWord, PYGetLegendCandWord, NULL, PYInit, NULL);
    if (bUseSP)
        RegisterNewIM (strNameOfShuangpin, ResetPYStatus, DoPYInput, PYGetCandWords, PYGetCandWord, PYGetLegendCandWord, NULL, SPInit, NULL);
    if (bUseQW)
        RegisterNewIM (strNameOfQuwei, NULL, DoQWInput, QWGetCandWords, QWGetCandWord, NULL, NULL, NULL, NULL);
    if (bUseTable) {
        for (i = 0; i < iTableCount; i++) {
            RegisterNewIM (table[i].strName, TableResetStatus, DoTableInput, TableGetCandWords, TableGetCandWord, TableGetLegendCandWord, TablePhraseTips, TableInit, FreeTableIM);
            table[i].iIMIndex = iIMCount - 1;
        }
    }

    if (strExternIM[0] && strExternIM[1])
        LoadExtraIM(strExternIM);

    SwitchIM (iIMIndex);
}

