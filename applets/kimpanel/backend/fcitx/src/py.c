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

#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h>
#endif

#include "ime.h"
#include "tools.h"
#include "InputWindow.h"
#include "py.h"
#include "PYFA.h"
#include "pyParser.h"
#include "sp.h"

int             iPYFACount;
PYFA           *PYFAList;
uint            iCounter = 0;
Bool            bPYBaseDictLoaded = False;
Bool            bPYOtherDictLoaded = False;

/*
Bool            bSingleHZMode = False;	//这种情况下，只进行单字输入，不进行词组的匹配、生成及保存，用于码表输入法中的拼音输入
*/
PyFreq         *pyFreq = NULL, *pCurFreq = NULL;
uint            iPYFreqCount = 0;

char            strFindString[MAX_USER_INPUT + 1];
ParsePYStruct   findMap;
int             iPYInsertPoint = 0;

char            strPYLegendSource[MAX_WORDS_USER_INPUT * 2 + 1] = "\0";
char            strPYLegendMap[MAX_WORDS_USER_INPUT * 2 + 1] = "\0";
PyBase         *pyBaseForLengend;
PYLegendCandWord PYLegendCandWords[MAX_CAND_WORD];

PY_SELECTED     pySelected[MAX_WORDS_USER_INPUT + 1];
uint            iPYSelected = 0;

PYCandWord      PYCandWords[MAX_CAND_WORD];
char            strPYAuto[MAX_WORDS_USER_INPUT * 2 + 1];
char            strPYAutoMap[MAX_WORDS_USER_INPUT * 2 + 1];
Bool            bPYCreateAuto = True;
Bool            bPYSaveAutoAsPhrase = False;

ADJUSTORDER     baseOrder = AD_FREQ;
ADJUSTORDER     phraseOrder = AD_FAST;
ADJUSTORDER     freqOrder = AD_NO;

INT8            iNewPYPhraseCount = 0;
INT8            iOrderCount = 0;
INT8            iNewFreqCount = 0;

INT8            iYCDZ = 0;
char            cPYYCDZ[3] = "[]";	//以词定字

HOTKEYS         hkPYAddFreq[HOT_KEY_COUNT] = { CTRL_8, 0 };
Bool            bIsPYAddFreq = False;
HOTKEYS         hkPYDelFreq[HOT_KEY_COUNT] = { CTRL_7, 0 };
Bool            bIsPYDelFreq = False;
HOTKEYS         hkPYDelUserPhr[HOT_KEY_COUNT] = { CTRL_DELETE, 0 };
Bool            bIsPYDelUserPhr = False;

Bool            bFullPY = False;	//该变量指示只要全拼输入(不使用简拼)

extern Bool     bIsInLegend;
extern Bool     bUseLegend;
extern Bool     bSP_UseSemicolon;
extern Bool     bSP;
extern int      iCandWordCount;
extern int      iCandPageCount;
extern int      iCodeInputCount;
extern int      iCurrentCandPage;
extern char     strCodeInput[];

extern int      iCursorPos;

extern int      iMaxCandWord;
extern MESSAGE  messageUp[];
extern uint     uMessageUp;
extern MESSAGE  messageDown[];
extern uint     uMessageDown;

extern char     strStringGet[];
extern Bool     bIsDoInputOnly;
extern Bool     bShowCursor;

extern int      iLegendCandWordCount;
extern int      iLegendCandPageCount;
extern int      iCurrentLegendCandPage;
extern Bool     bDisablePagingInLegend;
extern Bool     bPointAfterNumber;

void PYInit (void)
{
    bSP = False;
}

Bool LoadPYBaseDict (void)
{
    FILE           *fp;
    char            strPath[PATH_MAX];

/*
#if defined(DARWIN)
    unsigned int    i, j, iLen;
#else*/
    int             i, j, iLen;

/*
#endif
*/

    strcpy (strPath, PKGDATADIR "/data/");
    strcat (strPath, PY_BASE_FILE);
    fp = fopen (strPath, "rb");
    if (!fp)
	return False;

    fread (&iPYFACount, sizeof (int), 1, fp);
    PYFAList = (PYFA *) malloc (sizeof (PYFA) * iPYFACount);
    for (i = 0; i < iPYFACount; i++) {
	fread (PYFAList[i].strMap, sizeof (char) * 2, 1, fp);
	PYFAList[i].strMap[2] = '\0';
	PYFAList[i].iChangeCount = 0;
	fread (&(PYFAList[i].iBase), sizeof (int), 1, fp);
	PYFAList[i].pyBase = (PyBase *) malloc (sizeof (PyBase) * PYFAList[i].iBase);
	for (j = 0; j < PYFAList[i].iBase; j++) {
	    fread (PYFAList[i].pyBase[j].strHZ, sizeof (char) * 2, 1, fp);
	    PYFAList[i].pyBase[j].strHZ[2] = '\0';
	    fread (&iLen, sizeof (int), 1, fp);
	    PYFAList[i].pyBase[j].iIndex = iLen;
	    PYFAList[i].pyBase[j].iHit = 0;
	    PYFAList[i].pyBase[j].flag = 0;
	    if (iLen > iCounter)
		iCounter = iLen;
	    PYFAList[i].pyBase[j].iPhrase = 0;
	    PYFAList[i].pyBase[j].iUserPhrase = 0;
	    PYFAList[i].pyBase[j].userPhrase = (PyPhrase *) malloc (sizeof (PyPhrase));
	    PYFAList[i].pyBase[j].userPhrase->next = PYFAList[i].pyBase[j].userPhrase;
	    PYFAList[i].pyBase[j].iChangeCount = 0;
	}
    }

    fclose (fp);
    bPYBaseDictLoaded = True;

    pyFreq = (PyFreq *) malloc (sizeof (PyFreq));
    pyFreq->next = NULL;

    return True;
}

Bool LoadPYOtherDict (void)
{
    //下面开始读系统词组    
    FILE           *fp;
    char            strPath[PATH_MAX];
    int             i, j, k, iLen;
    char            strBase[3];
    PyPhrase       *phrase, *temp;
    uint            iIndex;
    PyFreq         *pyFreqTemp, *pPyFreq;
    HZ             *HZTemp, *pHZ;

    bPYOtherDictLoaded = True;

    strcpy (strPath, PKGDATADIR "/data/");
    strcat (strPath, PY_PHRASE_FILE);
    fp = fopen (strPath, "rb");
    if (!fp)
	fprintf (stderr, "\nCan not find System Database of Pinyin! %s\n", strPath);
    else {
	while (!feof (fp)) {
	    if (!fread (&i, sizeof (int), 1, fp))
		break;
	    if (!fread (strBase, sizeof (char) * 2, 1, fp))
		break;
	    strBase[2] = '\0';
	    if (!fread (&k, sizeof (int), 1, fp))
		break;
/*
#if defined(DARWIN)
            k = ReverseInt (k);
#endif
*/
	    j = GetBaseIndex (i, strBase);
	    if (j == -1)
		break;

	    PYFAList[i].pyBase[j].iPhrase = k;
	    PYFAList[i].pyBase[j].phrase = (PyPhrase *) malloc (sizeof (PyPhrase) * k);

	    for (k = 0; k < PYFAList[i].pyBase[j].iPhrase; k++) {
		fread (&iLen, sizeof (int), 1, fp);
		PYFAList[i].pyBase[j].phrase[k].strMap = (char *) malloc (sizeof (char) * (iLen + 1));
		fread (PYFAList[i].pyBase[j].phrase[k].strMap, sizeof (char) * iLen, 1, fp);
		PYFAList[i].pyBase[j].phrase[k].strMap[iLen] = '\0';
		PYFAList[i].pyBase[j].phrase[k].strPhrase = (char *) malloc (sizeof (char) * (iLen + 1));
		fread (PYFAList[i].pyBase[j].phrase[k].strPhrase, sizeof (char) * iLen, 1, fp);
		PYFAList[i].pyBase[j].phrase[k].strPhrase[iLen] = '\0';
		fread (&iLen, sizeof (unsigned int), 1, fp);
		PYFAList[i].pyBase[j].phrase[k].iIndex = iLen;
		if (iLen > iCounter)
		    iCounter = iLen;
		PYFAList[i].pyBase[j].phrase[k].iHit = 0;
		PYFAList[i].pyBase[j].phrase[k].flag = 0;
	    }
	}
	fclose (fp);
    }

    //下面开始读取用户词库
    snprintf(strPath, sizeof(strPath), "%s/.fcitx/%s",
             getenv ("HOME"),
             PY_USERPHRASE_FILE);
    fp = fopen (strPath, "rb");
    if (fp) {
	while (!feof (fp)) {
	    if (!fread (&i, sizeof (int), 1, fp))
		break;
	    if (!fread (strBase, sizeof (char) * 2, 1, fp))
		break;
	    strBase[2] = '\0';
	    if (!fread (&k, sizeof (int), 1, fp))
		break;

	    j = GetBaseIndex (i, strBase);

	    PYFAList[i].pyBase[j].iUserPhrase = k;
	    temp = PYFAList[i].pyBase[j].userPhrase;
	    for (k = 0; k < PYFAList[i].pyBase[j].iUserPhrase; k++) {
		phrase = (PyPhrase *) malloc (sizeof (PyPhrase));
		fread (&iLen, sizeof (int), 1, fp);
		phrase->strMap = (char *) malloc (sizeof (char) * (iLen + 1));
		fread (phrase->strMap, sizeof (char) * iLen, 1, fp);
		phrase->strMap[iLen] = '\0';
		phrase->strPhrase = (char *) malloc (sizeof (char) * (iLen + 1));
		fread (phrase->strPhrase, sizeof (char) * iLen, 1, fp);
		phrase->strPhrase[iLen] = '\0';
		fread (&iLen, sizeof (int), 1, fp);
		phrase->iIndex = iLen;
		if (iLen > iCounter)
		    iCounter = iLen;
		fread (&iLen, sizeof (int), 1, fp);
		phrase->iHit = iLen;
		phrase->flag = 0;

		phrase->next = temp->next;
		temp->next = phrase;

		temp = phrase;
	    }
	}

	fclose (fp);
    }

    //下面读取索引文件
    snprintf(strPath, sizeof(strPath), "%s/.fcitx/%s",
             getenv ("HOME"),
             PY_INDEX_FILE);
    fp = fopen (strPath, "rb");
    if (fp) {
	fread (&iLen, sizeof (uint), 1, fp);
	if (iLen > iCounter)
	    iCounter = iLen;
	while (!feof (fp)) {
	    fread (&i, sizeof (int), 1, fp);
	    fread (&j, sizeof (int), 1, fp);
	    fread (&k, sizeof (int), 1, fp);
	    fread (&iIndex, sizeof (uint), 1, fp);
	    fread (&iLen, sizeof (uint), 1, fp);

	    if (i < iPYFACount) {
		if (j < PYFAList[i].iBase) {
		    if (k < PYFAList[i].pyBase[j].iPhrase) {
			if (k >= 0) {
			    PYFAList[i].pyBase[j].phrase[k].iIndex = iIndex;
			    PYFAList[i].pyBase[j].phrase[k].iHit = iLen;
			}
			else {
			    PYFAList[i].pyBase[j].iIndex = iIndex;
			    PYFAList[i].pyBase[j].iHit = iLen;
			}
		    }
		}
	    }
	}

	fclose (fp);
    }

    //下面读取常用词表
    snprintf(strPath, sizeof(strPath), "%s/.fcitx/%s",
             getenv ("HOME"),
             PY_FREQ_FILE);
    fp = fopen (strPath, "rb");
    if (fp) {
	pPyFreq = pyFreq;

	fread (&iPYFreqCount, sizeof (uint), 1, fp);

	for (i = 0; i < iPYFreqCount; i++) {
	    pyFreqTemp = (PyFreq *) malloc (sizeof (PyFreq));
	    pyFreqTemp->next = NULL;
	    pyFreqTemp->bIsSym = False;

	    fread (pyFreqTemp->strPY, sizeof (char) * 11, 1, fp);
	    fread (&j, sizeof (int), 1, fp);
	    pyFreqTemp->iCount = j;

	    pyFreqTemp->HZList = (HZ *) malloc (sizeof (HZ));
	    pyFreqTemp->HZList->next = NULL;
	    pHZ = pyFreqTemp->HZList;

	    for (k = 0; k < pyFreqTemp->iCount; k++) {
		HZTemp = (HZ *) malloc (sizeof (HZ));
		fread (HZTemp->strHZ, sizeof (char) * 2, 1, fp);
		HZTemp->strHZ[2] = '\0';
		fread (&j, sizeof (int), 1, fp);
		HZTemp->iPYFA = j;
		fread (&j, sizeof (int), 1, fp);
		HZTemp->iHit = j;
		fread (&j, sizeof (int), 1, fp);
		HZTemp->iIndex = j;
		HZTemp->flag = 0;
		pHZ->next = HZTemp;
		pHZ = HZTemp;
	    }

	    pPyFreq->next = pyFreqTemp;
	    pPyFreq = pyFreqTemp;
	}

	fclose (fp);
    }

    //下面读取特殊符号表
    snprintf(strPath, sizeof(strPath), "%s/.fcitx/%s",
             getenv ("HOME"),
             PY_SYMBOL_FILE);

    if (access (strPath, 0)) {
	strcpy (strPath, PKGDATADIR "/data/");
	strcat (strPath, PY_SYMBOL_FILE);
    }

    fp = fopen (strPath, "rt");
    if (fp) {
	char            strTxt[256];
	char            str1[MAX_PY_PHRASE_LENGTH * MAX_PY_LENGTH + 1], str2[MAX_PY_PHRASE_LENGTH * 2 + 1];
	char           *str;

	for (;;) {
	    if (!fgets (strTxt, 255, fp))
		break;
	    i = strlen (strTxt) - 1;
	    if (strTxt[0] == '#')
		continue;
	    if (strTxt[i] == '\n')
		strTxt[i] = '\0';
	    str = strTxt;
	    while (*str == ' ' || *str == '\t')
		str++;
	    if (!strlen (str))
		continue;
	    sscanf (str, "%s %s", str1, str2);

	    //首先看看str1是否已经在列表中
	    pyFreqTemp = pyFreq->next;
	    pPyFreq = pyFreq;
	    while (pyFreqTemp) {
		if (!strcmp (pyFreqTemp->strPY, str1))
		    break;
		pPyFreq = pPyFreq->next;
		pyFreqTemp = pyFreqTemp->next;
	    }

	    if (!pyFreqTemp) {
		pyFreqTemp = (PyFreq *) malloc (sizeof (PyFreq));
		strcpy (pyFreqTemp->strPY, str1);
		pyFreqTemp->next = NULL;
		pyFreqTemp->iCount = 0;
		pyFreqTemp->bIsSym = True;
		pyFreqTemp->HZList = (HZ *) malloc (sizeof (HZ));
		pyFreqTemp->HZList->next = NULL;
		pPyFreq->next = pyFreqTemp;
		iPYFreqCount++;
	    }
	    else {
		if (!pyFreqTemp->bIsSym)	//不能与常用字的编码相同
		    continue;
	    }

	    HZTemp = (HZ *) malloc (sizeof (HZ));
	    strcpy (HZTemp->strHZ, str2);
	    HZTemp->next = NULL;
	    pyFreqTemp->iCount++;

	    pHZ = pyFreqTemp->HZList;
	    while (pHZ->next)
		pHZ = pHZ->next;

	    pHZ->next = HZTemp;
	}

	fclose (fp);
    }

    return True;
}

