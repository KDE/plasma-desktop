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
 * @file   ime.h
 * @author Yuking yuking_net@sohu.com
 * @date   2008-1-16
 * 
 * @brief  按键和输入法通用功能处理
 * 
 * 
 */

#ifndef _IME_H
#define _IME_H

#include <X11/keysym.h>
#include "xim.h"
#include "KeyList.h"

#define MAX_CAND_WORD	10
#define MAX_USER_INPUT	300

#define HOT_KEY_COUNT	2

#define MAX_IM_NAME	15

#define TEMP_FILE		"FCITX_DICT_TEMP"

typedef enum _SEARCH_MODE {
    SM_FIRST,
    SM_NEXT,
    SM_PREV
} SEARCH_MODE;

typedef enum ADJUST_ORDER {
    AD_NO,
    AD_FAST,
    AD_FREQ
} ADJUSTORDER;

typedef enum _INPUT_RETURN_VALUE {
    //IRV_UNKNOWN = -1,
    IRV_DO_NOTHING = 0,
    IRV_DONOT_PROCESS,
    IRV_DONOT_PROCESS_CLEAN,
    IRV_CLEAN,
    IRV_TO_PROCESS,
    IRV_DISPLAY_MESSAGE,
    IRV_DISPLAY_CANDWORDS,
    IRV_DISPLAY_LAST,
    IRV_PUNC,
    IRV_ENG,
    IRV_GET_LEGEND,
    IRV_GET_CANDWORDS,
    IRV_GET_CANDWORDS_NEXT
} INPUT_RETURN_VALUE;

typedef enum _ENTER_TO_DO {
    K_ENTER_NOTHING,
    K_ENTER_CLEAN,
    K_ENTER_SEND
} ENTER_TO_DO;

typedef enum _SEMICOLON_TO_DO {
    K_SEMICOLON_NOCHANGE,
    K_SEMICOLON_ENG,
    K_SEMICOLON_QUICKPHRASE
} SEMICOLON_TO_DO;

typedef struct _SINGLE_HZ {
    char            strHZ[3];
} SINGLE_HZ;

typedef enum _KEY_RELEASED {
    KR_OTHER = 0,
    KR_CTRL,
    KR_2ND_SELECTKEY,
    KR_2ND_SELECTKEY_OTHER,
    KR_3RD_SELECTKEY,
    KR_3RD_SELECTKEY_OTHER
} KEY_RELEASED;

typedef struct {
    char            strName[MAX_IM_NAME + 1];
    void            (*ResetIM) (void);
                    INPUT_RETURN_VALUE (*DoInput) (int);
                    INPUT_RETURN_VALUE (*GetCandWords) (SEARCH_MODE);
    char           *(*GetCandWord) (int);
    char           *(*GetLegendCandWord) (int);
                    Bool (*PhraseTips) (void);
    void            (*Init) (void);
    void            (*Destroy) (void);
} IM;

typedef int     HOTKEYS;

void            ProcessKey (IMForwardEventStruct * call_data);
void            ResetInput (void);
void            CloseIM (IMForwardEventStruct * call_data);
void            ChangeIMState (CARD16 call_data);
Bool            IsHotKey (int iKey, HOTKEYS * hotkey);
INPUT_RETURN_VALUE ChangeCorner (void);
INPUT_RETURN_VALUE ChangePunc (void);
INPUT_RETURN_VALUE ChangeGBK (void);
INPUT_RETURN_VALUE ChangeLegend (void);
INPUT_RETURN_VALUE ChangeTrack (void);
INPUT_RETURN_VALUE ChangeGBKT (void);

void            RegisterNewIM (char *strName, void (*ResetIM) (void), INPUT_RETURN_VALUE (*DoInput) (int), INPUT_RETURN_VALUE (*GetCandWords) (SEARCH_MODE), char *(*GetCandWord) (int), char *(*GetLegendCandWord) (int),
			       Bool (*PhraseTips) (void), void (*Init) (void), void (*Destroy) (void));
void            SwitchIM (INT8 index);
void            DoPhraseTips ();
Bool            IsIM (char *strName);
void            SaveIM (void);
void            SetIM (void);
void            ConvertPunc (void);

void            imeSendHZtoClient(char *pstr);

// Bool            IsKeyIgnored (int iKeyCode);

#endif