void ResetPYStatus ()
{
    iPYInsertPoint = 0;
    iPYSelected = 0;
    strFindString[0] = '\0';
    strPYAuto[0] = '\0';

    bIsPYAddFreq = False;
    bIsPYDelFreq = False;
    bIsPYDelUserPhr = False;
    //bSingleHZMode = False;

    findMap.iMode = PARSE_SINGLEHZ;	//只要不是PARSE_ERROR就可以
}

int GetBaseIndex (int iPYFA, char *strBase)
{
    int             i;

    for (i = 0; i < PYFAList[iPYFA].iBase; i++) {
	if (!strcmp (strBase, PYFAList[iPYFA].pyBase[i].strHZ))
	    return i;
    }

    return -1;
}

INPUT_RETURN_VALUE DoPYInput (int iKey)
{
    int             i = 0;
    int             val;
    char           *strGet = NULL;
    char            strTemp[MAX_USER_INPUT + 1];

    if (!bPYBaseDictLoaded)
	LoadPYBaseDict ();
    //if (!bSingleHZMode && !bPYOtherDictLoaded)
    if (!bPYOtherDictLoaded)
	LoadPYOtherDict ();

    val = IRV_TO_PROCESS;
    if (!bIsPYAddFreq && !bIsPYDelFreq && !bIsPYDelUserPhr) {
	if ((iKey >= 'a' && iKey <= 'z') || iKey == PY_SEPARATOR || (bSP && bSP_UseSemicolon && iKey == ';')) {
	    bIsInLegend = False;
	    bShowCursor = True;

	    if (iKey == PY_SEPARATOR) {
		if (!iPYInsertPoint)
		    return IRV_TO_PROCESS;
		if (strFindString[iPYInsertPoint - 1] == PY_SEPARATOR)
		    return IRV_DO_NOTHING;
	    }

	    val = i = strlen (strFindString);

	    for (; i > iPYInsertPoint; i--)
		strFindString[i] = strFindString[i - 1];

	    strFindString[iPYInsertPoint++] = iKey;
	    strFindString[val + 1] = '\0';
	    ParsePY (strFindString, &findMap, PY_PARSE_INPUT_USER);

	    val = 0;
	    for (i = 0; i < iPYSelected; i++)
		val += strlen (pySelected[i].strHZ) / 2;

	    if (findMap.iHZCount > (MAX_WORDS_USER_INPUT - val)) {
		UpdateFindString ();
		ParsePY (strFindString, &findMap, PY_PARSE_INPUT_USER);
	    }

	    val = IRV_DISPLAY_CANDWORDS;
	}
	else if (iKey == (XK_BackSpace & 0x00FF) || iKey == CTRL_H) {
	    if (iPYInsertPoint) {
		val = ((iPYInsertPoint > 1) && (strFindString[iPYInsertPoint - 2] == PY_SEPARATOR)) ? 2 : 1;
		strcpy (strFindString + iPYInsertPoint - val, strFindString + iPYInsertPoint);
		ParsePY (strFindString, &findMap, PY_PARSE_INPUT_USER);
		val = IRV_DISPLAY_CANDWORDS;
		iPYInsertPoint--;

		if (!strlen (strFindString)) {
		    if (!iPYSelected)
			return IRV_CLEAN;

		    val = strlen (strFindString);
		    strcpy (strTemp, pySelected[iPYSelected - 1].strPY);
		    strcat (strTemp, strFindString);
		    strcpy (strFindString, strTemp);
		    iPYInsertPoint = strlen (strFindString) - val;
		    iPYSelected--;
		    ParsePY (strFindString, &findMap, PY_PARSE_INPUT_USER);
		}

		val = IRV_DISPLAY_CANDWORDS;
	    }
	}
	else if (iKey == (XK_Delete & 0x00FF)) {
	    if (iPYInsertPoint == strlen (strFindString))
		return IRV_DONOT_PROCESS;
	    val = (strFindString[iPYInsertPoint + 1] == PY_SEPARATOR) ? 2 : 1;
	    strcpy (strFindString + iPYInsertPoint, strFindString + iPYInsertPoint + val);
	    ParsePY (strFindString, &findMap, PY_PARSE_INPUT_USER);
	    if (!strlen (strFindString))
		return IRV_CLEAN;
	    val = IRV_DISPLAY_CANDWORDS;
	}
	else if (iKey == HOME) {
	    if (iCodeInputCount == 0)
		return IRV_DONOT_PROCESS;
	    iPYInsertPoint = 0;
	    val = IRV_DISPLAY_CANDWORDS;
	}
	else if (iKey == END) {
	    if (iCodeInputCount == 0)
		return IRV_DONOT_PROCESS;
	    iPYInsertPoint = strlen (strFindString);
	    val = IRV_DISPLAY_CANDWORDS;
	}
	else if (iKey == RIGHT) {
	    if (!iCodeInputCount)
		return IRV_TO_PROCESS;
	    if (iPYInsertPoint == strlen (strFindString))
		return IRV_DO_NOTHING;
	    iPYInsertPoint++;
	    val = IRV_DISPLAY_CANDWORDS;
	}
	else if (iKey == LEFT) {
	    if (!iCodeInputCount)
		return IRV_TO_PROCESS;
	    if (iPYInsertPoint < 2) {
		if (iPYSelected) {
		    char            strTemp[MAX_USER_INPUT + 1];

		    val = strlen (strFindString);
		    strcpy (strTemp, pySelected[iPYSelected - 1].strPY);
		    strcat (strTemp, strFindString);
		    strcpy (strFindString, strTemp);
		    iPYInsertPoint = strlen (strFindString) - val;
		    iPYSelected--;
		    ParsePY (strFindString, &findMap, PY_PARSE_INPUT_USER);

		    val = IRV_DISPLAY_CANDWORDS;
		}
		else if (iPYInsertPoint) {
		    iPYInsertPoint--;
		    val = IRV_DISPLAY_CANDWORDS;
		}
		else
		    val = IRV_DO_NOTHING;
	    }
	    else {
		iPYInsertPoint--;
		val = IRV_DISPLAY_CANDWORDS;
	    }
	}
	else if (iKey == ' ') {
	    if (!bIsInLegend) {
		if (findMap.iMode == PARSE_ERROR)
		    return IRV_DO_NOTHING;

		if (!iCandWordCount) {
		    if (iCodeInputCount == 1 && strCodeInput[0] == ';' && bSP) {
			strcpy (strStringGet, "；");
			return IRV_PUNC;
		    }
		    return IRV_TO_PROCESS;
		}

		strGet = PYGetCandWord (0);

		if (strGet) {
		    strcpy (strStringGet, strGet);

		    if (bIsInLegend)
			val = IRV_GET_LEGEND;
		    else
			val = IRV_GET_CANDWORDS;
		}
		else
		    val = IRV_DISPLAY_CANDWORDS;
	    }
	    else {
		strcpy (strStringGet, PYGetLegendCandWord (0));
		val = IRV_GET_LEGEND;
	    }
	}
	else if (IsHotKey (iKey, hkPYDelUserPhr)) {
	    if (!bIsPYDelUserPhr) {
		for (val = 0; val < iCandWordCount; val++) {
		    if (PYCandWords[val].iWhich == PY_CAND_USERPHRASE)
			goto _NEXT;
		}
		return IRV_TO_PROCESS;

	      _NEXT:
		bIsPYDelUserPhr = True;
		bIsDoInputOnly = True;

		uMessageUp = 1;
		strcpy (messageUp[0].strMsg, "选择标号的用户词组将被删除(ESC取消)");
		messageUp[0].type = MSG_TIPS;
		bShowCursor = False;

		return IRV_DISPLAY_MESSAGE;
	    }
	}
	else if (IsHotKey (iKey, hkPYAddFreq)) {
	    if (!bIsPYAddFreq && findMap.iHZCount == 1 && iCodeInputCount) {
		bIsPYAddFreq = True;
		bIsDoInputOnly = True;

		uMessageUp = 1;
		sprintf (messageUp[0].strMsg, "选择标号的字将进入 %s 的常用字表(ESC取消)", strFindString);
		messageUp[0].type = MSG_TIPS;
		bShowCursor = False;

		return IRV_DISPLAY_MESSAGE;
	    }
	}
	else if (IsHotKey (iKey, hkPYDelFreq)) {
	    if (!bIsPYDelFreq && (pCurFreq && !pCurFreq->bIsSym)) {
		for (val = 0; val < iCandWordCount; val++) {
		    if (PYCandWords[val].iWhich != PY_CAND_FREQ)
			break;
		}

		if (!val)
		    return IRV_DO_NOTHING;
		else if (val == 1)
		    sprintf (messageUp[0].strMsg, "按 1 删除 %s 的常用字(ESC取消)", strFindString);
		else
		    sprintf (messageUp[0].strMsg, "按 1-%d 删除 %s 的常用字(ESC取消)", val, strFindString);

		bIsPYDelFreq = True;
		bIsDoInputOnly = True;

		uMessageUp = 1;
		messageUp[0].type = MSG_TIPS;
		bShowCursor = False;

		return IRV_DISPLAY_MESSAGE;
	    }
	}
    }

    if (val == IRV_TO_PROCESS) {
	if (iKey >= '0' && iKey <= '9') {
	    iKey -= '0';
	    if (iKey == 0)
		iKey = 10;

	    if (!bIsInLegend) {
		if (!iCodeInputCount)
		    return IRV_TO_PROCESS;
		else if (!iCandWordCount || (iKey > iCandWordCount))
		    return IRV_DO_NOTHING;
		else {
		    if (bIsPYAddFreq || bIsPYDelFreq || bIsPYDelUserPhr) {
			if (bIsPYAddFreq) {
			    PYAddFreq (iKey - 1);
			    bIsPYAddFreq = False;
			}
			else if (bIsPYDelFreq) {
			    PYDelFreq (iKey - 1);
			    bIsPYDelFreq = False;
			}
			else {
			    if (PYCandWords[iKey - 1].iWhich == PY_CAND_USERPHRASE)
				PYDelUserPhrase (PYCandWords[iKey - 1].cand.phrase.iPYFA, PYCandWords[iKey - 1].cand.phrase.iBase, PYCandWords[iKey - 1].cand.phrase.phrase);
			    bIsPYDelUserPhr = False;
			}
			bIsDoInputOnly = False;
			bShowCursor = True;

			val = IRV_DISPLAY_CANDWORDS;
		    }
		    else {
			strGet = PYGetCandWord (iKey - 1);
			if (strGet) {
			    strcpy (strStringGet, strGet);

			    if (bIsInLegend)
				val = IRV_GET_LEGEND;
			    else
				val = IRV_GET_CANDWORDS;
			}
			else
			    val = IRV_DISPLAY_CANDWORDS;
		    }
		}
	    }
	    else {
		strcpy (strStringGet, PYGetLegendCandWord (iKey - 1));
		val = IRV_GET_LEGEND;
	    }
	}
	else if (iKey == -1) {
	    ParsePY (strFindString, &findMap, PY_PARSE_INPUT_USER);
	    iPYInsertPoint = 0;
	    val = IRV_DISPLAY_CANDWORDS;
	}
	else if (iKey == ESC)
	    return IRV_TO_PROCESS;
	else {
	    if (bIsPYAddFreq || bIsPYDelFreq || bIsPYDelUserPhr)
		return IRV_DO_NOTHING;

	    //下面实现以词定字
	    if (iCandWordCount) {
		if (iKey == cPYYCDZ[0] || iKey == cPYYCDZ[1]) {
		    if (PYCandWords[iYCDZ].iWhich == PY_CAND_USERPHRASE || PYCandWords[iYCDZ].iWhich == PY_CAND_SYMPHRASE) {
			char           *pBase, *pPhrase;

			pBase = PYFAList[PYCandWords[iYCDZ].cand.phrase.iPYFA].pyBase[PYCandWords[iYCDZ].cand.phrase.iBase].strHZ;
			pPhrase = PYCandWords[iYCDZ].cand.phrase.phrase->strPhrase;

			if (iKey == cPYYCDZ[0])
			    strcpy (strStringGet, pBase);
			else {
			    strncpy (strStringGet, pPhrase, 2);
			    strStringGet[2] = '\0';
			}
			uMessageDown = 0;
			return IRV_GET_CANDWORDS;
		    }
		}
		else if (!bIsInLegend) {
		    val = -1;
		    switch (iKey) {
		    case ')':
			val++;
		    case '(':
			val++;
		    case '*':
			val++;
		    case '&':
			val++;
		    case '^':
			val++;
		    case '%':
			val++;
		    case '$':
			val++;
		    case '#':
			val++;
		    case '@':
			val++;
		    case '!':
			val++;
		    }

		    if (val != -1 && val < iCandWordCount) {
			iYCDZ = val;
			PYCreateCandString ();
			return IRV_DISPLAY_CANDWORDS;
		    }

		    return IRV_TO_PROCESS;
		}
	    }
	    else
		return IRV_TO_PROCESS;
	}
    }

    if (!bIsInLegend) {
	UpdateCodeInputPY ();
	CalculateCursorPosition ();
    }

    if (val == IRV_DISPLAY_CANDWORDS) {
	if (iPYSelected) {
	    uMessageUp = 1;
	    messageUp[0].strMsg[0] = '\0';
	    for (i = 0; i < iPYSelected; i++)
		strcat (messageUp[0].strMsg, pySelected[i].strHZ);
	    messageUp[0].type = MSG_OTHER;
	}
	else
	    uMessageUp = 0;

	for (i = 0; i < findMap.iHZCount; i++) {
	    strcpy (messageUp[uMessageUp].strMsg, findMap.strPYParsed[i]);
	    strcat (messageUp[uMessageUp].strMsg, " ");
	    messageUp[uMessageUp++].type = MSG_CODE;
	}

	return PYGetCandWords (SM_FIRST);
    }

    return (INPUT_RETURN_VALUE) val;
}

/*
 * 本函数根据当前插入点计算光标的实际位置
 */
void CalculateCursorPosition (void)
{
    int             i;
    int             iTemp;

    iCursorPos = 0;
    for (i = 0; i < iPYSelected; i++)
	iCursorPos += strlen (pySelected[i].strHZ);

    if (iPYInsertPoint > strlen (strFindString))
	iPYInsertPoint = strlen (strFindString);
    iTemp = iPYInsertPoint;

    for (i = 0; i < findMap.iHZCount; i++) {
	if (strlen (findMap.strPYParsed[i]) >= iTemp) {
	    iCursorPos += iTemp;
	    break;
	}
	iCursorPos += strlen (findMap.strPYParsed[i]);

	iCursorPos++;
	iTemp -= strlen (findMap.strPYParsed[i]);
    }
}

/*
 * 由于拼音的编辑功能修改了strFindString，必须保证strCodeInput与用户的输入一致
 */
void UpdateCodeInputPY (void)
{
    int             i;

    strCodeInput[0] = '\0';
    for (i = 0; i < iPYSelected; i++)
	strcat (strCodeInput, pySelected[i].strPY);
    strcat (strCodeInput, strFindString);
    iCodeInputCount = strlen (strCodeInput);
}

void PYResetFlags (void)
{
    int             i, j, k;
    PyPhrase       *phrase;
    PyFreq         *freq;
    HZ             *hz;

    for (i = 0; i < iPYFACount; i++) {
	for (j = 0; j < PYFAList[i].iBase; j++) {
	    PYFAList[i].pyBase[j].flag = 0;
	    for (k = 0; k < PYFAList[i].pyBase[j].iPhrase; k++)
		PYFAList[i].pyBase[j].phrase[k].flag = 0;
	    phrase = PYFAList[i].pyBase[j].userPhrase->next;
	    for (k = 0; k < PYFAList[i].pyBase[j].iUserPhrase; k++) {
		phrase->flag = 0;
		phrase = phrase->next;
	    }
	}
    }

    freq = pyFreq->next;
    for (i = 0; i < iPYFreqCount; i++) {
	hz = freq->HZList->next;
	for (j = 0; j < freq->iCount; j++) {
	    hz->flag = False;
	    hz = hz->next;
	}
	freq = freq->next;
    }
}

void UpdateFindString (void)
{
    int             i;

    strFindString[0] = '\0';
    for (i = 0; i < findMap.iHZCount; i++) {
	if (i >= MAX_WORDS_USER_INPUT)
	    break;
	strcat (strFindString, findMap.strPYParsed[i]);
    }
    if (iPYInsertPoint > strlen (strFindString))
	iPYInsertPoint = strlen (strFindString);
}

INPUT_RETURN_VALUE PYGetCandWords (SEARCH_MODE mode)
{
    int             iVal;

    //if (findMap.iMode == PARSE_ERROR || (bSingleHZMode && findMap.iHZCount > 1)) {
    if (findMap.iMode == PARSE_ERROR) {
	uMessageDown = 0;
	iCandPageCount = 0;
	iCandWordCount = 0;

	return IRV_DISPLAY_MESSAGE;
    }

    if (bIsInLegend)
	return PYGetLegendCandWords (mode);

    if (mode == SM_FIRST) {
	iCurrentCandPage = 0;
	iCandPageCount = 0;
	iCandWordCount = 0;
	iYCDZ = 0;

	PYResetFlags ();

	//判断是不是要输入常用字或符号
	pCurFreq = pyFreq->next;
	for (iVal = 0; iVal < iPYFreqCount; iVal++) {
	    if (!strcmp (strFindString, pCurFreq->strPY))
		break;
	    pCurFreq = pCurFreq->next;
	}

	//if (!bSingleHZMode && bPYCreateAuto)
	if (bPYCreateAuto)
	    PYCreateAuto ();
    }
    else {
	if (!iCandPageCount)
	    return IRV_TO_PROCESS;

	if (mode == SM_NEXT) {
	    if (iCurrentCandPage == iCandPageCount)
		return IRV_DO_NOTHING;

	    iCurrentCandPage++;
	}
	else {
	    if (!iCurrentCandPage)
		return IRV_DO_NOTHING;

	    iCurrentCandPage--;
	    //需要将目前的候选词的标志重置
	    PYSetCandWordsFlag (False);
	}

	iCandWordCount = 0;
    }

    if (!(pCurFreq && pCurFreq->bIsSym)) {
	//if (!iCurrentCandPage && strPYAuto[0] && !bSingleHZMode) {
	if (!iCurrentCandPage && strPYAuto[0]) {
	    iCandWordCount = 1;
	    PYCandWords[0].iWhich = PY_CAND_AUTO;
	}
    }

    if (mode != SM_PREV) {
	PYGetCandWordsForward ();

	if (iCurrentCandPage == iCandPageCount) {
	    if (PYCheckNextCandPage ())
		iCandPageCount++;
	}
    }
    else
	PYGetCandWordsBackward ();

    PYCreateCandString ();

    return IRV_DISPLAY_CANDWORDS;
}

void PYCreateCandString (void)
{
    char            str[3];
    char           *pBase = NULL, *pPhrase;
    int             iType, iVal;

    if (bPointAfterNumber) {
	str[1] = '.';
	str[2] = '\0';
    }
    else
	str[1] = '\0';
    uMessageDown = 0;

    for (iVal = 0; iVal < iCandWordCount; iVal++) {
	if (iVal == 9)
	    str[0] = '0';
	else
	    str[0] = iVal + 1 + '0';
	strcpy (messageDown[uMessageDown].strMsg, str);
	messageDown[uMessageDown++].type = MSG_INDEX;

	iType = MSG_OTHER;
	if (PYCandWords[iVal].iWhich == PY_CAND_AUTO) {
	    strcpy (messageDown[uMessageDown].strMsg, strPYAuto);
	    iType = MSG_TIPS;
	}
	else {
	    pPhrase = NULL;
	    switch (PYCandWords[iVal].iWhich) {
	    case PY_CAND_BASE:	//是系统单字
		pBase = PYFAList[PYCandWords[iVal].cand.base.iPYFA].pyBase[PYCandWords[iVal].cand.base.iBase].strHZ;
		break;
	    case PY_CAND_USERPHRASE:	//是用户词组
		iType = MSG_USERPHR;
	    case PY_CAND_SYMPHRASE:	//是系统词组
		pBase = PYFAList[PYCandWords[iVal].cand.phrase.iPYFA].pyBase[PYCandWords[iVal].cand.phrase.iBase].strHZ;
		pPhrase = PYCandWords[iVal].cand.phrase.phrase->strPhrase;
		break;
	    case PY_CAND_FREQ:	//是常用字
		pBase = PYCandWords[iVal].cand.freq.hz->strHZ;
		iType = MSG_CODE;
		break;
	    case PY_CAND_SYMBOL:	//是特殊符号
		pBase = PYCandWords[iVal].cand.freq.hz->strHZ;
		break;
	    }
	    strcpy (messageDown[uMessageDown].strMsg, pBase);
	    if (pPhrase)
		strcat (messageDown[uMessageDown].strMsg, pPhrase);
	}

	if (iVal != (iCandWordCount - 1)) {
	    strcat (messageDown[uMessageDown].strMsg, " ");
	}
	if (PYCandWords[iVal].iWhich != PY_CAND_AUTO && iVal == iYCDZ)
	    iType = MSG_FIRSTCAND;

	messageDown[uMessageDown++].type = (MSG_TYPE) iType;
    }
}

void PYGetCandText (int iIndex, char *strText)
{
    char           *pBase = NULL, *pPhrase;

    if (PYCandWords[iIndex].iWhich == PY_CAND_AUTO)
	strcpy (strText, strPYAuto);
    else {
	pPhrase = NULL;
	
	switch (PYCandWords[iIndex].iWhich) {
	case PY_CAND_BASE:	//是系统单字
	    pBase = PYFAList[PYCandWords[iIndex].cand.base.iPYFA].pyBase[PYCandWords[iIndex].cand.base.iBase].strHZ;
	    break;
	case PY_CAND_USERPHRASE:	//是用户词组
	case PY_CAND_SYMPHRASE:	//是系统词组
	    pBase = PYFAList[PYCandWords[iIndex].cand.phrase.iPYFA].pyBase[PYCandWords[iIndex].cand.phrase.iBase].strHZ;
	    pPhrase = PYCandWords[iIndex].cand.phrase.phrase->strPhrase;
	    break;
	case PY_CAND_FREQ:	//是常用字
	    pBase = PYCandWords[iIndex].cand.freq.hz->strHZ;
	    break;
	case PY_CAND_SYMBOL:	//是特殊符号
	    pBase = PYCandWords[iIndex].cand.freq.hz->strHZ;
	    break;
	}
	
	strcpy (strText, pBase);
	if (pPhrase)
	    strcat (strText, pPhrase);
    }
}

void PYSetCandWordsFlag (Bool flag)
{
    int             i;

    for (i = 0; i < iCandWordCount; i++)
	PYSetCandWordFlag (i, flag);
}

void PYSetCandWordFlag (int iIndex, Bool flag)
{
    switch (PYCandWords[iIndex].iWhich) {
    case PY_CAND_BASE:
	PYFAList[PYCandWords[iIndex].cand.base.iPYFA].pyBase[PYCandWords[iIndex].cand.base.iBase].flag = flag;
	break;
    case PY_CAND_SYMPHRASE:
    case PY_CAND_USERPHRASE:
	PYCandWords[iIndex].cand.phrase.phrase->flag = flag;
	break;
    case PY_CAND_FREQ:
	PYCandWords[iIndex].cand.freq.hz->flag = flag;
    case PY_CAND_SYMBOL:
	PYCandWords[iIndex].cand.sym.hz->flag = flag;
	break;
    }
}

/*
 * 根据用户的录入自动生成一个汉字组合
 * 此处采用的策略是按照使用频率最高的字/词
 */
void PYCreateAuto (void)
{
    PYCandIndex     candPos;
    int             val;
    int             iMatchedLength;
    char            str[3];
    PyPhrase       *phrase;
    PyPhrase       *phraseSelected = NULL;
    PyBase         *baseSelected = NULL;
    PYFA           *pPYFA = NULL;
    char            strMap[MAX_WORDS_USER_INPUT * 2 + 1];
    int             iCount;

    strPYAuto[0] = '\0';
    strPYAutoMap[0] = '\0';
    str[2] = '\0';

    if (findMap.iHZCount == 1)
	return;

    while (strlen (strPYAuto) != findMap.iHZCount * 2) {
	phraseSelected = NULL;
	baseSelected = NULL;

	iCount = strlen (strPYAuto) / 2;
	str[0] = findMap.strMap[iCount][0];
	str[1] = findMap.strMap[iCount][1];

	strMap[0] = '\0';

	for (val = iCount + 1; val < findMap.iHZCount; val++)
	    strcat (strMap, findMap.strMap[val]);

	candPos.iPYFA = 0;
	candPos.iBase = 0;
	candPos.iPhrase = 0;
	if ((findMap.iHZCount - iCount) > 1) {
	    for (candPos.iPYFA = 0; candPos.iPYFA < iPYFACount; candPos.iPYFA++) {
		if (!Cmp2Map (PYFAList[candPos.iPYFA].strMap, str)) {
		    for (candPos.iBase = 0; candPos.iBase < PYFAList[candPos.iPYFA].iBase; candPos.iBase++) {
			phrase = PYFAList[candPos.iPYFA].pyBase[candPos.iBase].userPhrase->next;
			for (candPos.iPhrase = 0; candPos.iPhrase < PYFAList[candPos.iPYFA].pyBase[candPos.iBase].iUserPhrase; candPos.iPhrase++) {
			    val = CmpMap (phrase->strMap, strMap, &iMatchedLength);
			    if (!val && iMatchedLength == (findMap.iHZCount - 1) * 2)
				return;
			    if (!val || (val && (strlen (phrase->strMap) == iMatchedLength))) {
				if (CheckHZCharset (phrase->strPhrase) && CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ)) {
				    if (!phraseSelected) {
					baseSelected = &(PYFAList[candPos.iPYFA].pyBase[candPos.iBase]);
					pPYFA = &PYFAList[candPos.iPYFA];
					phraseSelected = phrase;
				    }
				    else if (strlen (phrase->strMap) <= (findMap.iHZCount - 1) * 2) {
					if (strlen (phrase->strMap) == strlen (phraseSelected->strMap)) {
					    //先看词频，如果词频一样，再最近优先
					    if ((phrase->iHit > phraseSelected->iHit) || ((phrase->iHit == phraseSelected->iHit) && (phrase->iIndex > phraseSelected->iIndex))) {
						baseSelected = &(PYFAList[candPos.iPYFA].pyBase[candPos.iBase]);
						pPYFA = &PYFAList[candPos.iPYFA];
						phraseSelected = phrase;
					    }
					}
					else if (strlen (phrase->strMap) > strlen (phraseSelected->strMap)) {
					    baseSelected = &(PYFAList[candPos.iPYFA].pyBase[candPos.iBase]);
					    pPYFA = &PYFAList[candPos.iPYFA];
					    phraseSelected = phrase;
					}
				    }
				}
			    }
			    phrase = phrase->next;
			}
		    }
		}
	    }

	    for (candPos.iPYFA = 0; candPos.iPYFA < iPYFACount; candPos.iPYFA++) {
		if (!Cmp2Map (PYFAList[candPos.iPYFA].strMap, str)) {
		    for (candPos.iBase = 0; candPos.iBase < PYFAList[candPos.iPYFA].iBase; candPos.iBase++) {
			for (candPos.iPhrase = 0; candPos.iPhrase < PYFAList[candPos.iPYFA].pyBase[candPos.iBase].iPhrase; candPos.iPhrase++) {
			    if (CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strPhrase) && CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ)) {
				val = CmpMap (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strMap, strMap, &iMatchedLength);
				if (!val && iMatchedLength == (findMap.iHZCount - 1) * 2)
				    return;
				if (!val || (val && (strlen (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strMap) == iMatchedLength))) {
				    if (!phraseSelected) {
					baseSelected = &(PYFAList[candPos.iPYFA].pyBase[candPos.iBase]);
					pPYFA = &PYFAList[candPos.iPYFA];
					phraseSelected = &(PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase]);
				    }
				    else if (strlen (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strMap) <= (findMap.iHZCount - 1) * 2) {
					if (strlen (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strMap) == strlen (phraseSelected->strMap)) {
					    //先看词频，如果词频一样，再最近优先
					    if ((PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].iHit > phraseSelected->iHit) ||
						((PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].iHit == phraseSelected->iHit) &&
						 (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].iIndex > phraseSelected->iIndex))) {
						baseSelected = &(PYFAList[candPos.iPYFA].pyBase[candPos.iBase]);
						pPYFA = &PYFAList[candPos.iPYFA];
						phraseSelected = &(PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase]);
					    }
					}
					else if (strlen (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strMap) > strlen (phraseSelected->strMap)) {
					    baseSelected = &(PYFAList[candPos.iPYFA].pyBase[candPos.iBase]);
					    pPYFA = &PYFAList[candPos.iPYFA];
					    phraseSelected = &(PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase]);
					}
				    }
				}
			    }
			}
		    }
		}
	    }
	    if (baseSelected) {
		strcat (strPYAuto, baseSelected->strHZ);
		strcat (strPYAutoMap, pPYFA->strMap);
		strcat (strPYAuto, phraseSelected->strPhrase);
		strcat (strPYAutoMap, phraseSelected->strMap);
	    }
	}

	if (!baseSelected) {
	    val = -1;
	    baseSelected = NULL;
	    for (candPos.iPYFA = 0; candPos.iPYFA < iPYFACount; candPos.iPYFA++) {
		if (!Cmp2Map (PYFAList[candPos.iPYFA].strMap, str)) {
		    for (candPos.iBase = 0; candPos.iBase < PYFAList[candPos.iPYFA].iBase; candPos.iBase++) {
			if (CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ))
			    if ((int)
				(PYFAList[candPos.iPYFA].pyBase[candPos.iBase].iHit) > val) {
				val = PYFAList[candPos.iPYFA].pyBase[candPos.iBase].iHit;
				baseSelected = &(PYFAList[candPos.iPYFA].pyBase[candPos.iBase]);
				pPYFA = &PYFAList[candPos.iPYFA];
			    }
		    }
		}
	    }

	    if (baseSelected) {
		strcat (strPYAuto, baseSelected->strHZ);
		strcat (strPYAutoMap, pPYFA->strMap);
	    }
	    else {		//出错了
		strPYAuto[0] = '\0';
		return;
	    }
	}
    }
}

/*
 * ** 从后往前查找
 */
/*void PYCreateAuto (void)
{
    PYCandIndex     candPos;
    char            str[3];
    PyPhrase       *phrase;
    PyPhrase       *phraseSelected = NULL;
    PyBase         *baseSelected = NULL;
    PYFA           *pPYFA = NULL;
    char            strMap[MAX_WORDS_USER_INPUT * 2 + 1];
    char strAutoTemp[MAX_WORDS_USER_INPUT * 2 + 1];
    char strAutoMapTemp[MAX_WORDS_USER_INPUT * 2 + 1];
		    
		    
    int             iStart,iEnd;
    int             val;
    int             iMatchedLength;

    if (findMap.iHZCount == 1)
	return;
    
    strPYAuto[0] = '\0';
    strPYAutoMap[0] = '\0';
    str[2] = '\0';

    while (strlen (strPYAuto) != findMap.iHZCount * 2) {
	phraseSelected = NULL;
	baseSelected = NULL;	    
	iStart = 0;
	while (!baseSelected ) {
	    str[0] = findMap.strMap[iStart][0];
	    str[1] = findMap.strMap[iStart][1];
	    strMap[0] = '\0';
	    
	    iEnd = (findMap.iHZCount-strlen (strPYAuto)/2);
	    candPos.iPYFA = 0;
	    candPos.iBase = 0;
	    if ((iEnd - iStart) > 1) {
		for (val = iStart + 1; val < (findMap.iHZCount-strlen (strPYAuto)/2); val++)
		    strcat (strMap, findMap.strMap[val]);
		
		candPos.iPhrase = 0;
	    
		for (candPos.iPYFA = 0; candPos.iPYFA < iPYFACount; candPos.iPYFA++) {
		    if (!Cmp2Map (PYFAList[candPos.iPYFA].strMap, str)) {
			for (candPos.iBase = 0; candPos.iBase < PYFAList[candPos.iPYFA].iBase; candPos.iBase++) {
			    phrase = PYFAList[candPos.iPYFA].pyBase[candPos.iBase].userPhrase->next;
			    for (candPos.iPhrase = 0; candPos.iPhrase < PYFAList[candPos.iPYFA].pyBase[candPos.iBase].iUserPhrase; candPos.iPhrase++) {
				val = CmpMap (phrase->strMap, strMap, &iMatchedLength);
				if (!val) {
				    if ( iMatchedLength == (findMap.iHZCount - 1) * 2) //查找成功，表示词库中已经有这个拼音组合了，此处不处理这种情况
					return;
				    if ( !phraseSelected || phraseSelected->iHit<phrase->iHit) {
					baseSelected=&(PYFAList[candPos.iPYFA].pyBase[candPos.iBase]);
					phraseSelected=phrase;
					pPYFA=&(PYFAList[candPos.iPYFA]);
				    }
				}
				phrase=phrase->next;
			    }
			}
		    }
		}
		
		for (candPos.iPYFA = 0; candPos.iPYFA < iPYFACount; candPos.iPYFA++) {
		    if (!Cmp2Map (PYFAList[candPos.iPYFA].strMap, str)) {
			for (candPos.iBase = 0; candPos.iBase < PYFAList[candPos.iPYFA].iBase; candPos.iBase++) {
			    for (candPos.iPhrase = 0; candPos.iPhrase < PYFAList[candPos.iPYFA].pyBase[candPos.iBase].iPhrase; candPos.iPhrase++) {
				if (CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strPhrase) && CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ)) {
				    val = CmpMap (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strMap, strMap, &iMatchedLength);
				    if (!val) {
					if (iMatchedLength == (findMap.iHZCount - 1) * 2)  //查找成功，表示词库中已经有这个拼音组合了，此处不处理这种情况
					    return;
					if ( !phraseSelected || phraseSelected->iHit<PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].iHit) {
					    baseSelected=&(PYFAList[candPos.iPYFA].pyBase[candPos.iBase]);
					    phraseSelected=&(PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase]);
					    pPYFA=&(PYFAList[candPos.iPYFA]);
					}
				    }
				}
			    }
			}
		    }
		}
		
		if (baseSelected) {   //因为是逆序查找的，因此需要倒过来
		    strcpy (strAutoTemp, baseSelected->strHZ);
		    strcat (strAutoTemp, phraseSelected->strPhrase);		    
		    strcat (strAutoTemp, strPYAuto);
		    strcpy (strPYAuto, strAutoTemp);
		    
		    strcpy (strAutoMapTemp, pPYFA->strMap);
		    strcat (strAutoMapTemp, phraseSelected->strMap);
		    strcat (strAutoMapTemp, strPYAutoMap);
		    strcpy (strPYAutoMap, strAutoMapTemp);
		}
	    }
	    
	    if (!baseSelected) {
		if ( (iEnd-iStart)>1 )
		    iStart++;
		else {
		    val = -1;
		    for (candPos.iPYFA = 0; candPos.iPYFA < iPYFACount; candPos.iPYFA++) {
			if (!Cmp2Map (PYFAList[candPos.iPYFA].strMap, str)) {
			    for (candPos.iBase = 0; candPos.iBase < PYFAList[candPos.iPYFA].iBase; candPos.iBase++) {
				if (CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ)) {
				    if ((int)(PYFAList[candPos.iPYFA].pyBase[candPos.iBase].iHit) > val) {
					val = PYFAList[candPos.iPYFA].pyBase[candPos.iBase].iHit;
					baseSelected = &(PYFAList[candPos.iPYFA].pyBase[candPos.iBase]);
					pPYFA = &PYFAList[candPos.iPYFA];
				    }
				}
			    }
			}
		    }

		    if (baseSelected) {   //因为是逆序查找的，因此需要倒过来
			strcpy (strAutoTemp, baseSelected->strHZ);
			strcat (strAutoTemp, strPYAuto);
			strcpy (strPYAuto, strAutoTemp);

			strcpy (strAutoMapTemp, pPYFA->strMap);
			strcat (strAutoMapTemp, strPYAutoMap);
			strcpy (strPYAutoMap, strAutoMapTemp);
		    }
		    else {		//出错了
			strPYAuto[0] = '\0';
			return;
		    }
		}
	    }
	}
    }
}*/

char           *PYGetCandWord (int iIndex)
{
    char           *pBase = NULL, *pPhrase = NULL;
    char           *pBaseMap = NULL, *pPhraseMap = NULL;
    uint           *pIndex = NULL;
    Bool            bAddNewPhrase = True;
    int             i;
    char            strHZString[MAX_WORDS_USER_INPUT * 2 + 1];
    int             iLen;

    if (!iCandWordCount)
	return NULL;
    if (iIndex > (iCandWordCount - 1))
	iIndex = iCandWordCount - 1;
    switch (PYCandWords[iIndex].iWhich) {
    case PY_CAND_AUTO:
	pBase = strPYAuto;
	pBaseMap = strPYAutoMap;
	bAddNewPhrase = bPYSaveAutoAsPhrase;
	break;
    case PY_CAND_BASE:		//是系统单字
	pBase = PYFAList[PYCandWords[iIndex].cand.base.iPYFA].pyBase[PYCandWords[iIndex].cand.base.iBase].strHZ;
	pBaseMap = PYFAList[PYCandWords[iIndex].cand.base.iPYFA].strMap;
	pIndex = &(PYFAList[PYCandWords[iIndex].cand.base.iPYFA].pyBase[PYCandWords[iIndex].cand.base.iBase].iIndex);
	PYFAList[PYCandWords[iIndex].cand.base.iPYFA].pyBase[PYCandWords[iIndex].cand.base.iBase].iHit++;
	PYFAList[PYCandWords[iIndex].cand.base.iPYFA].iChangeCount++;
	iOrderCount++;
	break;
    case PY_CAND_SYMPHRASE:	//是系统词组    
    case PY_CAND_USERPHRASE:	//是用户词组
	pBase = PYFAList[PYCandWords[iIndex].cand.phrase.iPYFA].pyBase[PYCandWords[iIndex].cand.phrase.iBase].strHZ;
	pBaseMap = PYFAList[PYCandWords[iIndex].cand.phrase.iPYFA].strMap;
	pPhrase = PYCandWords[iIndex].cand.phrase.phrase->strPhrase;
	pPhraseMap = PYCandWords[iIndex].cand.phrase.phrase->strMap;
	pIndex = &(PYCandWords[iIndex].cand.phrase.phrase->iIndex);
	PYCandWords[iIndex].cand.phrase.phrase->iHit++;
	PYFAList[PYCandWords[iIndex].cand.phrase.iPYFA].pyBase[PYCandWords[iIndex].cand.phrase.iBase].iChangeCount++;
	iOrderCount++;
	break;
    case PY_CAND_FREQ:		//是常用字
	pBase = PYCandWords[iIndex].cand.freq.hz->strHZ;
	pBaseMap = PYFAList[PYCandWords[iIndex].cand.freq.hz->iPYFA].strMap;
	PYCandWords[iIndex].cand.freq.hz->iHit++;
	pIndex = &(PYCandWords[iIndex].cand.freq.hz->iIndex);
	iNewFreqCount++;
	break;
    case PY_CAND_SYMBOL:	//是特殊符号
	pBase = PYCandWords[iIndex].cand.freq.hz->strHZ;
	bAddNewPhrase = False;
	break;
    }

    if (pIndex && (*pIndex != iCounter))
	*pIndex = ++iCounter;
    if (iOrderCount == AUTOSAVE_ORDER_COUNT) {
	SavePYIndex ();
	iOrderCount = 0;
    }
    if (iNewFreqCount == AUTOSAVE_FREQ_COUNT) {
	SavePYFreq ();
	iNewFreqCount = 0;
    }

    strcpy (messageDown[uMessageDown].strMsg, pBase);
    if (pPhrase)
	strcat (messageDown[uMessageDown].strMsg, pPhrase);
    strcpy (strHZString, pBase);
    if (pPhrase)
	strcat (strHZString, pPhrase);
    iLen = strlen (strHZString) / 2;
    if (iLen == findMap.iHZCount || PYCandWords[iIndex].iWhich == PY_CAND_SYMBOL) {
	strPYAuto[0] = '\0';
	for (iLen = 0; iLen < iPYSelected; iLen++)
	    strcat (strPYAuto, pySelected[iLen].strHZ);
	strcat (strPYAuto, strHZString);
	ParsePY (strCodeInput, &findMap, PY_PARSE_INPUT_USER);
	strHZString[0] = '\0';
	for (i = 0; i < iPYSelected; i++)
	    strcat (strHZString, pySelected[i].strMap);
	if (pBaseMap)
	    strcat (strHZString, pBaseMap);
	if (pPhraseMap)
	    strcat (strHZString, pPhraseMap);
	//if (!bSingleHZMode && bAddNewPhrase && (strlen (strPYAuto) <= (MAX_PY_PHRASE_LENGTH * 2)))
	if (bAddNewPhrase && (strlen (strPYAuto) <= (MAX_PY_PHRASE_LENGTH * 2)))
	    PYAddUserPhrase (strPYAuto, strHZString);
	uMessageDown = 0;
	uMessageUp = 0;
	if (bUseLegend) {
	    strcpy (strPYLegendSource, strPYAuto);
	    strcpy (strPYLegendMap, strHZString);
	    PYGetLegendCandWords (SM_FIRST);
	    iPYInsertPoint = 0;
	    strFindString[0] = '\0';
	}

	return strPYAuto;
    }

    //此时进入自造词状态    
    pySelected[iPYSelected].strPY[0] = '\0';
    pySelected[iPYSelected].strMap[0] = '\0';
    for (i = 0; i < iLen; i++)
	strcat (pySelected[iPYSelected].strPY, findMap.strPYParsed[i]);
    if (pBaseMap)
	strcat (pySelected[iPYSelected].strMap, pBaseMap);
    if (pPhraseMap)
	strcat (pySelected[iPYSelected].strMap, pPhraseMap);
    strcpy (pySelected[iPYSelected].strHZ, strHZString);
    iPYSelected++;
    strFindString[0] = '\0';
    for (; i < findMap.iHZCount; i++)
	strcat (strFindString, findMap.strPYParsed[i]);
    DoPYInput (-1);
    iPYInsertPoint = strlen (strFindString);
    return NULL;
}

void PYGetCandWordsForward (void)
{
    if (pCurFreq && pCurFreq->bIsSym)
	PYGetSymCandWords (SM_NEXT);
    else {
	PYGetPhraseCandWords (SM_NEXT);
	if (pCurFreq)
	    PYGetFreqCandWords (SM_NEXT);
    }

    if (!(pCurFreq && pCurFreq->bIsSym))
	PYGetBaseCandWords (SM_NEXT);
}

void PYGetCandWordsBackward (void)
{
    //if (pCurFreq && pCurFreq->bIsSym && !bSingleHZMode)
    if (pCurFreq && pCurFreq->bIsSym)
	PYGetSymCandWords (SM_PREV);
    else {
	//if (!bSingleHZMode)
	PYGetFreqCandWords (SM_PREV);
	PYGetBaseCandWords (SM_PREV);
	//if (iCandWordCount == iMaxCandWord || bSingleHZMode)
	if (iCandWordCount == iMaxCandWord)
	    return;
	PYGetPhraseCandWords (SM_PREV);
    }
}

Bool PYCheckNextCandPage (void)
{
    PYCandIndex     candPos;
    int             val;
    int             iMatchedLength;
    char            str[3];
    PyPhrase       *phrase;
    char            strMap[MAX_WORDS_USER_INPUT * 2 + 1];
    HZ             *hz;

    str[0] = findMap.strMap[0][0];
    str[1] = findMap.strMap[0][1];
    str[2] = '\0';
    strMap[0] = '\0';
    if (pCurFreq && pCurFreq->bIsSym) {
	hz = pCurFreq->HZList->next;
	for (val = 0; val < pCurFreq->iCount; val++) {
	    if (!hz->flag)
		return True;
	    hz = hz->next;
	}
    }
    else {
	if (findMap.iHZCount > 1) {
	    for (val = 1; val < findMap.iHZCount; val++)
		strcat (strMap, findMap.strMap[val]);
	    for (candPos.iPYFA = 0; candPos.iPYFA < iPYFACount; candPos.iPYFA++) {
		if (!Cmp2Map (PYFAList[candPos.iPYFA].strMap, str)) {
		    for (candPos.iBase = 0; candPos.iBase < PYFAList[candPos.iPYFA].iBase; candPos.iBase++) {
			phrase = PYFAList[candPos.iPYFA].pyBase[candPos.iBase].userPhrase->next;
			for (candPos.iPhrase = 0; candPos.iPhrase < PYFAList[candPos.iPYFA].pyBase[candPos.iBase].iUserPhrase; candPos.iPhrase++) {
			    val = CmpMap (phrase->strMap, strMap, &iMatchedLength);
			    if (!val || (val && (strlen (phrase->strMap) == iMatchedLength))) {
				if (CheckHZCharset (phrase->strPhrase) && CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ)) {
				    if (!phrase->flag)
					return True;
				}
			    }
			    phrase = phrase->next;
			}
		    }
		}
	    }

	    for (candPos.iPYFA = 0; candPos.iPYFA < iPYFACount; candPos.iPYFA++) {
		if (!Cmp2Map (PYFAList[candPos.iPYFA].strMap, str)) {
		    for (candPos.iBase = 0; candPos.iBase < PYFAList[candPos.iPYFA].iBase; candPos.iBase++) {
			for (candPos.iPhrase = 0; candPos.iPhrase < PYFAList[candPos.iPYFA].pyBase[candPos.iBase].iPhrase; candPos.iPhrase++) {
			    if (!PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].flag) {
				val = CmpMap (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strMap, strMap, &iMatchedLength);
				if (!val || (val && (strlen (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strMap) == iMatchedLength))) {
				    if (CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strPhrase) && CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ))
					return True;
				}
			    }
			}
		    }
		}
	    }
	}

	if (pCurFreq) {
	    hz = pCurFreq->HZList->next;
	    for (val = 0; val < pCurFreq->iCount; val++) {
		if (!hz->flag)
		    return True;
		hz = hz->next;
	    }
	}

	for (candPos.iPYFA = 0; candPos.iPYFA < iPYFACount; candPos.iPYFA++) {
	    if (!Cmp2Map (PYFAList[candPos.iPYFA].strMap, str)) {
		for (candPos.iBase = 0; candPos.iBase < PYFAList[candPos.iPYFA].iBase; candPos.iBase++) {
		    if (!PYFAList[candPos.iPYFA].pyBase[candPos.iBase].flag) {
			if (CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ) && !PYIsInFreq (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ))
			    return True;
		    }
		}
	    }
	}
    }

    return False;
}

void PYGetPhraseCandWords (SEARCH_MODE mode)
{
    PYCandIndex     candPos;
    char            str[3];
    int             val, iMatchedLength;
    char            strMap[MAX_WORDS_USER_INPUT * 2 + 1];
    PyPhrase       *phrase;

    if (findMap.iHZCount == 1)
	return;
    str[0] = findMap.strMap[0][0];
    str[1] = findMap.strMap[0][1];
    str[2] = '\0';
    strMap[0] = '\0';
    for (val = 1; val < findMap.iHZCount; val++)
	strcat (strMap, findMap.strMap[val]);
    for (candPos.iPYFA = 0; candPos.iPYFA < iPYFACount; candPos.iPYFA++) {
	if (!Cmp2Map (PYFAList[candPos.iPYFA].strMap, str)) {
	    for (candPos.iBase = 0; candPos.iBase < PYFAList[candPos.iPYFA].iBase; candPos.iBase++) {
		if (CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ)) {
		    phrase = PYFAList[candPos.iPYFA].pyBase[candPos.iBase].userPhrase->next;
		    for (candPos.iPhrase = 0; candPos.iPhrase < PYFAList[candPos.iPYFA].pyBase[candPos.iBase].iUserPhrase; candPos.iPhrase++) {
			if (CheckHZCharset (phrase->strPhrase)) {
			    val = CmpMap (phrase->strMap, strMap, &iMatchedLength);
			    if (!val || (val && (strlen (phrase->strMap) == iMatchedLength))) {
				if ((mode != SM_PREV && !phrase->flag) || (mode == SM_PREV && phrase->flag)) {
				    if (!PYAddPhraseCandWord (candPos, phrase, mode, False))
					goto _end;
				}
			    }
			}

			phrase = phrase->next;
		    }
		}
	    }
	}
    }

    for (candPos.iPYFA = 0; candPos.iPYFA < iPYFACount; candPos.iPYFA++) {
	if (!Cmp2Map (PYFAList[candPos.iPYFA].strMap, str)) {
	    for (candPos.iBase = 0; candPos.iBase < PYFAList[candPos.iPYFA].iBase; candPos.iBase++) {
		if (CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ)) {
		    for (candPos.iPhrase = 0; candPos.iPhrase < PYFAList[candPos.iPYFA].pyBase[candPos.iBase].iPhrase; candPos.iPhrase++) {
			if (CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strPhrase)) {
			    val = CmpMap (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strMap, strMap, &iMatchedLength);
			    if (!val || (val && (strlen (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].strMap) == iMatchedLength))) {
				if ((mode != SM_PREV && !PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].flag)
				    || (mode == SM_PREV && PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase].flag)) {
				    if (!PYAddPhraseCandWord (candPos, &(PYFAList[candPos.iPYFA].pyBase[candPos.iBase].phrase[candPos.iPhrase]), mode, True))
					goto _end;
				}
			    }
			}
		    }
		}
	    }
	}
    }

  _end:
    PYSetCandWordsFlag (True);
}

/*
 * 将一个词加入到候选列表的合适位置中
 * b = True 表示是系统词组，False表示是用户词组
 */
Bool PYAddPhraseCandWord (PYCandIndex pos, PyPhrase * phrase, SEARCH_MODE mode, Bool b)
{
    char            str[MAX_WORDS_USER_INPUT * 2 + 1];
    int             i = 0, j, iStart = 0;

    strcpy (str, PYFAList[pos.iPYFA].pyBase[pos.iBase].strHZ);
    strcat (str, phrase->strPhrase);
    if (strPYAuto[0]) {
	if (!strcmp (strPYAuto, str)) {
	    phrase->flag = 1;
	    return True;
	}
    }

    switch (phraseOrder) {
    case AD_NO:
	if (mode == SM_PREV) {
	    for (i = (iCandWordCount - 1); i >= 0; i--) {
		if (PYCandWords[i].iWhich == PY_CAND_AUTO) {
		    iStart = i + 1;
		    i++;
		    break;
		}
		else if (PYCandWords[i].iWhich == PY_CAND_USERPHRASE || PYCandWords[i].iWhich == PY_CAND_SYMPHRASE) {
		    if (strlen (PYCandWords[i].cand.phrase.phrase->strPhrase) >= strlen (phrase->strPhrase)) {
			i++;
			break;
		    }
		}
	    }

	    if (i < 0) {
		if (iCandWordCount == iMaxCandWord)
		    return False;
		else
		    i = 0;
	    }
	    else if (iCandWordCount == iMaxCandWord)
		i--;
	}
	else {
	    for (i = 0; i < iCandWordCount; i++) {
		if (PYCandWords[i].iWhich == PY_CAND_USERPHRASE || PYCandWords[i].iWhich == PY_CAND_SYMPHRASE)
		    if (strlen (PYCandWords[i].cand.phrase.phrase->strPhrase) < strlen (phrase->strPhrase))
			break;
	    }
	    if (i > iMaxCandWord)
		return False;
	}
	break;
	//下面两部分可以放在一起××××××××××××××××××××××××××××××××××××××××××
    case AD_FAST:
	if (mode == SM_PREV) {
	    for (i = (iCandWordCount - 1); i >= 0; i--) {
		if (PYCandWords[i].iWhich == PY_CAND_AUTO) {
		    iStart = ++i;
		    break;
		}
		else if (PYCandWords[i].iWhich == PY_CAND_USERPHRASE || PYCandWords[i].iWhich == PY_CAND_SYMPHRASE) {
		    if (strlen (phrase->strPhrase) < strlen (PYCandWords[i].cand.phrase.phrase->strPhrase)) {
			i++;
			break;
		    }
		    else if (strlen (PYCandWords[i].cand.phrase.phrase->strPhrase) == strlen (phrase->strPhrase)) {
			if (phrase->iIndex < PYCandWords[i].cand.phrase.phrase->iIndex) {
			    i++;
			    break;
			}
			if (phrase->iIndex == PYCandWords[i].cand.phrase.phrase->iIndex) {
			    if (phrase->iHit <= PYCandWords[i].cand.phrase.phrase->iHit) {
				i++;
				break;
			    }
			}
		    }
		}
	    }

	    if (i < 0) {
		if (iCandWordCount == iMaxCandWord)
		    return True;
		i = 0;
	    }
	    else if (iCandWordCount == iMaxCandWord)
		i--;
	}
	else {
	    for (i = 0; i < iCandWordCount; i++) {
		if (PYCandWords[i].iWhich == PY_CAND_USERPHRASE || PYCandWords[i].iWhich == PY_CAND_SYMPHRASE) {
		    if (strlen (PYCandWords[i].cand.phrase.phrase->strPhrase) < strlen (phrase->strPhrase))
			break;
		    else if (strlen (PYCandWords[i].cand.phrase.phrase->strPhrase) == strlen (phrase->strPhrase)) {
			if (phrase->iIndex > PYCandWords[i].cand.phrase.phrase->iIndex)
			    break;
			if (phrase->iIndex == PYCandWords[i].cand.phrase.phrase->iIndex) {
			    if (phrase->iHit > PYCandWords[i].cand.phrase.phrase->iHit)
				break;
			}
		    }
		}
	    }

	    if (i == iMaxCandWord)
		return True;
	}
	break;
    case AD_FREQ:
	if (mode == SM_PREV) {
	    for (i = (iCandWordCount - 1); i >= 0; i--) {
		if (PYCandWords[i].iWhich == PY_CAND_AUTO) {
		    iStart = ++i;
		    break;
		}
		else if (PYCandWords[i].iWhich == PY_CAND_USERPHRASE || PYCandWords[i].iWhich == PY_CAND_SYMPHRASE) {
		    if (strlen (PYCandWords[i].cand.phrase.phrase->strPhrase) < strlen (phrase->strPhrase)) {
			i++;
			break;
		    }
		    else if (strlen (PYCandWords[i].cand.phrase.phrase->strPhrase) == strlen (phrase->strPhrase)) {
			if (phrase->iHit < PYCandWords[i].cand.phrase.phrase->iHit) {
			    i++;
			    break;
			}
			if (phrase->iHit == PYCandWords[i].cand.phrase.phrase->iHit) {
			    if (phrase->iIndex <= PYCandWords[i].cand.phrase.phrase->iIndex) {
				i++;
				break;
			    }
			}
		    }
		}
	    }

	    if (i < 0) {
		if (iCandWordCount == iMaxCandWord)
		    return True;
		i = 0;
	    }
	    else if (iCandWordCount == iMaxCandWord)
		i--;
	}
	else {
	    for (i = 0; i < iCandWordCount; i++) {
		if (PYCandWords[i].iWhich == PY_CAND_USERPHRASE || PYCandWords[i].iWhich == PY_CAND_SYMPHRASE) {
		    if (strlen (PYCandWords[i].cand.phrase.phrase->strPhrase) < strlen (phrase->strPhrase))
			break;
		    else if (strlen (PYCandWords[i].cand.phrase.phrase->strPhrase) == strlen (phrase->strPhrase)) {
			if (phrase->iHit > PYCandWords[i].cand.phrase.phrase->iHit)
			    break;
			if (phrase->iHit == PYCandWords[i].cand.phrase.phrase->iHit) {
			    if (phrase->iIndex > PYCandWords[i].cand.phrase.phrase->iIndex)
				break;
			}
		    }
		}
	    }
	    if (i == iMaxCandWord)
		return True;
	}
	break;
    }
    //×××××××××××××××××××××××××××××××××××××××××××××××××××××

    if (mode == SM_PREV) {
	if (iCandWordCount == iMaxCandWord) {
	    for (j = iStart; j < i; j++) {
		PYCandWords[j].iWhich = PYCandWords[j + 1].iWhich;
		switch (PYCandWords[j].iWhich) {
		case PY_CAND_BASE:
		    PYCandWords[j].cand.base.iPYFA = PYCandWords[j + 1].cand.base.iPYFA;
		    PYCandWords[j].cand.base.iBase = PYCandWords[j + 1].cand.base.iBase;
		    break;
		case PY_CAND_SYMPHRASE:
		case PY_CAND_USERPHRASE:
		    PYCandWords[j].cand.phrase.phrase = PYCandWords[j + 1].cand.phrase.phrase;
		    PYCandWords[j].cand.phrase.iPYFA = PYCandWords[j + 1].cand.phrase.iPYFA;
		    PYCandWords[j].cand.phrase.iBase = PYCandWords[j + 1].cand.phrase.iBase;
		    break;
		case PY_CAND_FREQ:
		    PYCandWords[j].cand.freq.hz = PYCandWords[j + 1].cand.freq.hz;
		    PYCandWords[j].cand.freq.strPY = PYCandWords[j + 1].cand.freq.strPY;
		    break;
		}
	    }
	}
	else {
	    //插在i的前面
	    for (j = iCandWordCount; j > i; j--) {
		PYCandWords[j].iWhich = PYCandWords[j - 1].iWhich;
		switch (PYCandWords[j].iWhich) {
		case PY_CAND_BASE:
		    PYCandWords[j].cand.base.iPYFA = PYCandWords[j - 1].cand.base.iPYFA;
		    PYCandWords[j].cand.base.iBase = PYCandWords[j - 1].cand.base.iBase;
		    break;
		case PY_CAND_SYMPHRASE:
		case PY_CAND_USERPHRASE:
		    PYCandWords[j].cand.phrase.phrase = PYCandWords[j - 1].cand.phrase.phrase;
		    PYCandWords[j].cand.phrase.iPYFA = PYCandWords[j - 1].cand.phrase.iPYFA;
		    PYCandWords[j].cand.phrase.iBase = PYCandWords[j - 1].cand.phrase.iBase;
		    break;
		case PY_CAND_FREQ:
		    PYCandWords[j].cand.freq.hz = PYCandWords[j - 1].cand.freq.hz;
		    PYCandWords[j].cand.freq.strPY = PYCandWords[j - 1].cand.freq.strPY;
		    break;
		}
	    }
	}
    }
    else {
	j = iCandWordCount;
	if (iCandWordCount == iMaxCandWord)
	    j--;
	for (; j > i; j--) {
	    PYCandWords[j].iWhich = PYCandWords[j - 1].iWhich;
	    switch (PYCandWords[j].iWhich) {
	    case PY_CAND_BASE:
		PYCandWords[j].cand.base.iPYFA = PYCandWords[j - 1].cand.base.iPYFA;
		PYCandWords[j].cand.base.iBase = PYCandWords[j - 1].cand.base.iBase;
		break;
	    case PY_CAND_SYMPHRASE:
	    case PY_CAND_USERPHRASE:
		PYCandWords[j].cand.phrase.phrase = PYCandWords[j - 1].cand.phrase.phrase;
		PYCandWords[j].cand.phrase.iPYFA = PYCandWords[j - 1].cand.phrase.iPYFA;
		PYCandWords[j].cand.phrase.iBase = PYCandWords[j - 1].cand.phrase.iBase;
		break;
	    case PY_CAND_FREQ:
		PYCandWords[j].cand.freq.hz = PYCandWords[j - 1].cand.freq.hz;
		PYCandWords[j].cand.freq.strPY = PYCandWords[j - 1].cand.freq.strPY;
		break;
	    }
	}
    }

    PYCandWords[i].iWhich = (b) ? PY_CAND_SYMPHRASE : PY_CAND_USERPHRASE;
    PYCandWords[i].cand.phrase.phrase = phrase;
    PYCandWords[i].cand.phrase.iPYFA = pos.iPYFA;
    PYCandWords[i].cand.phrase.iBase = pos.iBase;
    if (iCandWordCount != iMaxCandWord)
	iCandWordCount++;
    return True;
}

//********************************************
void PYGetSymCandWords (SEARCH_MODE mode)
{
    int             i;
    HZ             *hz;

    if (pCurFreq && pCurFreq->bIsSym) {
	hz = pCurFreq->HZList->next;
	for (i = 0; i < pCurFreq->iCount; i++) {
	    if ((mode != SM_PREV && !hz->flag) || (mode == SM_PREV && hz->flag)) {
		if (!PYAddSymCandWord (hz, mode))
		    break;
	    }
	    hz = hz->next;
	}
    }

    PYSetCandWordsFlag (True);
}

/*
 * 将一个符号加入到候选列表的合适位置中
 * 符号不需进行频率调整
 */
Bool PYAddSymCandWord (HZ * hz, SEARCH_MODE mode)
{
    int             i, j;

    if (mode == SM_PREV) {
	if (iCandWordCount == iMaxCandWord) {
	    i = iCandWordCount - 1;
	    for (j = 0; j < i; j++)
		PYCandWords[j].cand.sym.hz = PYCandWords[j + 1].cand.sym.hz;
	}
	else
	    i = iCandWordCount;
    }
    else {
	if (iCandWordCount == iMaxCandWord)
	    return False;
	i = iCandWordCount;
	for (j = iCandWordCount - 1; j > i; j--)
	    PYCandWords[j].cand.sym.hz = PYCandWords[j - 1].cand.sym.hz;
    }

    PYCandWords[i].iWhich = PY_CAND_SYMBOL;
    PYCandWords[i].cand.sym.hz = hz;
    if (iCandWordCount != iMaxCandWord)
	iCandWordCount++;
    return True;
}

//*****************************************************

void PYGetBaseCandWords (SEARCH_MODE mode)
{
    PYCandIndex     candPos = {
	0, 0, 0
    };
    char            str[3];

    str[0] = findMap.strMap[0][0];
    str[1] = findMap.strMap[0][1];
    str[2] = '\0';
    for (candPos.iPYFA = 0; candPos.iPYFA < iPYFACount; candPos.iPYFA++) {
	if (!Cmp2Map (PYFAList[candPos.iPYFA].strMap, str)) {
	    for (candPos.iBase = 0; candPos.iBase < PYFAList[candPos.iPYFA].iBase; candPos.iBase++) {
		if (CheckHZCharset (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ)) {
		    if ((mode != SM_PREV && !PYFAList[candPos.iPYFA].pyBase[candPos.iBase].flag) || (mode == SM_PREV && PYFAList[candPos.iPYFA].pyBase[candPos.iBase].flag)) {
			if (!PYIsInFreq (PYFAList[candPos.iPYFA].pyBase[candPos.iBase].strHZ)) {
			    if (!PYAddBaseCandWord (candPos, mode))
				goto _end;
			}
		    }
		}
	    }
	}
    }

  _end:
    PYSetCandWordsFlag (True);
}

/*
 * 将一个字加入到候选列表的合适位置中
 */
Bool PYAddBaseCandWord (PYCandIndex pos, SEARCH_MODE mode)
{
    int             i = 0, j;
    int             iStart = 0;

    switch (baseOrder) {
    case AD_NO:
	if (mode == SM_PREV) {
	    if (iCandWordCount == iMaxCandWord)
		i = iCandWordCount - 1;
	    else
		i = iCandWordCount;
	}
	else {
	    if (iCandWordCount == iMaxCandWord)
		return False;
	    i = iCandWordCount;
	}
	break;
    case AD_FAST:
	if (mode == SM_PREV) {
	    for (i = (iCandWordCount - 1); i >= 0; i--) {
		if (PYCandWords[i].iWhich == PY_CAND_AUTO || PYCandWords[i].iWhich == PY_CAND_FREQ) {
		    iStart = i + 1;
		    i++;
		    break;
		}
		else if (PYCandWords[i].iWhich == PY_CAND_BASE) {
		    if (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].iIndex > PYFAList[pos.iPYFA].pyBase[pos.iBase].iIndex) {
			i++;
			break;
		    }
		    else if (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].iIndex == PYFAList[pos.iPYFA].pyBase[pos.iBase].iIndex) {
			if (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].iHit >= PYFAList[pos.iPYFA].pyBase[pos.iBase].iHit) {
			    i++;
			    break;
			}
		    }
		}
	    }

	    if (i < 0) {
		if (iCandWordCount == iMaxCandWord)
		    return True;
		i = 0;
	    }
	    else if (iCandWordCount == iMaxCandWord)
		i--;
	}
	else {
	    for (i = 0; i < iCandWordCount; i++) {
		if (PYCandWords[i].iWhich == PY_CAND_BASE) {
		    if (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].iIndex < PYFAList[pos.iPYFA].pyBase[pos.iBase].iIndex)
			break;
		    else if (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].iIndex == PYFAList[pos.iPYFA].pyBase[pos.iBase].iIndex) {
			if (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].iHit < PYFAList[pos.iPYFA].pyBase[pos.iBase].iHit)
			    break;
		    }
		}
	    }

	    if (i == iMaxCandWord)
		return True;
	}

	break;
    case AD_FREQ:
	if (mode == SM_PREV) {
	    for (i = (iCandWordCount - 1); i >= 0; i--) {
		if (PYCandWords[i].iWhich == PY_CAND_AUTO || PYCandWords[i].iWhich == PY_CAND_FREQ) {
		    iStart = i + 1;
		    i++;
		    break;
		}
		else if (PYCandWords[i].iWhich == PY_CAND_BASE) {
		    if (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].iHit > PYFAList[pos.iPYFA].pyBase[pos.iBase].iHit) {
			i++;
			break;
		    }
		    else if (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].iHit == PYFAList[pos.iPYFA].pyBase[pos.iBase].iHit) {
			if (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].iIndex >= PYFAList[pos.iPYFA].pyBase[pos.iBase].iIndex) {
			    i++;
			    break;
			}
		    }
		}
	    }

	    if (i < 0) {
		if (iCandWordCount == iMaxCandWord)
		    return True;
		i = 0;
	    }
	    else if (iCandWordCount == iMaxCandWord)
		i--;
	}
	else {
	    for (i = 0; i < iCandWordCount; i++) {
		if (PYCandWords[i].iWhich == PY_CAND_BASE) {
		    if (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].iHit < PYFAList[pos.iPYFA].pyBase[pos.iBase].iHit)
			break;
		    else if (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].iHit == PYFAList[pos.iPYFA].pyBase[pos.iBase].iHit) {
			if (PYFAList[PYCandWords[i].cand.base.iPYFA].pyBase[PYCandWords[i].cand.base.iBase].iIndex < PYFAList[pos.iPYFA].pyBase[pos.iBase].iIndex)
			    break;
		    }
		}
	    }
	    if (i == iMaxCandWord)
		return True;
	}
	break;
    }

    if (mode == SM_PREV) {
	if (iCandWordCount == iMaxCandWord) {
	    for (j = iStart; j < i; j++) {
		PYCandWords[j].iWhich = PYCandWords[j + 1].iWhich;
		switch (PYCandWords[j].iWhich) {
		case PY_CAND_BASE:
		    PYCandWords[j].cand.base.iPYFA = PYCandWords[j + 1].cand.base.iPYFA;
		    PYCandWords[j].cand.base.iBase = PYCandWords[j + 1].cand.base.iBase;
		    break;
		case PY_CAND_SYMPHRASE:
		case PY_CAND_USERPHRASE:
		    PYCandWords[j].cand.phrase.phrase = PYCandWords[j + 1].cand.phrase.phrase;
		    PYCandWords[j].cand.phrase.iPYFA = PYCandWords[j + 1].cand.phrase.iPYFA;
		    PYCandWords[j].cand.phrase.iBase = PYCandWords[j + 1].cand.phrase.iBase;
		    break;
		case PY_CAND_FREQ:
		    PYCandWords[j].cand.freq.hz = PYCandWords[j + 1].cand.freq.hz;
		    PYCandWords[j].cand.freq.strPY = PYCandWords[j + 1].cand.freq.strPY;
		    break;
		}
	    }
	}
	else {
	    for (j = iCandWordCount; j > i; j--) {
		PYCandWords[j].iWhich = PYCandWords[j - 1].iWhich;
		switch (PYCandWords[j].iWhich) {
		case PY_CAND_BASE:
		    PYCandWords[j].cand.base.iPYFA = PYCandWords[j - 1].cand.base.iPYFA;
		    PYCandWords[j].cand.base.iBase = PYCandWords[j - 1].cand.base.iBase;
		    break;
		case PY_CAND_SYMPHRASE:
		case PY_CAND_USERPHRASE:
		    PYCandWords[j].cand.phrase.phrase = PYCandWords[j - 1].cand.phrase.phrase;
		    PYCandWords[j].cand.phrase.iPYFA = PYCandWords[j - 1].cand.phrase.iPYFA;
		    PYCandWords[j].cand.phrase.iBase = PYCandWords[j - 1].cand.phrase.iBase;
		    break;
		case PY_CAND_FREQ:
		    PYCandWords[j].cand.freq.hz = PYCandWords[j - 1].cand.freq.hz;
		    PYCandWords[j].cand.freq.strPY = PYCandWords[j - 1].cand.freq.strPY;
		    break;
		}
	    }
	}
    }
    else {
	j = iCandWordCount;
	if (iCandWordCount == iMaxCandWord)
	    j--;
	for (; j > i; j--) {
	    PYCandWords[j].iWhich = PYCandWords[j - 1].iWhich;
	    switch (PYCandWords[j].iWhich) {
	    case PY_CAND_BASE:
		PYCandWords[j].cand.base.iPYFA = PYCandWords[j - 1].cand.base.iPYFA;
		PYCandWords[j].cand.base.iBase = PYCandWords[j - 1].cand.base.iBase;
		break;
	    case PY_CAND_SYMPHRASE:
	    case PY_CAND_USERPHRASE:
		PYCandWords[j].cand.phrase.phrase = PYCandWords[j - 1].cand.phrase.phrase;
		PYCandWords[j].cand.phrase.iPYFA = PYCandWords[j - 1].cand.phrase.iPYFA;
		PYCandWords[j].cand.phrase.iBase = PYCandWords[j - 1].cand.phrase.iBase;
		break;
	    case PY_CAND_FREQ:
		PYCandWords[j].cand.freq.hz = PYCandWords[j - 1].cand.freq.hz;
		PYCandWords[j].cand.freq.strPY = PYCandWords[j - 1].cand.freq.strPY;
		break;
	    }
	}
    }

    PYCandWords[i].iWhich = PY_CAND_BASE;
    PYCandWords[i].cand.base.iPYFA = pos.iPYFA;
    PYCandWords[i].cand.base.iBase = pos.iBase;
    if (iCandWordCount != iMaxCandWord)
	iCandWordCount++;
    return True;
}

void PYGetFreqCandWords (SEARCH_MODE mode)
{
    int             i;
    HZ             *hz;

    if (pCurFreq && !pCurFreq->bIsSym) {
	hz = pCurFreq->HZList->next;
	for (i = 0; i < pCurFreq->iCount; i++) {
	    if ((mode != SM_PREV && !hz->flag) || (mode == SM_PREV && hz->flag)) {
		if (!PYAddFreqCandWord (hz, pCurFreq->strPY, mode))
		    break;
	    }
	    hz = hz->next;
	}
    }

    PYSetCandWordsFlag (True);
}

/*
 * 将一个常用字加入到候选列表的合适位置中
 */
Bool PYAddFreqCandWord (HZ * hz, char *strPY, SEARCH_MODE mode)
{
    int             i = 0, j;
    int             iStart = 0;

    switch (freqOrder) {
    case AD_NO:
	if (mode == SM_PREV) {
	    if (iCandWordCount == iMaxCandWord)
		i = iCandWordCount - 1;
	}
	else {
	    if (iCandWordCount == iMaxCandWord)
		return False;
	    i = iCandWordCount;
	}
	break;
    case AD_FAST:
	if (mode == SM_PREV) {
	    for (i = (iCandWordCount - 1); i >= 0; i--) {
		if (PYCandWords[i].iWhich == PY_CAND_AUTO) {
		    iStart = i + 1;
		    i++;
		    break;
		}
		else if (PYCandWords[i].iWhich == PY_CAND_FREQ && (hz->iIndex <= PYCandWords[i].cand.freq.hz->iIndex)) {
		    i++;
		    break;
		}
	    }

	    if (i < 0) {
		if (iCandWordCount == iMaxCandWord)
		    return True;
		i = 0;
	    }
	    else if (iCandWordCount == iMaxCandWord)
		i--;
	}
	else {
	    for (i = 0; i < iCandWordCount; i++) {
		if (PYCandWords[i].iWhich == PY_CAND_FREQ && (hz->iIndex > PYCandWords[i].cand.freq.hz->iIndex))
		    break;
	    }
	    if (i == iMaxCandWord)
		return True;
	}
	break;
    case AD_FREQ:
	if (mode == SM_PREV) {
	    for (i = (iCandWordCount - 1); i >= 0; i--) {
		if (PYCandWords[i].iWhich == PY_CAND_AUTO) {
		    iStart = i + 1;
		    i++;
		    break;
		}
		else if (PYCandWords[i].iWhich == PY_CAND_FREQ && (hz->iHit <= PYCandWords[i].cand.freq.hz->iHit)) {
		    i++;
		    break;
		}
	    }

	    if (i < 0) {
		if (iCandWordCount == iMaxCandWord)
		    return True;
		i = 0;
	    }
	    else if (iCandWordCount == iMaxCandWord)
		i--;
	}
	else {
	    for (i = 0; i < iCandWordCount; i++) {
		if (PYCandWords[i].iWhich == PY_CAND_FREQ && (hz->iHit > PYCandWords[i].cand.freq.hz->iHit))
		    break;
	    }
	    if (i == iMaxCandWord)
		return True;
	}
	break;
    }

    if (mode == SM_PREV) {
	if (iCandWordCount == iMaxCandWord) {
	    for (j = iStart; j < i; j++) {
		PYCandWords[j].iWhich = PYCandWords[j + 1].iWhich;
		switch (PYCandWords[j].iWhich) {
		case PY_CAND_BASE:
		    PYCandWords[j].cand.base.iPYFA = PYCandWords[j + 1].cand.base.iPYFA;
		    PYCandWords[j].cand.base.iBase = PYCandWords[j + 1].cand.base.iBase;
		    break;
		case PY_CAND_SYMPHRASE:
		case PY_CAND_USERPHRASE:
		    PYCandWords[j].cand.phrase.phrase = PYCandWords[j + 1].cand.phrase.phrase;
		    PYCandWords[j].cand.phrase.iPYFA = PYCandWords[j + 1].cand.phrase.iPYFA;
		    PYCandWords[j].cand.phrase.iBase = PYCandWords[j + 1].cand.phrase.iBase;
		    break;
		case PY_CAND_FREQ:
		    PYCandWords[j].cand.freq.hz = PYCandWords[j + 1].cand.freq.hz;
		    PYCandWords[j].cand.freq.strPY = PYCandWords[j + 1].cand.freq.strPY;
		    break;
		}
	    }
	}
	else {
	    for (j = iCandWordCount; j > i; j--) {
		PYCandWords[j].iWhich = PYCandWords[j - 1].iWhich;
		switch (PYCandWords[j].iWhich) {
		case PY_CAND_BASE:
		    PYCandWords[j].cand.base.iPYFA = PYCandWords[j - 1].cand.base.iPYFA;
		    PYCandWords[j].cand.base.iBase = PYCandWords[j - 1].cand.base.iBase;
		    break;
		case PY_CAND_SYMPHRASE:
		case PY_CAND_USERPHRASE:
		    PYCandWords[j].cand.phrase.phrase = PYCandWords[j - 1].cand.phrase.phrase;
		    PYCandWords[j].cand.phrase.iPYFA = PYCandWords[j - 1].cand.phrase.iPYFA;
		    PYCandWords[j].cand.phrase.iBase = PYCandWords[j - 1].cand.phrase.iBase;
		    break;
		case PY_CAND_FREQ:
		    PYCandWords[j].cand.freq.hz = PYCandWords[j - 1].cand.freq.hz;
		    PYCandWords[j].cand.freq.strPY = PYCandWords[j - 1].cand.freq.strPY;
		    break;
		}
	    }
	}
    }
    else {
	j = iCandWordCount;
	if (iCandWordCount == iMaxCandWord)
	    j--;
	for (; j > i; j--) {
	    PYCandWords[j].iWhich = PYCandWords[j - 1].iWhich;
	    switch (PYCandWords[j].iWhich) {
	    case PY_CAND_BASE:
		PYCandWords[j].cand.base.iPYFA = PYCandWords[j - 1].cand.base.iPYFA;
		PYCandWords[j].cand.base.iBase = PYCandWords[j - 1].cand.base.iBase;
		break;
	    case PY_CAND_SYMPHRASE:
	    case PY_CAND_USERPHRASE:
		PYCandWords[j].cand.phrase.phrase = PYCandWords[j - 1].cand.phrase.phrase;
		PYCandWords[j].cand.phrase.iPYFA = PYCandWords[j - 1].cand.phrase.iPYFA;
		PYCandWords[j].cand.phrase.iBase = PYCandWords[j - 1].cand.phrase.iBase;
		break;
	    case PY_CAND_FREQ:
		PYCandWords[j].cand.freq.hz = PYCandWords[j - 1].cand.freq.hz;
		PYCandWords[j].cand.freq.strPY = PYCandWords[j - 1].cand.freq.strPY;
		break;
	    }
	}
    }

    PYCandWords[i].iWhich = PY_CAND_FREQ;
    PYCandWords[i].cand.freq.hz = hz;
    PYCandWords[i].cand.freq.strPY = strPY;
    if (iCandWordCount != iMaxCandWord)
	iCandWordCount++;
    return True;
}

/*
 * 将一个词组保存到用户词组库中
 * 返回True表示是新词组
 */
Bool PYAddUserPhrase (char *phrase, char *map)
{
    PyPhrase       *userPhrase, *newPhrase, *temp;
    char            str[3];
    int             i, j, k, iTemp;

    //如果短于两个汉字，则不能组成词组
    if (strlen (phrase) < 4)
	return False;
    str[0] = map[0];
    str[1] = map[1];
    str[2] = '\0';
    i = GetBaseMapIndex (str);
    str[0] = phrase[0];
    str[1] = phrase[1];
    j = GetBaseIndex (i, str);;
    //判断该词组是否已经在库中
    //首先，看它是不是在用户词组库中
    userPhrase = PYFAList[i].pyBase[j].userPhrase->next;
    for (k = 0; k < PYFAList[i].pyBase[j].iUserPhrase; k++) {
	if (!strcmp (map + 2, userPhrase->strMap) && !strcmp (phrase + 2, userPhrase->strPhrase))
	    return False;
	userPhrase = userPhrase->next;
    }

    //然后，看它是不是在系统词组库中
    for (k = 0; k < PYFAList[i].pyBase[j].iPhrase; k++)
	if (!strcmp (map + 2, PYFAList[i].pyBase[j].phrase[k].strMap) && !strcmp (phrase + 2, PYFAList[i].pyBase[j].phrase[k].strPhrase))
	    return False;
    //下面将词组添加到列表中
    newPhrase = (PyPhrase *) malloc (sizeof (PyPhrase));
    newPhrase->strMap = (char *) malloc (sizeof (char) * strlen (map + 2) + 1);
    newPhrase->strPhrase = (char *) malloc (sizeof (char) * strlen (phrase + 2) + 1);
    strcpy (newPhrase->strMap, map + 2);
    strcpy (newPhrase->strPhrase, phrase + 2);
    newPhrase->iIndex = ++iCounter;
    newPhrase->iHit = 1;
    newPhrase->flag = 0;
    temp = PYFAList[i].pyBase[j].userPhrase;
    userPhrase = PYFAList[i].pyBase[j].userPhrase->next;
    for (k = 0; k < PYFAList[i].pyBase[j].iUserPhrase; k++) {
	if (CmpMap (map + 2, userPhrase->strMap, &iTemp) > 0)
	    break;
	temp = userPhrase;
	userPhrase = userPhrase->next;
    }

    newPhrase->next = temp->next;
    temp->next = newPhrase;
    PYFAList[i].pyBase[j].iUserPhrase++;
    iNewPYPhraseCount++;
    if (iNewPYPhraseCount == AUTOSAVE_PHRASE_COUNT) {
	SavePYUserPhrase ();
	iNewPYPhraseCount = 0;
    }

    return True;
}

void PYDelUserPhrase (int iPYFA, int iBase, PyPhrase * phrase)
{
    PyPhrase       *temp;

    //首先定位该词组
    temp = PYFAList[iPYFA].pyBase[iBase].userPhrase;
    while (temp) {
	if (temp->next == phrase)
	    break;
	temp = temp->next;
    }
    if (!temp)
	return;
    temp->next = phrase->next;
    free (phrase->strPhrase);
    free (phrase->strMap);
    free (phrase);
    PYFAList[iPYFA].pyBase[iBase].iUserPhrase--;
    iNewPYPhraseCount++;
    if (iNewPYPhraseCount == AUTOSAVE_PHRASE_COUNT) {
	SavePYUserPhrase ();
	iNewPYPhraseCount = 0;
    }
}

int GetBaseMapIndex (char *strMap)
{
    int             i;

    for (i = 0; i < iPYFACount; i++) {
	if (!strcmp (strMap, PYFAList[i].strMap))
	    return i;
    }
    return -1;
}

/*
 * 保存用户词库
 */
void SavePYUserPhrase (void)
{
    int             i, j, k;
    int             iTemp;
    char            strPath[PATH_MAX];
    char            strPathTemp[PATH_MAX];
    FILE           *fp;
    PyPhrase       *phrase;

    strcpy (strPathTemp, (char *) getenv ("HOME"));
    strcat (strPathTemp, "/.fcitx/");
    if (access (strPathTemp, 0))
	mkdir (strPathTemp, S_IRWXU);
    strcat (strPathTemp, TEMP_FILE);
    fp = fopen (strPathTemp, "wb");
    if (!fp) {
	fprintf (stderr, "无法保存拼音用户词库：%s\n", strPathTemp);
	return;
    }

    for (i = 0; i < iPYFACount; i++) {
	for (j = 0; j < PYFAList[i].iBase; j++) {
	    iTemp = PYFAList[i].pyBase[j].iUserPhrase;
	    if (iTemp) {
		fwrite (&i, sizeof (int), 1, fp);
		fwrite (PYFAList[i].pyBase[j].strHZ, sizeof (char) * 2, 1, fp);
		fwrite (&iTemp, sizeof (int), 1, fp);
		phrase = PYFAList[i].pyBase[j].userPhrase->next;
		for (k = 0; k < PYFAList[i].pyBase[j].iUserPhrase; k++) {
		    iTemp = strlen (phrase->strMap);
		    fwrite (&iTemp, sizeof (int), 1, fp);
		    fwrite (phrase->strMap, sizeof (char) * iTemp, 1, fp);
		    fwrite (phrase->strPhrase, sizeof (char) * iTemp, 1, fp);
		    iTemp = phrase->iIndex;
		    fwrite (&iTemp, sizeof (int), 1, fp);
		    iTemp = phrase->iHit;
		    fwrite (&iTemp, sizeof (int), 1, fp);
		    phrase = phrase->next;
		}
	    }
	}
    }

    fclose (fp);
    snprintf(strPath, sizeof(strPath), "%s/.fcitx/%s",
             getenv ("HOME"),
             PY_USERPHRASE_FILE);
    if (access (strPath, 0))
	unlink (strPath);
    rename (strPathTemp, strPath);
}

void SavePYFreq (void)
{
    int             i, j, k;
    char            strPath[PATH_MAX];
    char            strPathTemp[PATH_MAX];
    int             iStrLen = PATH_MAX;
    FILE           *fp;
    PyFreq         *pPyFreq;
    HZ             *hz;

    snprintf(strPathTemp, iStrLen, "%s/.fcitx/",
             getenv ("HOME"));
    iStrLen -= strlen(strPathTemp);
    if (access (strPathTemp, 0))
	mkdir (strPathTemp, S_IRWXU);
    strncat(strPathTemp, TEMP_FILE, iStrLen);
    fp = fopen (strPathTemp, "wb");
    if (!fp) {
	fprintf (stderr, "无法保存常用词表：%s\n", strPathTemp);
	return;
    }
    i = 0;
    pPyFreq = pyFreq->next;
    while (pPyFreq) {
	if (!pPyFreq->bIsSym)
	    i++;
	pPyFreq = pPyFreq->next;
    }
    fwrite (&i, sizeof (uint), 1, fp);
    pPyFreq = pyFreq->next;
    while (pPyFreq) {
	if (!pPyFreq->bIsSym) {
	    fwrite (pPyFreq->strPY, sizeof (char) * 11, 1, fp);
	    j = pPyFreq->iCount;
	    fwrite (&j, sizeof (int), 1, fp);
	    hz = pPyFreq->HZList->next;
	    for (k = 0; k < pPyFreq->iCount; k++) {
		fwrite (hz->strHZ, sizeof (char) * 2, 1, fp);
		j = hz->iPYFA;
		fwrite (&j, sizeof (int), 1, fp);
		j = hz->iHit;
		fwrite (&j, sizeof (int), 1, fp);
		j = hz->iIndex;
		fwrite (&j, sizeof (int), 1, fp);
		hz = hz->next;
	    }
	}
	pPyFreq = pPyFreq->next;
    }

    fclose (fp);
    snprintf(strPath, sizeof(strPath), "%s/.fcitx/%s",
             getenv ("HOME"),
             PY_FREQ_FILE);
    if (access (strPath, 0))
	unlink (strPath);
    rename (strPathTemp, strPath);
}

/*
 * 保存索引文件
 */
void SavePYIndex (void)
{
    int             i, j, k, l;
    char            strPath[PATH_MAX];
    char            strPathTemp[PATH_MAX];
    int             iStrLen = PATH_MAX;
    FILE           *fp;

    snprintf(strPathTemp, iStrLen, "%s/.fcitx/",
             getenv ("HOME"));
    iStrLen -= strlen(strPathTemp);
    if (access (strPathTemp, 0))
	mkdir (strPathTemp, S_IRWXU);
    strncat(strPathTemp, TEMP_FILE, iStrLen);
    fp = fopen (strPathTemp, "wb");
    if (!fp) {
	fprintf (stderr, "无法保存索引文件：%s\n", strPathTemp);
	return;
    }

    //保存计数器
    fwrite (&iCounter, sizeof (uint), 1, fp);
    //先保存索引不为0的单字
    k = -1;
    for (i = 0; i < iPYFACount; i++) {
	if (PYFAList[i].iChangeCount) {
	    for (j = 0; j < PYFAList[i].iBase; j++) {
		if (PYFAList[i].pyBase[j].iIndex) {
		    fwrite (&i, sizeof (int), 1, fp);
		    fwrite (&j, sizeof (int), 1, fp);
		    fwrite (&k, sizeof (int), 1, fp);
		    l = PYFAList[i].pyBase[j].iIndex;
		    fwrite (&l, sizeof (uint), 1, fp);
		    l = PYFAList[i].pyBase[j].iHit;
		    fwrite (&l, sizeof (uint), 1, fp);
		}
	    }
	}
    }

    //再保存索引不为0的系统词组
    for (i = 0; i < iPYFACount; i++) {
	if (PYFAList[i].iChangeCount) {
	    for (j = 0; j < PYFAList[i].iBase; j++) {
		if (PYFAList[i].pyBase[j].iChangeCount) {
		    for (k = 0; k < PYFAList[i].pyBase[j].iPhrase; k++) {
			if (PYFAList[i].pyBase[j].phrase[k].iIndex) {
			    fwrite (&i, sizeof (int), 1, fp);
			    fwrite (&j, sizeof (int), 1, fp);
			    fwrite (&k, sizeof (int), 1, fp);
			    l = PYFAList[i].pyBase[j].phrase[k].iIndex;
			    fwrite (&l, sizeof (uint), 1, fp);
			    l = PYFAList[i].pyBase[j].phrase[k].iHit;
			    fwrite (&l, sizeof (uint), 1, fp);
			}
		    }
		}
	    }
	}
    }

    fclose (fp);
    snprintf(strPath, sizeof(strPath), "%s/.fcitx/%s",
             getenv ("HOME"),
             PY_INDEX_FILE);
    if (access (strPath, 0))
	unlink (strPath);
    rename (strPathTemp, strPath);
}

/*
 * 设置拼音的常用字表
 * 只有以下情形才能设置
 *	当用户输入单字时
 * 至于常用词的问题暂时不考虑
 */
void PYAddFreq (int iIndex)
{
    int             i;
    HZ             *HZTemp;
    PyFreq         *freq;
    HZ             *hz;

    //能到这儿来，就说明候选列表中都是单字
    //首先，看这个字是不是已经在常用字表中
    i = 1;
    if (pCurFreq) {
	i = -1;
	if (PYCandWords[iIndex].iWhich != PY_CAND_FREQ) {
	    //说明该字是系统单字
	    HZTemp = pCurFreq->HZList->next;
	    for (i = 0; i < pCurFreq->iCount; i++) {
		if (!strcmp (PYFAList[PYCandWords[iIndex].cand.base.iPYFA].pyBase[PYCandWords[iIndex].cand.base.iBase].strHZ, HZTemp->strHZ)) {
		    i = -1;
		    break;
		}
		HZTemp = HZTemp->next;
	    }
	}
    }

    //借用i来指示是否需要添加新的常用字
    if (i < 0)
	return;
    PYSetCandWordsFlag (False);
    //需要添加该字，此时该字必然是系统单字
    if (!pCurFreq) {
	freq = (PyFreq *) malloc (sizeof (PyFreq));
	freq->HZList = (HZ *) malloc (sizeof (HZ));
	freq->HZList->next = NULL;
	strcpy (freq->strPY, strFindString);
	freq->next = NULL;
	freq->iCount = 0;
	freq->bIsSym = False;
	pCurFreq = pyFreq;
	for (i = 0; i < iPYFreqCount; i++)
	    pCurFreq = pCurFreq->next;
	pCurFreq->next = freq;
	iPYFreqCount++;
	pCurFreq = freq;
    }

    HZTemp = (HZ *) malloc (sizeof (HZ));
    strcpy (HZTemp->strHZ, PYFAList[PYCandWords[iIndex].cand.base.iPYFA].pyBase[PYCandWords[iIndex].cand.base.iBase].strHZ);
    HZTemp->iPYFA = PYCandWords[iIndex].cand.base.iPYFA;
    HZTemp->iHit = 0;
    HZTemp->iIndex = 0;
    HZTemp->flag = 0;
    HZTemp->next = NULL;
    //将HZTemp加到链表尾部
    hz = pCurFreq->HZList;
    for (i = 0; i < pCurFreq->iCount; i++)
	hz = hz->next;
    hz->next = HZTemp;
    pCurFreq->iCount++;
    iNewFreqCount++;
    if (iNewFreqCount == AUTOSAVE_FREQ_COUNT) {
	SavePYFreq ();
	iNewFreqCount = 0;
    }
}

/*
 * 删除拼音常用字表中的某个字
 */
void PYDelFreq (int iIndex)
{
    HZ             *hz;

    //能到这儿来，就说明候选列表中都是单字
    //首先，看这个字是不是已经在常用字表中
    if (PYCandWords[iIndex].iWhich != PY_CAND_FREQ)
	return;
    PYSetCandWordsFlag (False);
    //先找到需要删除单字的位置
    hz = pCurFreq->HZList;
    while (hz->next != PYCandWords[iIndex].cand.freq.hz)
	hz = hz->next;
    hz->next = PYCandWords[iIndex].cand.freq.hz->next;
    free (PYCandWords[iIndex].cand.freq.hz);
    pCurFreq->iCount--;
    iNewFreqCount++;
    if (iNewFreqCount == AUTOSAVE_FREQ_COUNT) {
	SavePYFreq ();
	iNewFreqCount = 0;
    }
}

/* 
 * 判断一个字是否已经是常用字
 */
Bool PYIsInFreq (char *strHZ)
{
    HZ             *hz;
    int             i;

    if (!pCurFreq || pCurFreq->bIsSym)
	return False;
    hz = pCurFreq->HZList->next;
    for (i = 0; i < pCurFreq->iCount; i++) {
	if (!strcmp (strHZ, hz->strHZ))
	    return True;
	hz = hz->next;
    }

    return False;
}

/*
 * 取得拼音的联想字串
 * 	按照频率来定排列顺序
 */
INPUT_RETURN_VALUE PYGetLegendCandWords (SEARCH_MODE mode)
{
    int             i, j;
    char            strTemp[2];
    PyPhrase       *phrase;

    if (!strPYLegendSource[0])
	return IRV_TO_PROCESS;
    if (mode == SM_FIRST) {
	iLegendCandPageCount = 0;
	iLegendCandWordCount = 0;
	iCurrentLegendCandPage = 0;
	PYResetFlags ();
	pyBaseForLengend = NULL;
	for (i = 0; i < iPYFACount; i++) {
	    if (!strncmp (strPYLegendMap, PYFAList[i].strMap, 2)) {
		for (j = 0; j < PYFAList[i].iBase; j++) {
		    if (!strncmp (strPYLegendSource, PYFAList[i].pyBase[j].strHZ, 2)) {
			pyBaseForLengend = &(PYFAList[i].pyBase[j]);
			goto _HIT;
		    }
		}
	    }
	}

      _HIT:
	if (!pyBaseForLengend)
	    return IRV_TO_PROCESS;
	bShowCursor = False;
    }
    else {
	if (!iLegendCandPageCount)
	    return IRV_TO_PROCESS;
	if (mode == SM_NEXT) {
	    if (iCurrentLegendCandPage == iLegendCandPageCount)
		return IRV_DO_NOTHING;
	    iLegendCandWordCount = 0;
	    iCurrentLegendCandPage++;
	}
	else {
	    if (!iCurrentLegendCandPage)
		return IRV_DO_NOTHING;
	    iCurrentLegendCandPage--;
	    PYSetLegendCandWordsFlag (False);
	}
    }

    for (i = 0; i < pyBaseForLengend->iPhrase; i++) {
	if (strlen (strPYLegendSource) == 2) {
	    if (strlen (pyBaseForLengend->phrase[i].strPhrase) == 2 && ((mode != SM_PREV && !pyBaseForLengend->phrase[i].flag) || (mode == SM_PREV && pyBaseForLengend->phrase[i].flag))) {
		if (!PYAddLengendCandWord (&pyBaseForLengend->phrase[i], mode))
		    break;
	    }
	}
	else if (strlen (pyBaseForLengend->phrase[i].strPhrase) == strlen (strPYLegendSource)) {
	    if (!strncmp (strPYLegendSource + 2, pyBaseForLengend->phrase[i].strPhrase, strlen (strPYLegendSource + 2)) && ((mode != SM_PREV && !pyBaseForLengend->phrase[i].flag) || (mode == SM_PREV && pyBaseForLengend->phrase[i].flag))) {
		if (!PYAddLengendCandWord (&pyBaseForLengend->phrase[i], mode))
		    break;
	    }
	}
    }

    phrase = pyBaseForLengend->userPhrase->next;
    for (i = 0; i < pyBaseForLengend->iUserPhrase; i++) {
	if (strlen (strPYLegendSource) == 2) {
	    if (strlen (phrase->strPhrase) == 2 && ((mode != SM_PREV && !phrase->flag) || (mode == SM_PREV && phrase->flag))) {
		if (!PYAddLengendCandWord (phrase, mode))
		    break;
	    }
	}
	else if (strlen (phrase->strPhrase) == strlen (strPYLegendSource)) {
	    if (!strncmp (strPYLegendSource + 2, phrase->strPhrase, strlen (strPYLegendSource + 2)) && ((mode != SM_PREV && !phrase->flag) || (mode == SM_PREV && phrase->flag))) {
		if (!PYAddLengendCandWord (phrase, mode))
		    break;
	    }
	}

	phrase = phrase->next;
    }

    PYSetLegendCandWordsFlag (True);
    if (!bDisablePagingInLegend && mode != SM_PREV && iCurrentLegendCandPage == iLegendCandPageCount) {
	for (i = 0; i < pyBaseForLengend->iPhrase; i++) {
	    if (strlen (strPYLegendSource) == 2) {
		if (strlen (pyBaseForLengend->phrase[i].strPhrase) == 2 && !pyBaseForLengend->phrase[i].flag) {
		    iLegendCandPageCount++;
		    goto _NEWPAGE;
		}
	    }
	    else if (strlen (pyBaseForLengend->phrase[i].strPhrase) == strlen (strPYLegendSource)) {
		if (!strncmp (strPYLegendSource + 2, pyBaseForLengend->phrase[i].strPhrase, strlen (strPYLegendSource + 2)) && (!pyBaseForLengend->phrase[i].flag)) {
		    iLegendCandPageCount++;
		    goto _NEWPAGE;
		}
	    }
	}

	phrase = pyBaseForLengend->userPhrase->next;
	for (i = 0; i < pyBaseForLengend->iUserPhrase; i++) {
	    if (strlen (strPYLegendSource) == 2) {
		if (strlen (phrase->strPhrase) == 2 && (!phrase->flag)) {
		    iLegendCandPageCount++;
		    goto _NEWPAGE;
		}
	    }
	    else if (strlen (pyBaseForLengend->phrase[i].strPhrase) == strlen (strPYLegendSource)) {
		if (!strncmp (strPYLegendSource + 2, phrase->strPhrase, strlen (strPYLegendSource + 2)) && (!phrase->flag)) {
		    iLegendCandPageCount++;
		    goto _NEWPAGE;
		}
	    }
	    phrase = phrase->next;
	}
      _NEWPAGE:
	;
    }

    uMessageUp = 2;
    strcpy (messageUp[0].strMsg, "联想：");
    messageUp[0].type = MSG_TIPS;
    strcpy (messageUp[1].strMsg, strPYLegendSource);
    messageUp[1].type = MSG_INPUT;
    strTemp[1] = '\0';
    uMessageDown = 0;
    for (i = 0; i < iLegendCandWordCount; i++) {
	strTemp[0] = i + 1 + '0';
	if (i == 9)
	    strTemp[0] = '0';
	strcpy (messageDown[uMessageDown].strMsg, strTemp);
	messageDown[uMessageDown++].type = MSG_INDEX;
	strcpy (messageDown[uMessageDown].strMsg, PYLegendCandWords[i].phrase->strPhrase + PYLegendCandWords[i].iLength);
	if (i != (iLegendCandWordCount - 1)) {
	    strcat (messageDown[uMessageDown].strMsg, " ");
	}
	messageDown[uMessageDown++].type = ((i == 0) ? MSG_FIRSTCAND : MSG_OTHER);
    }

    bIsInLegend = (iLegendCandWordCount != 0);
    return IRV_DISPLAY_CANDWORDS;
}

Bool PYAddLengendCandWord (PyPhrase * phrase, SEARCH_MODE mode)
{
    int             i = 0, j;

    if (mode == SM_PREV) {
	for (i = (iLegendCandWordCount - 1); i >= 0; i--) {
	    if (PYLegendCandWords[i].phrase->iHit >= phrase->iHit) {
		i++;
		break;
	    }
	}

	if (i < 0) {
	    if (iLegendCandWordCount == iMaxCandWord)
		return True;
	    i = 0;
	}
	else if (iLegendCandWordCount == iMaxCandWord)
	    i--;
    }
    else {
	for (i = 0; i < iLegendCandWordCount; i++) {
	    if (PYLegendCandWords[i].phrase->iHit < phrase->iHit)
		break;
	}
	if (i == iMaxCandWord)
	    return True;
    }

    if (mode == SM_PREV) {
	if (iLegendCandWordCount == iMaxCandWord) {
	    for (j = 0; j < i; j++) {
		PYLegendCandWords[j].phrase = PYLegendCandWords[j + 1].phrase;
		PYLegendCandWords[j].iLength = PYLegendCandWords[j + 1].iLength;
	    }
	}
	else {
	    for (j = iLegendCandWordCount; j > i; j--) {
		PYLegendCandWords[j].phrase = PYLegendCandWords[j - 1].phrase;
		PYLegendCandWords[j].iLength = PYLegendCandWords[j - 1].iLength;
	    }
	}
    }
    else {
	j = iLegendCandWordCount;
	if (iLegendCandWordCount == iMaxCandWord)
	    j--;
	for (; j > i; j--) {
	    PYLegendCandWords[j].phrase = PYLegendCandWords[j - 1].phrase;
	    PYLegendCandWords[j].iLength = PYLegendCandWords[j - 1].iLength;
	}
    }

    PYLegendCandWords[i].phrase = phrase;
    PYLegendCandWords[i].iLength = strlen (strPYLegendSource) - 2;
    if (iLegendCandWordCount != iMaxCandWord)
	iLegendCandWordCount++;
    return True;
}

char           *PYGetLegendCandWord (int iIndex)
{
    if (iLegendCandWordCount) {
	if (iIndex > (iLegendCandWordCount - 1))
	    iIndex = iLegendCandWordCount - 1;
	strcpy (strPYLegendSource, PYLegendCandWords[iIndex].phrase->strPhrase + PYLegendCandWords[iIndex].iLength);
	strcpy (strPYLegendMap, PYLegendCandWords[iIndex].phrase->strMap + PYLegendCandWords[iIndex].iLength);
	PYGetLegendCandWords (SM_FIRST);
	return strPYLegendSource;
    }

    return NULL;
}

void PYSetLegendCandWordsFlag (Bool flag)
{
    int             i;

    for (i = 0; i < iLegendCandWordCount; i++)
	PYLegendCandWords[i].phrase->flag = flag;
}

void PYGetPYByHZ (char *strHZ, char *strPY)
{
    int             i, j;
    char            str_PY[MAX_PY_LENGTH + 1];

    strPY[0] = '\0';
    for (i = iPYFACount - 1; i >= 0; i--) {
	if (MapToPY (PYFAList[i].strMap, str_PY)) {
	    for (j = 0; j < PYFAList[i].iBase; j++) {
		if (!strcmp (PYFAList[i].pyBase[j].strHZ, strHZ)) {
		    if (strPY[0])
			strcat (strPY, " ");
		    strcat (strPY, str_PY);
		}
	    }
	}
    }
}
