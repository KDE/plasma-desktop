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

#include "table.h"
#include "punc.h"

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include "tools.h"
#include "InputWindow.h"
#include "py.h"
#include "pyParser.h"
#include "ui.h"

#ifdef _USE_XFT
#include <ft2build.h>
#include <X11/Xft/Xft.h>
#endif

TABLE          *table;
INT8            iTableIMIndex = 0;
INT8            iTableCount;

Bool            bTableDictLoaded = False;		//Loads tables only if needed

RECORD         *currentRecord = (RECORD *)NULL;
RECORD	       *recordHead = (RECORD *)NULL;
RECORD         *tableSingleHZ[SINGLE_HZ_COUNT];		//Records the single characters in table to speed auto phrase
RECORD	       *pCurCandRecord = (RECORD *)NULL;	//Records current cand word selected, to update the hit-frequency information
TABLECANDWORD   tableCandWord[MAX_CAND_WORD*2];

RECORD_INDEX   *recordIndex = (RECORD_INDEX *)NULL;

AUTOPHRASE     *autoPhrase = (AUTOPHRASE *)NULL;
AUTOPHRASE     *insertPoint = (AUTOPHRASE *)NULL;

uint            iAutoPhrase = 0;
uint            iTableCandDisplayed;
uint            iTableTotalCandCount;
INT16           iTableOrderChanged = 0;
char            strTableLegendSource[PHRASE_MAX_LENGTH * 2 + 1] = "\0";

FH             *fh;
int             iFH = 0;
unsigned int    iTableIndex = 0;

Bool            bIsTableDelPhrase = False;
HOTKEYS         hkTableDelPhrase[HOT_KEY_COUNT] = { CTRL_7, 0 };
Bool            bIsTableAdjustOrder = False;
HOTKEYS         hkTableAdjustOrder[HOT_KEY_COUNT] = { CTRL_6, 0 };
Bool            bIsTableAddPhrase = False;
HOTKEYS         hkTableAddPhrase[HOT_KEY_COUNT] = { CTRL_8, 0 };
HOTKEYS         hkGetPY[HOT_KEY_COUNT] = { CTRL_ALT_E, 0 };

INT8            iTableChanged = 0;
INT8            iTableNewPhraseHZCount;
Bool            bCanntFindCode;	//Records if new phrase has corresponding code - should be always false
char           *strNewPhraseCode;

SINGLE_HZ       hzLastInput[PHRASE_MAX_LENGTH];	//Records last HZ input
INT16           iHZLastInputCount = 0;
Bool            bTablePhraseTips = False;

ADJUSTORDER     PYBaseOrder;
extern char     strPYAuto[];

extern INT8     iInternalVersion;

extern Display *dpy;
extern Window   inputWindow;

#ifdef _USE_XFT
extern XftFont *xftFont;
extern XftFont *xftFontEn;
#else
extern XFontSet fontSet;
#endif

extern char     strCodeInput[];
extern Bool     bIsDoInputOnly;
extern int      iCandPageCount;
extern int      iCurrentCandPage;
extern int      iCandWordCount;
extern int      iLegendCandWordCount;
extern int      iLegendCandPageCount;
extern int      iCurrentLegendCandPage;
extern int      iCodeInputCount;
extern int      iMaxCandWord;
extern char     strStringGet[];
extern MESSAGE  messageUp[];
extern uint     uMessageUp;
extern MESSAGE  messageDown[];
extern uint     uMessageDown;
extern Bool     bPointAfterNumber;

extern INT8     iIMIndex;
extern Bool     bUseLegend;
extern Bool     bIsInLegend;
extern INT8     lastIsSingleHZ;
extern Bool     bDisablePagingInLegend;
extern Bool     bShowCursor;

extern ADJUSTORDER baseOrder;
extern Bool     bSP;
extern Bool     bPYBaseDictLoaded;
extern uint     iFixedInputWindowWidth;

extern PYFA    *PYFAList;
extern PYCandWord PYCandWords[];

extern char     strFindString[];
extern ParsePYStruct findMap;

/*
 * 读取码表输入法的名称和文件路径
 */
void LoadTableInfo (void)
{
    FILE           *fp;
    char            strPath[PATH_MAX];
    int             i;
    char           *pstr;

    FreeTableIM ();
    if (table)
	free (table);
    iTableCount = 0;

     snprintf (strPath, sizeof(strPath), "%s/.fcitx/%s",
              getenv ("HOME"),
              TABLE_CONFIG_FILENAME);

    if (access (strPath, 0))
	snprintf (strPath, sizeof(strPath), PKGDATADIR "/data/%s", TABLE_CONFIG_FILENAME);

    fp = fopen (strPath, "rt");
    if (!fp)
	return;

    //首先来看看有多少个码表
    while (1) {
	if (!fgets (strPath, PATH_MAX, fp))
	    break;

	i = strlen (strPath) - 1;
	while ((i >= 0) && (strPath[i] == ' ' || strPath[i] == '\n'))
	    strPath[i--] = '\0';

	pstr = strPath;
	if (*pstr == ' ')
	    pstr++;
	if (pstr[0] == '#')
	    continue;

	if (strstr (pstr, "[码表]"))
	    iTableCount++;
    }

    table = (TABLE *) malloc (sizeof (TABLE) * iTableCount);
    for (iTableIMIndex = 0; iTableIMIndex < iTableCount; iTableIMIndex++) {
	table[iTableIMIndex].strInputCode = NULL;
	table[iTableIMIndex].strEndCode = NULL;
	table[iTableIMIndex].strName[0] = '\0';
	table[iTableIMIndex].strPath[0] = '\0';
	table[iTableIMIndex].strSymbolFile[0] = '\0';
	table[iTableIMIndex].tableOrder = AD_NO;
	table[iTableIMIndex].bUsePY = False;
	table[iTableIMIndex].cPinyin = '\0';
	table[iTableIMIndex].iTableAutoSendToClient = -1;
	table[iTableIMIndex].iTableAutoSendToClientWhenNone = -1;
	table[iTableIMIndex].rule = NULL;
	table[iTableIMIndex].bUseMatchingKey = False;
	table[iTableIMIndex].cMatchingKey = '\0';
	table[iTableIMIndex].bTableExactMatch = False;
	table[iTableIMIndex].iMaxPhraseAllowed = 0;
	table[iTableIMIndex].bAutoPhrase = True;
	table[iTableIMIndex].bAutoPhrasePhrase = True;
	table[iTableIMIndex].iSaveAutoPhraseAfter = 0;
	table[iTableIMIndex].iAutoPhrase = 4;
	table[iTableIMIndex].bPromptTableCode = True;
	table[iTableIMIndex].strSymbol[0] = '\0';
	table[iTableIMIndex].bHasPinyin = False;
	strcpy (table[iTableIMIndex].choose, "1234567890");//初始化为1234567890
    }

    iTableIMIndex = -1;
    //读取其它属性的默认值
    if (iTableCount) {
	rewind (fp);

	while (1) {
	    if (!fgets (strPath, PATH_MAX, fp))
		break;

	    i = strlen (strPath) - 1;
	    while ((i >= 0) && (strPath[i] == ' ' || strPath[i] == '\n'))
		strPath[i--] = '\0';

	    pstr = strPath;
	    if (*pstr == ' ')
		pstr++;
	    if (pstr[0] == '#')
		continue;

	    if (!strcmp (pstr, "[码表]")) {
		if (iTableIMIndex != -1) {
		    if (table[iTableIMIndex].strName[0] == '\0' || table[iTableIMIndex].strPath[0] == '\0') {
			iTableCount = 0;
			free (table);
			fprintf (stderr, "The config file of No.%d table errors!\n", iTableIMIndex);
			return;
		    }
		}
		iTableIMIndex++;
	    }
	    else if (MyStrcmp (pstr, "名称=")) {
		pstr += 5;
		strcpy (table[iTableIMIndex].strName, pstr);
	    }
	    else if (MyStrcmp (pstr, "码表=")) {
		pstr += 5;
		strcpy (table[iTableIMIndex].strPath, pstr);
	    }
	    else if (MyStrcmp (pstr, "调频=")) {
		pstr += 5;
		table[iTableIMIndex].tableOrder = (ADJUSTORDER) atoi (pstr);
	    }
	    else if (MyStrcmp (pstr, "拼音=")) {
		pstr += 5;
		table[iTableIMIndex].bUsePY = atoi (pstr);
	    }
	    else if (MyStrcmp (pstr, "拼音键=")) {
		pstr += 7;
		while (*pstr == ' ')
		    pstr++;
		table[iTableIMIndex].cPinyin = *pstr;
	    }
	    else if (MyStrcmp (pstr, "自动上屏=")) {
		pstr += 9;
		table[iTableIMIndex].iTableAutoSendToClient = atoi (pstr);
	    }
	    else if (MyStrcmp (pstr, "空码自动上屏=")) {
		pstr += 13;
		table[iTableIMIndex].iTableAutoSendToClientWhenNone = atoi (pstr);
	    }
	    else if (MyStrcmp (pstr, "中止键=")) {
		pstr += 7;
		while (*pstr == ' ' || *pstr == '\t')
		    pstr++;
		table[iTableIMIndex].strEndCode = (char *) malloc (sizeof (char) * (strlen (pstr) + 1));
		strcpy (table[iTableIMIndex].strEndCode, pstr);
	    }
	    else if (MyStrcmp (pstr, "模糊=")) {
		pstr += 5;
		table[iTableIMIndex].bUseMatchingKey = atoi (pstr);
	    }
	    else if (MyStrcmp (pstr, "模糊键=")) {
		pstr += 7;
		while (*pstr == ' ')
		    pstr++;
		table[iTableIMIndex].cMatchingKey = *pstr;
	    }
	    else if (MyStrcmp (pstr, "精确匹配=")) {
		pstr += 9;
		table[iTableIMIndex].bTableExactMatch = atoi (pstr);
	    }
	    else if (MyStrcmp (pstr, "最长词组字数=")) {
		pstr += 13;
		table[iTableIMIndex].iMaxPhraseAllowed = atoi (pstr);
	    }	    
	    else if (MyStrcmp (pstr, "自动词组=")) {
		pstr += 9;
		table[iTableIMIndex].bAutoPhrase = atoi (pstr);
	    }
	    else if (MyStrcmp (pstr, "自动词组长度=")) {
		pstr += 13;
		table[iTableIMIndex].iAutoPhrase = atoi (pstr);
	    }
	    else if (MyStrcmp (pstr, "词组参与自动造词=")) {
		pstr += 17;
		table[iTableIMIndex].bAutoPhrasePhrase = atoi (pstr);
	    }
	    else if (MyStrcmp (pstr, "保存自动词组=")) {
		pstr += 13;
		table[iTableIMIndex].iSaveAutoPhraseAfter = atoi (pstr);
	    }
	    else if (MyStrcmp (pstr, "提示编码=")) {
		pstr += 9;
		table[iTableIMIndex].bPromptTableCode = atoi (pstr);
	    }
	    else if (MyStrcmp (pstr, "符号=")) {
		pstr += 5;
		strcpy (table[iTableIMIndex].strSymbol, pstr);
	    }
	    else if (MyStrcmp (pstr, "符号文件=")) {
		pstr += 9;
		strcpy (table[iTableIMIndex].strSymbolFile, pstr);
	    }
	    else if(MyStrcmp (pstr, "候选词选择键=")){
                pstr += 13;
                strncpy (table[iTableIMIndex].choose, pstr, 10);		
            }
	}
    }

    fclose (fp);
}

Bool LoadTableDict (void)
{
    char            strCode[MAX_CODE_LENGTH + 1];
    char            strHZ[PHRASE_MAX_LENGTH * 2 + 1];
    FILE           *fpDict;
    RECORD         *recTemp;
    char            strPath[PATH_MAX];
    unsigned int    i = 0;
    unsigned int    iTemp, iTempCount;
    char            cChar = 0, cTemp;
    INT8            iVersion = 1;
    int             iRecordIndex;

    //首先，来看看我们该调入哪个码表
    for (i = 0; i < iTableCount; i++) {
	if (table[i].iIMIndex == iIMIndex)
	    iTableIMIndex = i;
    }

    //读入码表
#ifdef _DEBUG
    fprintf (stderr, "Loading Table Dict\n");
#endif

    snprintf (strPath, sizeof(strPath), "%s/.fcitx/%s",
              getenv ("HOME"),
              table[iTableIMIndex].strPath);;

    if (access (strPath, 0))
	snprintf (strPath, sizeof(strPath), PKGDATADIR "/data/%s", table[iTableIMIndex].strPath);

    fpDict = fopen (strPath, "rb");
    if (!fpDict) {
	fprintf (stderr, "Cannot load table file: %s\n", strPath);
	return False;
    }

    //先读取码表的信息
    //判断版本信息
    fread (&iTemp, sizeof (unsigned int), 1, fpDict);
    if (!iTemp) {
	fread (&iVersion, sizeof (INT8), 1, fpDict);
	iVersion = (iVersion < iInternalVersion);
	fread (&iTemp, sizeof (unsigned int), 1, fpDict);
    }

    table[iTableIMIndex].strInputCode = (char *) malloc (sizeof (char) * (iTemp + 1));
    fread (table[iTableIMIndex].strInputCode, sizeof (char), iTemp + 1, fpDict);
    /*
     * 建立索引，加26是为了为拼音编码预留空间
     */
    recordIndex = (RECORD_INDEX *) malloc ((strlen (table[iTableIMIndex].strInputCode) + 26) * sizeof (RECORD_INDEX));
    for (iTemp = 0; iTemp < strlen (table[iTableIMIndex].strInputCode) + 26; iTemp++) {
	recordIndex[iTemp].cCode = 0;
	recordIndex[iTemp].record = NULL;
    }
    /* ********************************************************************** */

    fread (&(table[iTableIMIndex].iCodeLength), sizeof (unsigned char), 1, fpDict);
    if (table[iTableIMIndex].iTableAutoSendToClient == -1)
	table[iTableIMIndex].iTableAutoSendToClient = table[iTableIMIndex].iCodeLength;
    if (table[iTableIMIndex].iTableAutoSendToClientWhenNone == -1)
	table[iTableIMIndex].iTableAutoSendToClientWhenNone = table[iTableIMIndex].iCodeLength;

    if (!iVersion)
	fread (&(table[iTableIMIndex].iPYCodeLength), sizeof (unsigned char), 1, fpDict);
    else
	table[iTableIMIndex].iPYCodeLength = table[iTableIMIndex].iCodeLength;

    fread (&iTemp, sizeof (unsigned int), 1, fpDict);
    table[iTableIMIndex].strIgnoreChars = (char *) malloc (sizeof (char) * (iTemp + 1));
    fread (table[iTableIMIndex].strIgnoreChars, sizeof (char), iTemp + 1, fpDict);

    fread (&(table[iTableIMIndex].bRule), sizeof (unsigned char), 1, fpDict);

    if (table[iTableIMIndex].bRule) {	//表示有组词规则
	table[iTableIMIndex].rule = (RULE *) malloc (sizeof (RULE) * (table[iTableIMIndex].iCodeLength - 1));
	for (i = 0; i < table[iTableIMIndex].iCodeLength - 1; i++) {
	    fread (&(table[iTableIMIndex].rule[i].iFlag), sizeof (unsigned char), 1, fpDict);
	    fread (&(table[iTableIMIndex].rule[i].iWords), sizeof (unsigned char), 1, fpDict);
	    table[iTableIMIndex].rule[i].rule = (RULE_RULE *) malloc (sizeof (RULE_RULE) * table[iTableIMIndex].iCodeLength);
	    for (iTemp = 0; iTemp < table[iTableIMIndex].iCodeLength; iTemp++) {
		fread (&(table[iTableIMIndex].rule[i].rule[iTemp].iFlag), sizeof (unsigned char), 1, fpDict);
		fread (&(table[iTableIMIndex].rule[i].rule[iTemp].iWhich), sizeof (unsigned char), 1, fpDict);
		fread (&(table[iTableIMIndex].rule[i].rule[iTemp].iIndex), sizeof (unsigned char), 1, fpDict);
	    }
	}
    }

    recordHead = (RECORD *) malloc (sizeof (RECORD));
    currentRecord = recordHead;

    fread (&(table[iTableIMIndex].iRecordCount), sizeof (unsigned int), 1, fpDict);

    for (i = 0; i < SINGLE_HZ_COUNT; i++)
	tableSingleHZ[i] = (RECORD *) NULL;

    iRecordIndex = 0;
    for (i = 0; i < table[iTableIMIndex].iRecordCount; i++) {
	fread (strCode, sizeof (char), table[iTableIMIndex].iPYCodeLength + 1, fpDict);
	fread (&iTemp, sizeof (unsigned int), 1, fpDict);
	fread (strHZ, sizeof (char), iTemp, fpDict);
	recTemp = (RECORD *) malloc (sizeof (RECORD));
	recTemp->strCode = (char *) malloc (sizeof (char) * (strlen (strCode) + 1));
	strcpy (recTemp->strCode, strCode);
	recTemp->strHZ = (char *) malloc (sizeof (char) * iTemp);
	strcpy (recTemp->strHZ, strHZ);

	if (!iVersion) {
	    fread (&cTemp, sizeof (char), 1, fpDict);
	    recTemp->bPinyin = cTemp;
	}

	recTemp->flag = 0;

	fread (&(recTemp->iHit), sizeof (unsigned int), 1, fpDict);
	fread (&(recTemp->iIndex), sizeof (unsigned int), 1, fpDict);
	if (recTemp->iIndex > iTableIndex)
	    iTableIndex = recTemp->iIndex;

	/* 建立索引 */
	if (cChar != recTemp->strCode[0]) {
	    cChar = recTemp->strCode[0];
	    recordIndex[iRecordIndex].cCode = cChar;
	    recordIndex[iRecordIndex].record = recTemp;
	    iRecordIndex++;
	}
	/* **************************************************************** */
	/** 为单字生成一个表   */
	if (strlen (recTemp->strHZ) == 2 && !IsIgnoreChar (strCode[0]) && !recTemp->bPinyin) {
	    iTemp = CalHZIndex (recTemp->strHZ);
	    if (iTemp >= 0 && iTemp < SINGLE_HZ_COUNT) {
		if (tableSingleHZ[iTemp]) {
		    if (strlen (strCode) > strlen (tableSingleHZ[iTemp]->strCode))
			tableSingleHZ[iTemp] = recTemp;
		}
		else
		    tableSingleHZ[iTemp] = recTemp;
	    }
	}

	if (recTemp->bPinyin)
	    table[iTableIMIndex].bHasPinyin = True;

	currentRecord->next = recTemp;
	recTemp->prev = currentRecord;
	currentRecord = recTemp;
    }

    currentRecord->next = recordHead;
    recordHead->prev = currentRecord;

    fclose (fpDict);
#ifdef _DEBUG
    fprintf (stderr, "Load Table Dict OK\n");
#endif

    //读取相应的特殊符号表
    snprintf (strPath, sizeof(strPath), "%s/.fcitx/%s",
              getenv ("HOME"),
              table[iTableIMIndex].strSymbolFile);

    if (access (strPath, 0)) {
	snprintf (strPath, sizeof(strPath), PKGDATADIR "/data/%s",
                  table[iTableIMIndex].strSymbolFile);
        fpDict = fopen (strPath, "rt");
    }

    fpDict = fopen (strPath, "rt");
    if (fpDict) {
	iFH = CalculateRecordNumber (fpDict);
	fh = (FH *) malloc (sizeof (FH) * iFH);

	for (i = 0; i < iFH; i++) {
	    if (EOF == fscanf (fpDict, "%s\n", fh[i].strFH))
		break;
	}
	iFH = i;

	fclose (fpDict);
    }

    strNewPhraseCode = (char *) malloc (sizeof (char) * (table[iTableIMIndex].iCodeLength + 1));
    strNewPhraseCode[table[iTableIMIndex].iCodeLength] = '\0';
    bTableDictLoaded = True;

    iAutoPhrase = 0;
    if (table[iTableIMIndex].bAutoPhrase) {
	//为自动词组分配空间
	autoPhrase = (AUTOPHRASE *) malloc (sizeof (AUTOPHRASE) * AUTO_PHRASE_COUNT);

	//读取上次保存的自动词组信息
#ifdef _DEBUG
	fprintf (stderr, "Loading Autophrase...\n");
#endif

	strcpy (strPath, (char *) getenv ("HOME"));
	strcat (strPath, "/.fcitx/");
	strcat (strPath, table[iTableIMIndex].strName);
	strcat (strPath, "_LastAutoPhrase.tmp");
	fpDict = fopen (strPath, "rb");
	i = 0;
	if (fpDict) {
	    fread (&iAutoPhrase, sizeof (unsigned int), 1, fpDict);

	    for (; i < iAutoPhrase; i++) {
		autoPhrase[i].strCode = (char *) malloc (sizeof (char) * (table[iTableIMIndex].iCodeLength + 1));
		autoPhrase[i].strHZ = (char *) malloc (sizeof (char) * (PHRASE_MAX_LENGTH * 2 + 1));
		fread (autoPhrase[i].strCode, table[iTableIMIndex].iCodeLength + 1, 1, fpDict);
		fread (autoPhrase[i].strHZ, PHRASE_MAX_LENGTH * 2 + 1, 1, fpDict);
		fread (&iTempCount, sizeof (unsigned int), 1, fpDict);
		autoPhrase[i].iSelected = iTempCount;
		if (i == AUTO_PHRASE_COUNT - 1)
		    autoPhrase[i].next = &autoPhrase[0];
		else
		    autoPhrase[i].next = &autoPhrase[i + 1];
	    }
	    fclose (fpDict);
	}

	for (; i < AUTO_PHRASE_COUNT; i++) {
	    autoPhrase[i].strCode = (char *) malloc (sizeof (char) * (table[iTableIMIndex].iCodeLength + 1));
	    autoPhrase[i].strHZ = (char *) malloc (sizeof (char) * (PHRASE_MAX_LENGTH * 2 + 1));
	    autoPhrase[i].iSelected = 0;
	    if (i == AUTO_PHRASE_COUNT - 1)
		autoPhrase[i].next = &autoPhrase[0];
	    else
		autoPhrase[i].next = &autoPhrase[i + 1];
	}

	if (i == AUTO_PHRASE_COUNT)
	    insertPoint = &autoPhrase[0];
	else
	    insertPoint = &autoPhrase[i - 1];

#ifdef _DEBUG
	fprintf (stderr, "Load Autophrase OK\n");
#endif
    }
    else
	autoPhrase = (AUTOPHRASE *) NULL;

    return True;
}

void TableInit (void)
{
    bSP = False;
    PYBaseOrder = baseOrder;
    strPYAuto[0] = '\0';
}

/*
 * 释放当前码表所占用的内存
 * 目的是切换码表时使占用的内存减少
 */
void FreeTableIM (void)
{
    RECORD         *recTemp, *recNext;
    INT16           i;

    if (!recordHead)
	return;

    if (iTableChanged || iTableOrderChanged)
	SaveTableDict ();

    //释放码表
    recTemp = recordHead->next;
    while (recTemp != recordHead) {
	recNext = recTemp->next;

	free (recTemp->strCode);
	free (recTemp->strHZ);
	free (recTemp);

	recTemp = recNext;
    }

    free (recordHead);
    recordHead = NULL;

    if (iFH) {
	free (fh);
	iFH = 0;
    }

    free (table[iTableIMIndex].strInputCode);
    free (table[iTableIMIndex].strIgnoreChars);
    free (table[iTableIMIndex].strEndCode);
    table[iTableIMIndex].iRecordCount = 0;
    bTableDictLoaded = False;

    free (strNewPhraseCode);

    //释放组词规则的空间
    if (table[iTableIMIndex].rule) {
	for (i = 0; i < table[iTableIMIndex].iCodeLength - 1; i++)
	    free (table[iTableIMIndex].rule[i].rule);
	free (table[iTableIMIndex].rule);

	table[iTableIMIndex].rule = NULL;
    }

    //释放索引的空间
    if (recordIndex) {
	free (recordIndex);
	recordIndex = NULL;
    }

    //释放自动词组的空间
    if (autoPhrase) {
	for (i = 0; i < AUTO_PHRASE_COUNT; i++) {
	    free (autoPhrase[i].strCode);
	    free (autoPhrase[i].strHZ);
	}
	free (autoPhrase);

	autoPhrase = (AUTOPHRASE *) NULL;
    }

    baseOrder = PYBaseOrder;
}

void TableResetStatus (void)
{
    bIsTableAddPhrase = False;
    bIsTableDelPhrase = False;
    bIsTableAdjustOrder = False;
    bIsDoInputOnly = False;
    //bSingleHZMode = False;
}

void SaveTableDict (void)
{
    RECORD         *recTemp;
    char            strPath[PATH_MAX];
    char            strPathTemp[PATH_MAX];
    FILE           *fpDict;
    unsigned int    iTemp;
    unsigned int    i;
    char            cTemp;

    //先将码表保存在一个临时文件中，如果保存成功，再将该临时文件改名是码表名──这样可以防止码表文件被破坏
#ifdef _DEBUG
    char            strPathDebug[PATH_MAX];
    int             iDebug = 0;
    FILE           *fpDebug;
#endif

    strcpy (strPathTemp, (char *) getenv ("HOME"));
    strcat (strPathTemp, "/.fcitx/");
    if (access (strPathTemp, 0))
	mkdir (strPathTemp, S_IRWXU);
    strcat (strPathTemp, TEMP_FILE);
    fpDict = fopen (strPathTemp, "wb");
    if (!fpDict) {
	fprintf (stderr, "Cannot create table file: %s\n", strPathTemp);
	return;
    }

#ifdef _DEBUG
    fprintf (stderr, "Saving TABLE Dict...\n");
    strcpy (strPathDebug, (char *) getenv ("HOME"));
    strcat (strPathDebug, "/.fcitx/fcitx.DEBUG");
    fpDebug = fopen (strPathDebug, "wt");
#endif

    //写入版本号--如果第一个字为0,表示后面那个字节为版本号，为了与老版本兼容
    iTemp = 0;
    fwrite (&iTemp, sizeof (unsigned int), 1, fpDict);
    fwrite (&iInternalVersion, sizeof (INT8), 1, fpDict);

    iTemp = strlen (table[iTableIMIndex].strInputCode);
    fwrite (&iTemp, sizeof (unsigned int), 1, fpDict);
    fwrite (table[iTableIMIndex].strInputCode, sizeof (char), iTemp + 1, fpDict);
    fwrite (&(table[iTableIMIndex].iCodeLength), sizeof (INT8), 1, fpDict);
    fwrite (&(table[iTableIMIndex].iPYCodeLength), sizeof (INT8), 1, fpDict);
    iTemp = strlen (table[iTableIMIndex].strIgnoreChars);
    fwrite (&iTemp, sizeof (unsigned int), 1, fpDict);
    fwrite (table[iTableIMIndex].strIgnoreChars, sizeof (char), iTemp + 1, fpDict);

    fwrite (&(table[iTableIMIndex].bRule), sizeof (unsigned char), 1, fpDict);
    if (table[iTableIMIndex].bRule) {	//表示有组词规则
	for (i = 0; i < table[iTableIMIndex].iCodeLength - 1; i++) {
	    fwrite (&(table[iTableIMIndex].rule[i].iFlag), sizeof (unsigned char), 1, fpDict);
	    fwrite (&(table[iTableIMIndex].rule[i].iWords), sizeof (unsigned char), 1, fpDict);
	    for (iTemp = 0; iTemp < table[iTableIMIndex].iCodeLength; iTemp++) {
		fwrite (&(table[iTableIMIndex].rule[i].rule[iTemp].iFlag), sizeof (unsigned char), 1, fpDict);
		fwrite (&(table[iTableIMIndex].rule[i].rule[iTemp].iWhich), sizeof (unsigned char), 1, fpDict);
		fwrite (&(table[iTableIMIndex].rule[i].rule[iTemp].iIndex), sizeof (unsigned char), 1, fpDict);
	    }
	}
    }

    fwrite (&(table[iTableIMIndex].iRecordCount), sizeof (unsigned int), 1, fpDict);
    recTemp = recordHead->next;
    while (recTemp != recordHead) {
	fwrite (recTemp->strCode, sizeof (char), table[iTableIMIndex].iPYCodeLength + 1, fpDict);

#ifdef _DEBUG
	fprintf (fpDebug, "%d:%s", iDebug, recTemp->strCode);
#endif

	iTemp = strlen (recTemp->strHZ) + 1;
	fwrite (&iTemp, sizeof (unsigned int), 1, fpDict);
	fwrite (recTemp->strHZ, sizeof (char), iTemp, fpDict);

#ifdef _DEBUG
	fprintf (fpDebug, " %s", recTemp->strHZ);
#endif

	cTemp = recTemp->bPinyin;
	fwrite (&cTemp, sizeof (char), 1, fpDict);
	fwrite (&(recTemp->iHit), sizeof (unsigned int), 1, fpDict);
	fwrite (&(recTemp->iIndex), sizeof (unsigned int), 1, fpDict);

#ifdef _DEBUG
	fprintf (fpDebug, " %d %d\n", recTemp->iHit, recTemp->iIndex);
	iDebug++;
#endif

	recTemp = recTemp->next;
    }

    fclose (fpDict);

#ifdef _DEBUG
    fprintf (fpDebug, "Save TABLE Dict OK\n");
#endif

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");
    strcat (strPath, table[iTableIMIndex].strPath);
    if (access (strPath, 0))
	unlink (strPath);
    rename (strPathTemp, strPath);

#ifdef _DEBUG
    fprintf (fpDebug, "Rename OK\n");
    fclose (fpDebug);
#endif

    iTableOrderChanged = 0;
    iTableChanged = 0;

    if (autoPhrase) {
	//保存上次的自动词组信息
	strcpy (strPathTemp, (char *) getenv ("HOME"));
	strcat (strPathTemp, "/.fcitx/");
	strcat (strPathTemp, TEMP_FILE);
	fpDict = fopen (strPathTemp, "wb");
	if (fpDict) {
	    fwrite (&iAutoPhrase, sizeof (int), 1, fpDict);
	    for (i = 0; i < iAutoPhrase; i++) {
		fwrite (autoPhrase[i].strCode, table[iTableIMIndex].iCodeLength + 1, 1, fpDict);
		fwrite (autoPhrase[i].strHZ, PHRASE_MAX_LENGTH * 2 + 1, 1, fpDict);
		iTemp = autoPhrase[i].iSelected;
		fwrite (&iTemp, sizeof (unsigned int), 1, fpDict);
	    }
	    fclose (fpDict);
	}

	strcpy (strPath, (char *) getenv ("HOME"));
	strcat (strPath, "/.fcitx/");
	strcat (strPath, table[iTableIMIndex].strName);
	strcat (strPath, "_LastAutoPhrase.tmp");
	if (access (strPath, 0))
	    unlink (strPath);
	rename (strPathTemp, strPath);
    }
}

Bool IsInputKey (int iKey)
{
    char           *p;

    p = table[iTableIMIndex].strInputCode;
    if (!p)
	return False;

    while (*p) {
	if (iKey == *p)
	    return True;
	p++;
    }

    if (table[iTableIMIndex].bHasPinyin) {
	if (iKey >= 'a' && iKey <= 'z')
	    return True;
    }

    return False;
}

Bool IsEndKey (char cChar)
{
    char           *p;

    p = table[iTableIMIndex].strEndCode;
    if (!p)
	return False;

    while (*p) {
	if (cChar == *p)
	    return True;
	p++;
    }

    return False;
}

Bool IsIgnoreChar (char cChar)
{
    char           *p;

    p = table[iTableIMIndex].strIgnoreChars;
    while (*p) {
	if (cChar == *p)
	    return True;
	p++;
    }

    return False;
}

INPUT_RETURN_VALUE DoTableInput (int iKey)
{
    INPUT_RETURN_VALUE retVal;

    if (!bTableDictLoaded)
	LoadTableDict ();

    if (bTablePhraseTips) {
	if (iKey == CTRL_DELETE) {
	    bTablePhraseTips = False;
	    TableDelPhraseByHZ (messageUp[1].strMsg);
	    return IRV_DONOT_PROCESS_CLEAN;
	}
	else if (iKey != LCTRL && iKey != RCTRL && iKey != LSHIFT && iKey != RSHIFT) {
	    uMessageUp = uMessageDown = 0;
	    bTablePhraseTips = False;
	    XUnmapWindow (dpy, inputWindow);
	}
    }

    retVal = IRV_DO_NOTHING;
    if (IsInputKey (iKey) || IsEndKey (iKey) || iKey == table[iTableIMIndex].cMatchingKey || iKey == table[iTableIMIndex].cPinyin) {
	bIsInLegend = False;

	if (!bIsTableAddPhrase && !bIsTableDelPhrase && !bIsTableAdjustOrder) {
	    if (strCodeInput[0] == table[iTableIMIndex].cPinyin && table[iTableIMIndex].bUsePY) {
		if (iCodeInputCount != (MAX_PY_LENGTH * 5 + 1)) {
		    strCodeInput[iCodeInputCount++] = (char) iKey;
		    strCodeInput[iCodeInputCount] = '\0';
		    retVal = TableGetCandWords (SM_FIRST);
		}
		else
		    retVal = IRV_DO_NOTHING;
	    }
	    else {
		if ((iCodeInputCount < table[iTableIMIndex].iCodeLength) || (table[iTableIMIndex].bHasPinyin && iCodeInputCount < table[iTableIMIndex].iPYCodeLength)) {
		    strCodeInput[iCodeInputCount++] = (char) iKey;
		    strCodeInput[iCodeInputCount] = '\0';

		    if (iCodeInputCount == 1 && strCodeInput[0] == table[iTableIMIndex].cPinyin && table[iTableIMIndex].bUsePY) {
			iCandWordCount = 0;
			retVal = IRV_DISPLAY_LAST;
		    }
		    else {
			char		*strTemp;
			char		*strLastFirstCand;
			CANDTYPE	 lastFirstCandType;
			RECORD		*pLastCandRecord = (RECORD *)NULL;
			

			strLastFirstCand = (char *)NULL;
			lastFirstCandType = CT_AUTOPHRASE;
			if ( iCandWordCount ) {			// to realize auto-sending HZ to client
			    strLastFirstCand = _TableGetCandWord (0,False);
			    lastFirstCandType = tableCandWord[0].flag;
			    pLastCandRecord = pCurCandRecord;
		        }
			
			retVal = TableGetCandWords (SM_FIRST);
			strTemp = GetPunc (strCodeInput[0]);
			if (IsEndKey (iKey)) {
			    if (iCodeInputCount == 1)
				return IRV_TO_PROCESS;

			    if (!iCandWordCount) {
				iCodeInputCount = 0;
				return IRV_CLEAN;
			    }

			    if (iCandWordCount == 1) {
				strcpy (strStringGet, TableGetCandWord (0));
				return IRV_GET_CANDWORDS;
			    }

			    return IRV_DISPLAY_CANDWORDS;
			}
			else if (table[iTableIMIndex].iTableAutoSendToClientWhenNone && (iCodeInputCount == (table[iTableIMIndex].iTableAutoSendToClientWhenNone + 1)) && !iCandWordCount) {
			    if ( strLastFirstCand && (lastFirstCandType!=CT_AUTOPHRASE)) {
			    	strcpy (strStringGet, strLastFirstCand);
				TableUpdateHitFrequency(pLastCandRecord);
				retVal=IRV_GET_CANDWORDS_NEXT;
			    }
			    else 
			    	retVal = IRV_DISPLAY_CANDWORDS;
			    iCodeInputCount = 1;
			    strCodeInput[0] = iKey;
			    strCodeInput[1] = '\0';
			    TableGetCandWords (SM_FIRST);
			}
			else if (table[iTableIMIndex].iTableAutoSendToClient && (iCodeInputCount >= table[iTableIMIndex].iTableAutoSendToClient)) {
			    if (iCandWordCount == 1 && (tableCandWord[0].flag != CT_AUTOPHRASE || (tableCandWord[0].flag == CT_AUTOPHRASE && !table[iTableIMIndex].iSaveAutoPhraseAfter))) {	//如果只有一个候选词，则送到客户程序中
			    	retVal = IRV_DO_NOTHING;
				if (tableCandWord[0].flag == CT_NORMAL) {
				    if (tableCandWord[0].candWord.record->bPinyin)
					retVal = IRV_DISPLAY_CANDWORDS;
				}

				if (retVal != IRV_DISPLAY_CANDWORDS) {
				    strcpy (strStringGet, TableGetCandWord (0));
				    iCandWordCount = 0;
				    if (bIsInLegend)
					retVal = IRV_GET_LEGEND;
				    else
					retVal = IRV_GET_CANDWORDS;
				}
			    }
			}
			else if ((iCodeInputCount == 1) && strTemp && !iCandWordCount) {	//如果第一个字母是标点，并且没有候选字/词，则当做标点处理──适用于二笔这样的输入法
			    strcpy (strStringGet, strTemp);
			    retVal = IRV_PUNC;
			}
		    }
		}
		else {
		    if (table[iTableIMIndex].iTableAutoSendToClient) {
			if (iCandWordCount) {
			    if (tableCandWord[0].flag != CT_AUTOPHRASE) {
				strcpy (strStringGet, TableGetCandWord (0));
				retVal = IRV_GET_CANDWORDS_NEXT;
			    }
			    else
				retVal = IRV_DISPLAY_CANDWORDS;
			}
			else
			    retVal = IRV_DISPLAY_CANDWORDS;

			iCodeInputCount = 1;
			strCodeInput[0] = iKey;
			strCodeInput[1] = '\0';
			bIsInLegend = False;

			if (retVal != IRV_DISPLAY_CANDWORDS)
			    TableGetCandWords (SM_FIRST);
		    }
		    else
			retVal = IRV_DO_NOTHING;
		}
	    }
	}
    }
    else {
	if (bIsTableAddPhrase) {
	    switch (iKey) {
	    case LEFT:
		if (iTableNewPhraseHZCount < iHZLastInputCount && iTableNewPhraseHZCount < PHRASE_MAX_LENGTH) {
		    iTableNewPhraseHZCount++;
		    TableCreateNewPhrase ();
		}
		break;
	    case RIGHT:
		if (iTableNewPhraseHZCount > 2) {
		    iTableNewPhraseHZCount--;
		    TableCreateNewPhrase ();
		}
		break;
	    case ENTER:
		if (!bCanntFindCode)
		    TableInsertPhrase (messageDown[1].strMsg, messageDown[0].strMsg);
	    case ESC:
		bIsTableAddPhrase = False;
		bIsDoInputOnly = False;
		return IRV_CLEAN;
	    default:
		return IRV_DO_NOTHING;
	    }

	    return IRV_DISPLAY_MESSAGE;
	}
	if (IsHotKey (iKey, hkTableAddPhrase)) {
	    if (!bIsTableAddPhrase) {
		if (iHZLastInputCount < 2 || !table[iTableIMIndex].bRule)	//词组最少为两个汉字
		    return IRV_DO_NOTHING;

		iTableNewPhraseHZCount = 2;
		bIsTableAddPhrase = True;
		bIsDoInputOnly = True;
		bShowCursor = False;

		uMessageUp = 1;
		strcpy (messageUp[0].strMsg, "左/右键增加/减少，ENTER确定，ESC取消");
		messageUp[0].type = MSG_TIPS;

		uMessageDown = 2;
		messageDown[0].type = MSG_FIRSTCAND;
		messageDown[1].type = MSG_CODE;

		TableCreateNewPhrase ();
		retVal = IRV_DISPLAY_MESSAGE;
	    }
	    else
		retVal = IRV_TO_PROCESS;

	    return retVal;
	}
	else if (IsHotKey (iKey, hkGetPY)) {
	    char            strPY[100];

	    //如果拼音单字字库没有读入，则读入它
	    if (!bPYBaseDictLoaded)
		LoadPYBaseDict ();

	    //如果刚刚输入的是个词组，刚不查拼音
	    if (strlen (strStringGet) != 2)
		return IRV_DO_NOTHING;

	    iCodeInputCount = 0;
	    uMessageUp = 1;
	    strcpy (messageUp[0].strMsg, strStringGet);
	    messageUp[0].type = MSG_INPUT;
	    uMessageDown = 2;
	    strcpy (messageDown[0].strMsg, "读音：");
	    messageDown[0].type = MSG_CODE;
	    PYGetPYByHZ (strStringGet, strPY);
	    strcpy (messageDown[1].strMsg, (strPY[0]) ? strPY : "无法查到该字读音");
	    messageDown[1].type = MSG_TIPS;
	    bShowCursor = False;

	    return IRV_DISPLAY_MESSAGE;
	}

	if (!iCodeInputCount && !bIsInLegend)
	    return IRV_TO_PROCESS;

	if (iKey == ESC) {
	    if (bIsTableDelPhrase || bIsTableAdjustOrder) {
		TableResetStatus ();
		retVal = IRV_DISPLAY_CANDWORDS;
	    }
	    else
		return IRV_CLEAN;
	}
	else if (iKey >= '0' && iKey <= '9') {
	    iKey -= '0';
	    if (iKey == 0)
		iKey = 10;

	    if (!bIsInLegend) {
		if (!iCandWordCount)
		    return IRV_TO_PROCESS;

		if (iKey > iCandWordCount)
		    return IRV_DO_NOTHING;
		else {
		    if (bIsTableDelPhrase) {
			TableDelPhraseByIndex (iKey);
			retVal = TableGetCandWords (SM_FIRST);
		    }
		    else if (bIsTableAdjustOrder) {
			TableAdjustOrderByIndex (iKey);
			retVal = TableGetCandWords (SM_FIRST);
		    }
		    else {
			//如果是拼音，进入快速调整字频方式
			if (strcmp (strCodeInput, table[iTableIMIndex].strSymbol) && strCodeInput[0] == table[iTableIMIndex].cPinyin && table[iTableIMIndex].bUsePY)
			    PYGetCandWord (iKey - 1);
			strcpy (strStringGet, TableGetCandWord (iKey - 1));

			if (bIsInLegend)
			    retVal = IRV_GET_LEGEND;
			else
			    retVal = IRV_GET_CANDWORDS;
		    }
		}
	    }
	    else {
		strcpy (strStringGet, TableGetLegendCandWord (iKey - 1));
		retVal = IRV_GET_LEGEND;
	    }
	}
	else if (!bIsTableDelPhrase && !bIsTableAdjustOrder) {
	    if (IsHotKey (iKey, hkTableAdjustOrder)) {
		if ((iTableCandDisplayed == iCandWordCount && iCandWordCount < 2) || bIsInLegend)
		    return IRV_DO_NOTHING;

		bIsTableAdjustOrder = True;
		uMessageUp = 1;
		strcpy (messageUp[0].strMsg, "选择需要提前的词组序号，ESC取消");
		messageUp[0].type = MSG_TIPS;
		retVal = IRV_DISPLAY_MESSAGE;
	    }
	    else if (IsHotKey (iKey, hkTableDelPhrase)) {
		if (!iCandWordCount || bIsInLegend)
		    return IRV_DO_NOTHING;

		bIsTableDelPhrase = True;
		uMessageUp = 1;
		strcpy (messageUp[0].strMsg, "选择需要删除的词组序号，ESC取消");
		messageUp[0].type = MSG_TIPS;
		retVal = IRV_DISPLAY_MESSAGE;
	    }
	    else if (iKey == (XK_BackSpace & 0x00FF) || iKey == CTRL_H) {
		if (!iCodeInputCount) {
		    bIsInLegend = False;
		    return IRV_DONOT_PROCESS_CLEAN;
		}

		iCodeInputCount--;
		strCodeInput[iCodeInputCount] = '\0';

		if (iCodeInputCount == 1 && strCodeInput[0] == table[iTableIMIndex].cPinyin) {
		    iCandWordCount = 0;
		    retVal = IRV_DISPLAY_LAST;
		}
		else if (iCodeInputCount)
		    retVal = TableGetCandWords (SM_FIRST);
		else
		    retVal = IRV_CLEAN;
	    }
	    else if (iKey == ' ') {
		if (!bIsInLegend) {
		    if (!(table[iTableIMIndex].bUsePY && iCodeInputCount == 1 && strCodeInput[0] == table[iTableIMIndex].cPinyin)) {
			if (strcmp (strCodeInput, table[iTableIMIndex].strSymbol) && strCodeInput[0] == table[iTableIMIndex].cPinyin && table[iTableIMIndex].bUsePY)
			    PYGetCandWord (0);

			if (!iCandWordCount) {
			    iCodeInputCount = 0;
			    return IRV_CLEAN;
			}

			strcpy (strStringGet, TableGetCandWord (0));
		    }
		    else
			uMessageDown = 0;

		    if (bIsInLegend)
			retVal = IRV_GET_LEGEND;
		    else
			retVal = IRV_GET_CANDWORDS;

		}
		else {
		    strcpy (strStringGet, TableGetLegendCandWord (0));
		    retVal = IRV_GET_LEGEND;
		}
	    }
	    else
		return IRV_TO_PROCESS;
	}
    }

    if (!bIsInLegend) {
	if (!bIsTableDelPhrase && !bIsTableAdjustOrder) {
	    if (iCodeInputCount) {
		uMessageUp = 1;
		strcpy (messageUp[0].strMsg, strCodeInput);
		messageUp[0].type = MSG_INPUT;
	    }
	    else
		uMessageUp = 0;
	}
	else
	    bIsDoInputOnly = True;
    }

    if (bIsTableDelPhrase || bIsTableAdjustOrder || bIsInLegend)
	bShowCursor = False;
    else
	bShowCursor = True;

    return retVal;
}

char           *TableGetCandWord (int iIndex)
{
    char *str;
    
    str=_TableGetCandWord(iIndex, True);
    if (str) {
    	if (table[iTableIMIndex].bAutoPhrase && (strlen (str) == 2 || (strlen (str) > 2 && table[iTableIMIndex].bAutoPhrasePhrase)))
    	    UpdateHZLastInput (str);
	    
	if ( pCurCandRecord )
	    TableUpdateHitFrequency (pCurCandRecord);
    }
    
    return str;
}

void		TableUpdateHitFrequency (RECORD * record)
{
    record->iHit++;
    record->iIndex = ++iTableIndex;
}

/*
 * 第二个参数表示是否进入联想模式，实现自动上屏功能时，不能使用联想模式
 */
char           *_TableGetCandWord (int iIndex, Bool _bLegend)
{
    char           *pCandWord = NULL;
    
    if (!strcmp (strCodeInput, table[iTableIMIndex].strSymbol))
	return TableGetFHCandWord (iIndex);

    bIsInLegend = False;

    if (!iCandWordCount)
	return NULL;

    if (iIndex > (iCandWordCount - 1))
	iIndex = iCandWordCount - 1;

    if (tableCandWord[iIndex].flag == CT_NORMAL)
	pCurCandRecord = tableCandWord[iIndex].candWord.record;
    else
	pCurCandRecord = (RECORD *)NULL;

    iTableOrderChanged++;
    if (iTableOrderChanged == TABLE_AUTO_SAVE_AFTER)
	SaveTableDict ();

    switch (tableCandWord[iIndex].flag) {
    case CT_NORMAL:
	pCandWord = tableCandWord[iIndex].candWord.record->strHZ;
	break;
    case CT_AUTOPHRASE:
	if (table[iTableIMIndex].iSaveAutoPhraseAfter) {
	    /* 当_bLegend为False时，不应该计算自动组词的频度，因此此时实际并没有选择这个词 */
	    if (table[iTableIMIndex].iSaveAutoPhraseAfter >= tableCandWord[iIndex].candWord.autoPhrase->iSelected && _bLegend)
		tableCandWord[iIndex].candWord.autoPhrase->iSelected++;
	    if (table[iTableIMIndex].iSaveAutoPhraseAfter == tableCandWord[iIndex].candWord.autoPhrase->iSelected)	//保存自动词组
		TableInsertPhrase (tableCandWord[iIndex].candWord.autoPhrase->strCode, tableCandWord[iIndex].candWord.autoPhrase->strHZ);
	}
	pCandWord = tableCandWord[iIndex].candWord.autoPhrase->strHZ;
	break;
    case CT_PYPHRASE:
	pCandWord = tableCandWord[iIndex].candWord.strPYPhrase;
	break;
    default:
	;
    }

    if (bUseLegend && _bLegend) {
	strcpy (strTableLegendSource, pCandWord);
	TableGetLegendCandWords (SM_FIRST);
    }
    else {
	if (table[iTableIMIndex].bPromptTableCode) {
	    RECORD         *temp;

	    uMessageUp = 1;
	    strcpy (messageUp[0].strMsg, strCodeInput);
	    messageUp[0].type = MSG_INPUT;

	    strcpy (messageDown[0].strMsg, pCandWord);
	    messageDown[0].type = MSG_TIPS;
	    temp = tableSingleHZ[CalHZIndex (pCandWord)];
	    if (temp) {
		strcpy (messageDown[1].strMsg, temp->strCode);
		messageDown[1].type = MSG_CODE;
		uMessageDown = 2;
	    }
	    else
		uMessageDown = 1;
	}
	else {
	    uMessageDown = 0;
	    uMessageUp = 0;
	 //   iCodeInputCount = 0;
	}
    }

    if (strlen (pCandWord) == 2)
	lastIsSingleHZ = 1;
    else
	lastIsSingleHZ = 0;

    return pCandWord;
}

INPUT_RETURN_VALUE TableGetPinyinCandWords (SEARCH_MODE mode)
{
    int             i;

    if (mode == SM_FIRST) {
	strcpy (strFindString, strCodeInput + 1);

	DoPYInput (-1);

	strCodeInput[0] = table[iTableIMIndex].cPinyin;
	strCodeInput[1] = '\0';

	strcat (strCodeInput, strFindString);
	iCodeInputCount = strlen (strCodeInput);
    }
    else
	PYGetCandWords (mode);

    //下面将拼音的候选字表转换为码表输入法的样式
    for (i = 0; i < iCandWordCount; i++) {
	tableCandWord[i].flag = CT_PYPHRASE;
	PYGetCandText (i, tableCandWord[i].candWord.strPYPhrase);
    }

    return IRV_DISPLAY_CANDWORDS;
}

INPUT_RETURN_VALUE TableGetCandWords (SEARCH_MODE mode)
{
    int             i, iTemp;
    char            strTemp[3], *pstr;
    RECORD         *recTemp;

    if (strCodeInput[0] == '\0')
	return IRV_TO_PROCESS;

    if (bIsInLegend)
	return TableGetLegendCandWords (mode);
    if (!strcmp (strCodeInput, table[iTableIMIndex].strSymbol))
	return TableGetFHCandWords (mode);

    if (strCodeInput[0] == table[iTableIMIndex].cPinyin && table[iTableIMIndex].bUsePY)
	TableGetPinyinCandWords (mode);
    else {
	if (mode == SM_FIRST) {
	    iCandWordCount = 0;
	    iTableCandDisplayed = 0;
	    iTableTotalCandCount = 0;
	    iCurrentCandPage = 0;
	    iCandPageCount = 0;
	    TableResetFlags ();
	    if (TableFindFirstMatchCode () == -1 && !iAutoPhrase) {
		uMessageDown = 0;
		return IRV_DISPLAY_CANDWORDS;	//Not Found
	    }
	}
	else {
	    if (!iCandWordCount)
		return IRV_TO_PROCESS;

	    if (mode == SM_NEXT) {
		if (iTableCandDisplayed >= iTableTotalCandCount)
		    return IRV_DO_NOTHING;
	    }
	    else {
		if (iTableCandDisplayed == iCandWordCount)
		    return IRV_DO_NOTHING;

		iTableCandDisplayed -= iCandWordCount;
		TableSetCandWordsFlag (iCandWordCount, False);
	    }

	    TableFindFirstMatchCode ();
	}

	iCandWordCount = 0;
	if (mode == SM_PREV && table[iTableIMIndex].bRule && table[iTableIMIndex].bAutoPhrase && iCodeInputCount == table[iTableIMIndex].iCodeLength) {
	    for (i = iAutoPhrase - 1; i >= 0; i--) {
		if (!TableCompareCode (strCodeInput, autoPhrase[i].strCode) && autoPhrase[i].flag) {
		    if (TableHasPhrase (autoPhrase[i].strCode, autoPhrase[i].strHZ))
			TableAddAutoCandWord (i, mode);
		}
	    }
	}

	if (iCandWordCount < iMaxCandWord) {
	    while (currentRecord && currentRecord != recordHead) {
		if ((mode == SM_PREV) ^ (!currentRecord->flag)) {
		    if ( ((strlen(currentRecord->strHZ)/2)<=table[iTableIMIndex].iMaxPhraseAllowed) || !table[iTableIMIndex].iMaxPhraseAllowed ) {
			if (!TableCompareCode (strCodeInput, currentRecord->strCode) && CheckHZCharset (currentRecord->strHZ)) {
			    if (mode == SM_FIRST)
				iTableTotalCandCount++;
			    TableAddCandWord (currentRecord, mode);
			}
		    }
		}

		currentRecord = currentRecord->next;
	    }
	}

	if (table[iTableIMIndex].bRule && table[iTableIMIndex].bAutoPhrase && mode != SM_PREV && iCodeInputCount == table[iTableIMIndex].iCodeLength) {
	    for (i = iAutoPhrase - 1; i >= 0; i--) {
		if (!TableCompareCode (strCodeInput, autoPhrase[i].strCode) && !autoPhrase[i].flag) {
		    if (TableHasPhrase (autoPhrase[i].strCode, autoPhrase[i].strHZ)) {
			if (mode == SM_FIRST)
			    iTableTotalCandCount++;
			TableAddAutoCandWord (i, mode);
		    }
		}
	    }
	}

	TableSetCandWordsFlag (iCandWordCount, True);

	if (mode != SM_PREV)
	    iTableCandDisplayed += iCandWordCount;

	//由于iCurrentCandPage和iCandPageCount用来指示是否显示上/下翻页图标，因此，此处需要设置一下
	iCurrentCandPage = (iTableCandDisplayed == iCandWordCount) ? 0 : 1;
	iCandPageCount = (iTableCandDisplayed >= iTableTotalCandCount) ? 1 : 2;
	if (iCandWordCount == iTableTotalCandCount)
	    iCandPageCount = 0;
	/* **************************************************** */
    }

    if (bPointAfterNumber) {
	strTemp[1] = '.';
	strTemp[2] = '\0';
    }
    else
	strTemp[1] = '\0';

    /* 如果用户设置了固定输入条宽度，就在此进行处理 */
    if (iFixedInputWindowWidth && !(strCodeInput[0] == table[iTableIMIndex].cPinyin && table[iTableIMIndex].bUsePY)) {
	uint            iWidth = 2 * INPUTWND_START_POS_DOWN + 1;

	if (mode == SM_PREV) {
	    for (i = (iCandWordCount - 1); i >= 0; i--) {
		strTemp[0] = i + 1 + '0';
		if (i == 9)
		    strTemp[0] = '0';

		if (HasMatchingKey ())
		    pstr = (tableCandWord[i].flag == CT_NORMAL) ? tableCandWord[i].candWord.record->strCode : tableCandWord[i].candWord.autoPhrase->strCode;
		/*else if (tableCandWord[i].flag==CT_PYPHRASE) {
		   if (strlen(tableCandWord[i].candWord.strPYPhrase)==2){
		   recTemp = tableSingleHZ[CalHZIndex (tableCandWord[i].candWord.strPYPhrase)];
		   if (!recTemp)
		   pstr=(char *)NULL;
		   else
		   pstr=recTemp->strCode;
		   }
		   else
		   pstr=(char *)NULL;
		   } */
		else
		    pstr = ((tableCandWord[i].flag == CT_NORMAL) ? tableCandWord[i].candWord.record->strCode : tableCandWord[i].candWord.autoPhrase->strCode) + iCodeInputCount;

#ifdef _USE_XFT
		if (pstr)
		    iWidth += StringWidth (pstr, xftFontEn);
		iWidth += StringWidth (strTemp, xftFontEn);
		switch (tableCandWord[i].flag) {
		case CT_NORMAL:
		    iWidth += StringWidth (tableCandWord[i].candWord.record->strHZ, xftFont);
		    break;
		case CT_AUTOPHRASE:
		    iWidth += StringWidth (tableCandWord[i].candWord.autoPhrase->strHZ, xftFont);
		    /*      case CT_PYPHRASE:
		       iWidth += StringWidth (tableCandWord[i].candWord.strPhrase, xftFont); */
		default:
		    ;
		}
		iWidth += StringWidth (" ", xftFontEn);
#else
		if (pstr)
		    iWidth += StringWidth (pstr, fontSet);
		iWidth += StringWidth (strTemp, fontSet);
		switch (tableCandWord[i].flag) {
		case CT_NORMAL:
		    iWidth += StringWidth (tableCandWord[i].candWord.record->strHZ, fontSet);
		    break;
		case CT_AUTOPHRASE:
		    iWidth += StringWidth (tableCandWord[i].candWord.autoPhrase->strHZ, fontSet);
		    /*case CT_PYPHRASE:
		       iWidth += StringWidth (tableCandWord[i].candWord.strPhrase, fontSet); */
		default:
		    ;
		}
		iWidth += StringWidth (" ", fontSet);
#endif

		if (iWidth > iFixedInputWindowWidth) {
		    if (i == 0) {
#ifdef _USE_XFT
			iWidth -= StringWidth (" ", xftFontEn);
#else
			iWidth -= StringWidth (" ", fontSet);
#endif
			if (iWidth > iFixedInputWindowWidth)
			    i = 1;
		    }
		    else
			i++;
		    break;
		}
	    }

	    if (i < 0)
		i = 0;
	    for (iWidth = 0; iWidth < i; iWidth++) {
		if (tableCandWord[iWidth].flag == CT_NORMAL)
		    tableCandWord[iWidth].candWord.record->flag = True;
		else
		    tableCandWord[iWidth].candWord.autoPhrase->flag = True;
	    }
	    for (iWidth = 0; iWidth < (iCandWordCount - i); iWidth++) {
		tableCandWord[iWidth].flag = tableCandWord[iWidth + i].flag;
		if (tableCandWord[iWidth].flag == CT_NORMAL)
		    tableCandWord[iWidth].candWord.record = tableCandWord[iWidth + i].candWord.record;
		else
		    tableCandWord[iWidth].candWord.autoPhrase = tableCandWord[iWidth + i].candWord.autoPhrase;
	    }

	    iCandWordCount -= i;
	}
	else {
	    //如果是向后翻，需要从前往后计算长度
	    iTemp = iCandWordCount;
	    for (i = 0; i < iCandWordCount; i++) {
		strTemp[0] = i + 1 + '0';
		if (i == 9)
		    strTemp[0] = '0';

		if (HasMatchingKey ())
		    pstr = (tableCandWord[i].flag == CT_NORMAL) ? tableCandWord[i].candWord.record->strCode : tableCandWord[i].candWord.autoPhrase->strCode;
		else
		    pstr = ((tableCandWord[i].flag == CT_NORMAL) ? tableCandWord[i].candWord.record->strCode : tableCandWord[i].candWord.autoPhrase->strCode) + iCodeInputCount;

#ifdef _USE_XFT
		iWidth += StringWidth (pstr, xftFontEn);
		iWidth += StringWidth (strTemp, xftFontEn);
		iWidth += StringWidth ((tableCandWord[i].flag == CT_NORMAL) ? tableCandWord[i].candWord.record->strHZ : tableCandWord[i].candWord.autoPhrase->strHZ, xftFont);
		iWidth += StringWidth (" ", xftFontEn);
#else
		iWidth += StringWidth (pstr, fontSet);
		iWidth += StringWidth (strTemp, fontSet);
		iWidth += StringWidth ((tableCandWord[i].flag == CT_NORMAL) ? tableCandWord[i].candWord.record->strHZ : tableCandWord[i].candWord.autoPhrase->strHZ, fontSet);
		iWidth += StringWidth (" ", fontSet);
#endif
		if (iWidth > iFixedInputWindowWidth) {
		    if (i == (iCandWordCount - 1)) {
#ifdef _USE_XFT
			iWidth -= StringWidth (" ", xftFontEn);
#else
			iWidth -= StringWidth (" ", fontSet);
#endif
			if (iWidth <= iFixedInputWindowWidth)
			    i = iCandWordCount;
		    }
		    break;
		}
	    }

	    for (iWidth = i; iWidth < iCandWordCount; iWidth++) {
		if (tableCandWord[iWidth].flag == CT_NORMAL)
		    tableCandWord[iWidth].candWord.record->flag = False;
		else
		    tableCandWord[iWidth].candWord.autoPhrase->flag = False;
	    }

	    iCandWordCount = i;
	    iTableCandDisplayed -= iTemp - i;
	}
    }

    uMessageDown = 0;
    for (i = 0; i < iCandWordCount; i++) {
	strTemp[0] = i + 1 + '0';
	if (i == 9)
	    strTemp[0] = '0';
	strcpy (messageDown[uMessageDown].strMsg, strTemp);
	messageDown[uMessageDown++].type = MSG_INDEX;

	switch (tableCandWord[i].flag) {
	case CT_NORMAL:
	    strcpy (messageDown[uMessageDown].strMsg, tableCandWord[i].candWord.record->strHZ);
	    break;
	case CT_AUTOPHRASE:
	    strcpy (messageDown[uMessageDown].strMsg, tableCandWord[i].candWord.autoPhrase->strHZ);
	    break;
	case CT_PYPHRASE:
	    strcpy (messageDown[uMessageDown].strMsg, tableCandWord[i].candWord.strPYPhrase);
	    break;
	default:
	    ;
	}

	if (tableCandWord[i].flag == CT_AUTOPHRASE)
	    messageDown[uMessageDown++].type = MSG_TIPS;
	else
	    messageDown[uMessageDown++].type = ((i == 0) ? MSG_FIRSTCAND : MSG_OTHER);

	if (tableCandWord[i].flag == CT_PYPHRASE) {
	    if (strlen (tableCandWord[i].candWord.strPYPhrase) == 2) {
		recTemp = tableSingleHZ[CalHZIndex (tableCandWord[i].candWord.strPYPhrase)];

		if (!recTemp)
		    pstr = (char *) NULL;
		else
		    pstr = recTemp->strCode;
	    }
	    else
		pstr = (char *) NULL;
	}
	else if ((tableCandWord[i].flag == CT_NORMAL) && (tableCandWord[i].candWord.record->bPinyin)) {
	    if (strlen (tableCandWord[i].candWord.record->strHZ) == 2) {
		recTemp = tableSingleHZ[CalHZIndex (tableCandWord[i].candWord.record->strHZ)];
		if (!recTemp)
		    pstr = (char *) NULL;
		else
		    pstr = recTemp->strCode;
	    }
	    else
		pstr = (char *) NULL;
	}
	else if (HasMatchingKey ())
	    pstr = (tableCandWord[i].flag == CT_NORMAL) ? tableCandWord[i].candWord.record->strCode : tableCandWord[i].candWord.autoPhrase->strCode;
	else
	    pstr = ((tableCandWord[i].flag == CT_NORMAL) ? tableCandWord[i].candWord.record->strCode : tableCandWord[i].candWord.autoPhrase->strCode) + iCodeInputCount;

	if (pstr)
	    strcpy (messageDown[uMessageDown].strMsg, pstr);
	else
	    messageDown[uMessageDown].strMsg[0] = '\0';

	if (i != (iCandWordCount - 1)) {
	    strcat (messageDown[uMessageDown].strMsg, " ");
	}
	messageDown[uMessageDown++].type = MSG_CODE;
    }

    return IRV_DISPLAY_CANDWORDS;
}

void TableAddAutoCandWord (INT16 which, SEARCH_MODE mode)
{
    int             i, j;

    if (mode == SM_PREV) {
	if (iCandWordCount == iMaxCandWord) {
	    i = iCandWordCount - 1;
	    for (j = 0; j < iCandWordCount - 1; j++)
		tableCandWord[j].candWord.autoPhrase = tableCandWord[j + 1].candWord.autoPhrase;
	}
	else
	    i = iCandWordCount++;
	tableCandWord[i].flag = CT_AUTOPHRASE;
	tableCandWord[i].candWord.autoPhrase = &autoPhrase[which];
    }
    else {
	if (iCandWordCount == iMaxCandWord)
	    return;

	tableCandWord[iCandWordCount].flag = CT_AUTOPHRASE;
	tableCandWord[iCandWordCount++].candWord.autoPhrase = &autoPhrase[which];
    }
}

void TableAddCandWord (RECORD * record, SEARCH_MODE mode)
{
    int             i = 0, j;

    switch (table[iTableIMIndex].tableOrder) {
    case AD_NO:
	if (mode == SM_PREV) {
	    if (iCandWordCount == iMaxCandWord)
		i = iCandWordCount - 1;
	    else {
		for (i = 0; i < iCandWordCount; i++) {
		    if (tableCandWord[i].flag != CT_NORMAL)
			break;
		}
	    }
	}
	else {
	    if (iCandWordCount == iMaxCandWord)
		return;
	    tableCandWord[iCandWordCount].flag = CT_NORMAL;
	    tableCandWord[iCandWordCount++].candWord.record = record;
	    return;
	}
	break;
    case AD_FREQ:
	if (mode == SM_PREV) {
	    for (i = iCandWordCount - 1; i >= 0; i--) {
		if (tableCandWord[i].flag == CT_NORMAL) {
		    if (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) < 0 || (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) == 0 && tableCandWord[i].candWord.record->iHit >= record->iHit))
			break;
		}
	    }

	    if (iCandWordCount == iMaxCandWord) {
		if (i < 0)
		    return;
	    }
	    else
		i++;
	}
	else {
	    for (; i < iCandWordCount; i++) {
		if (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) > 0 || (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) == 0 && tableCandWord[i].candWord.record->iHit < record->iHit))
		    break;
	    }
	    if (i == iMaxCandWord)
		return;
	}
	break;

    case AD_FAST:
	if (mode == SM_PREV) {
	    for (i = iCandWordCount - 1; i >= 0; i--) {
		if (tableCandWord[i].flag == CT_NORMAL) {
		    if (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) < 0 || (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) == 0 && tableCandWord[i].candWord.record->iIndex >= record->iIndex))
			break;
		}
	    }

	    if (iCandWordCount == iMaxCandWord) {
		if (i < 0)
		    return;
	    }
	    else
		i++;
	}
	else {
	    for (i = 0; i < iCandWordCount; i++) {
		if (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) > 0 || (strcmp (tableCandWord[i].candWord.record->strCode, record->strCode) == 0 && tableCandWord[i].candWord.record->iIndex < record->iIndex))
		    break;
	    }

	    if (i == iMaxCandWord)
		return;
	}
	break;
    }

    if (mode == SM_PREV) {
	if (iCandWordCount == iMaxCandWord) {
	    for (j = 0; j < i; j++) {
		tableCandWord[j].flag = tableCandWord[j + 1].flag;
		if (tableCandWord[j].flag == CT_NORMAL)
		    tableCandWord[j].candWord.record = tableCandWord[j + 1].candWord.record;
		else
		    tableCandWord[j].candWord.autoPhrase = tableCandWord[j + 1].candWord.autoPhrase;
	    }
	}
	else {
	    for (j = iCandWordCount; j > i; j--) {
		tableCandWord[j].flag = tableCandWord[j - 1].flag;
		if (tableCandWord[j].flag == CT_NORMAL)
		    tableCandWord[j].candWord.record = tableCandWord[j - 1].candWord.record;
		else
		    tableCandWord[j].candWord.autoPhrase = tableCandWord[j - 1].candWord.autoPhrase;
	    }
	}
    }
    else {
	j = iCandWordCount;
	if (iCandWordCount == iMaxCandWord)
	    j--;
	for (; j > i; j--) {
	    tableCandWord[j].flag = tableCandWord[j - 1].flag;
	    if (tableCandWord[j].flag == CT_NORMAL)
		tableCandWord[j].candWord.record = tableCandWord[j - 1].candWord.record;
	    else
		tableCandWord[j].candWord.autoPhrase = tableCandWord[j - 1].candWord.autoPhrase;
	}
    }

    tableCandWord[i].flag = CT_NORMAL;
    tableCandWord[i].candWord.record = record;

    if (iCandWordCount != iMaxCandWord)
	iCandWordCount++;
}

void TableResetFlags (void)
{
    RECORD         *record = recordHead->next;
    INT16           i;

    while (record != recordHead) {
	record->flag = False;
	record = record->next;
    }
    for (i = 0; i < iAutoPhrase; i++)
	autoPhrase[i].flag = False;
}

void TableSetCandWordsFlag (int iCount, Bool flag)
{
    int             i;

    for (i = 0; i < iCount; i++) {
	if (tableCandWord[i].flag == CT_NORMAL)
	    tableCandWord[i].candWord.record->flag = flag;
	else
	    tableCandWord[i].candWord.autoPhrase->flag = flag;
    }
}
Bool HasMatchingKey (void)
{
    char           *str;

    str = strCodeInput;
    while (*str) {
	if (*str++ == table[iTableIMIndex].cMatchingKey)
	    return True;
    }
    return False;
}

int TableCompareCode (char *strUser, char *strDict)
{
    int             i;

    for (i = 0; i < strlen (strUser); i++) {
	if (!strDict[i])
	    return strUser[i];
	if (strUser[i] != table[iTableIMIndex].cMatchingKey || !table[iTableIMIndex].bUseMatchingKey) {
	    if (strUser[i] != strDict[i])
		return (strUser[i] - strDict[i]);
	}
    }
    if (table[iTableIMIndex].bTableExactMatch) {
	if (strlen (strUser) != strlen (strDict))
	    return -999;	//随意的一个值
    }

    return 0;
}

int TableFindFirstMatchCode (void)
{
    int             i = 0;

    if (!recordHead)
	return -1;

    if (table[iTableIMIndex].bUseMatchingKey && (strCodeInput[0] == table[iTableIMIndex].cMatchingKey))
	i = 0;
    else {
	while (strCodeInput[0] != recordIndex[i].cCode) {
	    if (!recordIndex[i].cCode)
		break;
	    i++;
	}
    }
    currentRecord = recordIndex[i].record;
    if (!currentRecord)
	return -1;

    while (currentRecord != recordHead) {
	if (!TableCompareCode (strCodeInput, currentRecord->strCode)) {
	    if (CheckHZCharset (currentRecord->strHZ))
		return i;
	}
	currentRecord = currentRecord->next;
	i++;
    }

    return -1;			//Not found
}

/*
 * 反查编码
 * bMode=True表示用于组词，此时不查一、二级简码。但如果只有二级简码时返回二级简码，不查一级简码
 */
/*RECORD         *TableFindCode (char *strHZ, Bool bMode)
{
    RECORD         *recShort = NULL;	//记录二级简码的位置
    int             i;

    for (i = 0; i < iSingleHZCount; i++) {
	if (!strcmp (tableSingleHZ[i]->strHZ, strHZ) && !IsIgnoreChar (tableSingleHZ[i]->strCode[0])) {
	    if (!bMode)
		return tableSingleHZ[i];

	    if (strlen (tableSingleHZ[i]->strCode) == 2)
		recShort = tableSingleHZ[i];
	    else if (strlen (tableSingleHZ[i]->strCode) > 2)
		return tableSingleHZ[i];
	}
    }

    return recShort;
}*/

/*
 * 根据序号调整词组顺序，序号从1开始
 * 将指定的字/词调整到同样编码的最前面
 */
void TableAdjustOrderByIndex (int iIndex)
{
    RECORD         *recTemp;
    int             iTemp;

    if (tableCandWord[iIndex - 1].flag != CT_NORMAL)
	return;

    recTemp = tableCandWord[iIndex - 1].candWord.record;
    while (!strcmp (recTemp->strCode, recTemp->prev->strCode))
	recTemp = recTemp->prev;
    if (recTemp == tableCandWord[iIndex - 1].candWord.record)	//说明已经是第一个
	return;

    //将指定的字/词放到recTemp前
    tableCandWord[iIndex - 1].candWord.record->prev->next = tableCandWord[iIndex - 1].candWord.record->next;
    tableCandWord[iIndex - 1].candWord.record->next->prev = tableCandWord[iIndex - 1].candWord.record->prev;
    recTemp->prev->next = tableCandWord[iIndex - 1].candWord.record;
    tableCandWord[iIndex - 1].candWord.record->prev = recTemp->prev;
    recTemp->prev = tableCandWord[iIndex - 1].candWord.record;
    tableCandWord[iIndex - 1].candWord.record->next = recTemp;

    iTableChanged++;

    //需要的话，更新索引
    if (tableCandWord[iIndex - 1].candWord.record->strCode[1] == '\0') {
	for (iTemp = 0; iTemp < strlen (table[iTableIMIndex].strInputCode); iTemp++) {
	    if (recordIndex[iTemp].cCode == tableCandWord[iIndex - 1].candWord.record->strCode[0]) {
		recordIndex[iTemp].record = tableCandWord[iIndex - 1].candWord.record;
		break;
	    }
	}
    }
    if (iTableChanged == 5)
	SaveTableDict ();
}

/*
 * 根据序号删除词组，序号从1开始
 */
void TableDelPhraseByIndex (int iIndex)
{
    if (tableCandWord[iIndex - 1].flag != CT_NORMAL)
	return;

    if (strlen (tableCandWord[iIndex - 1].candWord.record->strHZ) <= 2)
	return;

    TableDelPhrase (tableCandWord[iIndex - 1].candWord.record);
}

/*
 * 根据字串删除词组
 */ void TableDelPhraseByHZ (char *strHZ)
{
    RECORD         *recTemp;

    recTemp = TableFindPhrase (strHZ);
    if (recTemp)
	TableDelPhrase (recTemp);
}

void TableDelPhrase (RECORD * record)
{
    record->prev->next = record->next;
    record->next->prev = record->prev;

    free (record->strCode);
    free (record->strHZ);
    free (record);

    table[iTableIMIndex].iRecordCount--;

    SaveTableDict ();
}

/*
 *判断某个词组是不是已经在词库中,有返回NULL，无返回插入点
 */
RECORD         *TableHasPhrase (char *strCode, char *strHZ)
{
    RECORD         *recTemp;
    int             i;

    i = 0;
    while (strCode[0] != recordIndex[i].cCode)
	i++;

    recTemp = recordIndex[i].record;
    while (recTemp != recordHead) {
	if (!recTemp->bPinyin) {
	    if (strcmp (recTemp->strCode, strCode) > 0)
	        break;
	    else if (!strcmp (recTemp->strCode, strCode)) {
	        if (!strcmp (recTemp->strHZ, strHZ))	//该词组已经在词库中
		    return NULL;
	    }
        }
	recTemp = recTemp->next;
    }

    return recTemp;
}

/*
 *根据字串判断词库中是否有某个字/词，注意该函数会忽略拼音词组
 */
RECORD         *TableFindPhrase (char *strHZ)
{
    RECORD         *recTemp;
    char            strTemp[3];
    int             i;

    //首先，先查找第一个汉字的编码
    strTemp[0] = strHZ[0];
    strTemp[1] = strHZ[1];
    strTemp[2] = '\0';

    recTemp = tableSingleHZ[CalHZIndex (strTemp)];
    if (!recTemp)
	return (RECORD *) NULL;

    //然后根据该编码找到检索的起始点
    i = 0;
    while (recTemp->strCode[0] != recordIndex[i].cCode)
	i++;

    recTemp = recordIndex[i].record;
    while (recTemp != recordHead) {	    
	if (recTemp->strCode[0] != recordIndex[i].cCode)
	    break;
	if (!strcmp (recTemp->strHZ, strHZ)) {
	    if (recTemp->bPinyin)
	        break;
	    return recTemp;
        }

	recTemp = recTemp->next;
    }

    return (RECORD *) NULL;
}

void TableInsertPhrase (char *strCode, char *strHZ)
{
    RECORD         *insertPoint, *dictNew;

    insertPoint = TableHasPhrase (strCode, strHZ);

    if (!insertPoint)
	return;

    dictNew = (RECORD *) malloc (sizeof (RECORD));
    dictNew->strCode = (char *) malloc (sizeof (char) * (table[iTableIMIndex].iCodeLength + 1));
    strcpy (dictNew->strCode, strCode);
    dictNew->strHZ = (char *) malloc (sizeof (char) * (strlen (strHZ) + 1));
    strcpy (dictNew->strHZ, strHZ);
    dictNew->iHit = 0;
    dictNew->iIndex = iTableIndex;

    dictNew->prev = insertPoint->prev;
    insertPoint->prev->next = dictNew;
    insertPoint->prev = dictNew;
    dictNew->next = insertPoint;

    table[iTableIMIndex].iRecordCount++;

    SaveTableDict ();
}

void TableCreateNewPhrase (void)
{
    int             i;

    messageDown[0].strMsg[0] = '\0';
    for (i = iTableNewPhraseHZCount; i > 0; i--)
	strcat (messageDown[0].strMsg, hzLastInput[iHZLastInputCount - i].strHZ);

    TableCreatePhraseCode (messageDown[0].strMsg);

    if (!bCanntFindCode)
	strcpy (messageDown[1].strMsg, strNewPhraseCode);
    else
	strcpy (messageDown[1].strMsg, "????");
}

void TableCreatePhraseCode (char *strHZ)
{
    unsigned char   i;
    unsigned char   i1, i2;
    size_t          iLen;
    char            strTemp[3];
    RECORD         *recTemp;

    strTemp[2] = '\0';
    bCanntFindCode = False;
    iLen = strlen (strHZ) / 2;
    if (iLen >= table[iTableIMIndex].iCodeLength) {
	i2 = table[iTableIMIndex].iCodeLength;
	i1 = 1;
    }
    else {
	i2 = iLen;
	i1 = 0;
    }

    for (i = 0; i < table[iTableIMIndex].iCodeLength - 1; i++) {
	if (table[iTableIMIndex].rule[i].iWords == i2 && table[iTableIMIndex].rule[i].iFlag == i1)
	    break;
    }

    for (i1 = 0; i1 < table[iTableIMIndex].iCodeLength; i1++) {
	if (table[iTableIMIndex].rule[i].rule[i1].iFlag) {
	    strTemp[0] = strHZ[(table[iTableIMIndex].rule[i].rule[i1].iWhich - 1) * 2];
	    strTemp[1] = strHZ[(table[iTableIMIndex].rule[i].rule[i1].iWhich - 1) * 2 + 1];
	}
	else {
	    strTemp[0] = strHZ[(iLen - table[iTableIMIndex].rule[i].rule[i1].iWhich) * 2];
	    strTemp[1] = strHZ[(iLen - table[iTableIMIndex].rule[i].rule[i1].iWhich) * 2 + 1];
	}

	recTemp = tableSingleHZ[CalHZIndex (strTemp)];

	if (!recTemp) {
	    bCanntFindCode = True;
	    break;
	}

	strNewPhraseCode[i1] = recTemp->strCode[table[iTableIMIndex].rule[i].rule[i1].iIndex - 1];
    }
}

/*
 * 获取联想候选字列表
 */
INPUT_RETURN_VALUE TableGetLegendCandWords (SEARCH_MODE mode)
{
    char            strTemp[3];
    int             iLength;
    int             i;
    RECORD         *tableLegend = NULL;
    unsigned int    iTableTotalLengendCandCount = 0;

    if (!strTableLegendSource[0])
	return IRV_TO_PROCESS;

    iLength = strlen (strTableLegendSource);
    if (mode == SM_FIRST) {
	iCurrentLegendCandPage = 0;
	iLegendCandPageCount = 0;
	TableResetFlags();
    }
    else {
	if (!iLegendCandPageCount)
	    return IRV_TO_PROCESS;

	if (mode == SM_NEXT) {
	    if (iCurrentLegendCandPage == iLegendCandPageCount)
		return IRV_DO_NOTHING;

	    iCurrentLegendCandPage++;
	}
	else {
	    if (!iCurrentLegendCandPage)
		return IRV_DO_NOTHING;

	    TableSetCandWordsFlag (iLegendCandWordCount, False);
	    iCurrentLegendCandPage--;
	}
    }

    iLegendCandWordCount = 0;
    tableLegend = recordHead->next;
	
    while (tableLegend != recordHead) {
	if (((mode == SM_PREV) ^ (!tableLegend->flag)) && ((iLength + 2) == strlen (tableLegend->strHZ))) {
	    if (!strncmp (tableLegend->strHZ, strTableLegendSource, iLength) && tableLegend->strHZ[iLength] && CheckHZCharset (tableLegend->strHZ)) {
		if (mode == SM_FIRST)
		    iTableTotalLengendCandCount++;
	
		TableAddLegendCandWord (tableLegend, mode);
	    }
	}

	tableLegend = tableLegend->next;
    }

    TableSetCandWordsFlag (iLegendCandWordCount, True);

    if (mode == SM_FIRST && bDisablePagingInLegend)
	iLegendCandPageCount = iTableTotalLengendCandCount / iMaxCandWord - ((iTableTotalLengendCandCount % iMaxCandWord) ? 0 : 1);

    uMessageUp = 2;
    strcpy (messageUp[0].strMsg, "联想：");
    messageUp[0].type = MSG_TIPS;
    strcpy (messageUp[1].strMsg, strTableLegendSource);
    messageUp[1].type = MSG_INPUT;

    if (bPointAfterNumber) {
	strTemp[1] = '.';
	strTemp[2] = '\0';
    }
    else
	strTemp[1] = '\0';

    uMessageDown = 0;
    for (i = 0; i < iLegendCandWordCount; i++) {
	if (i == 9)
	    strTemp[0] = '0';
	else
	    strTemp[0] = i + 1 + '0';
	strcpy (messageDown[uMessageDown].strMsg, strTemp);
	messageDown[uMessageDown++].type = MSG_INDEX;

	strcpy (messageDown[uMessageDown].strMsg, tableCandWord[i].candWord.record->strHZ + strlen (strTableLegendSource));
	if (i != (iLegendCandWordCount - 1)) {
	    strcat (messageDown[uMessageDown].strMsg, " ");
	}
	messageDown[uMessageDown++].type = ((i == 0) ? MSG_FIRSTCAND : MSG_OTHER);
    }

    bIsInLegend = (iLegendCandWordCount != 0);

    return IRV_DISPLAY_CANDWORDS;
}

void TableAddLegendCandWord (RECORD * record, SEARCH_MODE mode)
{
    int             i, j;
    
    if (mode == SM_PREV) {
	for (i = iLegendCandWordCount - 1; i >= 0; i--) {
	    if (tableCandWord[i].candWord.record->iHit >= record->iHit)
		break;
	}

	if (iLegendCandWordCount == iMaxCandWord) {
	    if (i < 0)
		return;
	}
	else
	    i++;
    }
    else {
	for (i = 0; i < iLegendCandWordCount; i++) {
	    if (tableCandWord[i].candWord.record->iHit < record->iHit)
		break;
	}
	
	if (i == iMaxCandWord)
	    return;
    }

    if (mode == SM_PREV) {
	if (iLegendCandWordCount == iMaxCandWord) {
	    for (j = 0; j < i; j++)
		tableCandWord[j].candWord.record = tableCandWord[j + 1].candWord.record;
	}
	else {
	    for (j = iLegendCandWordCount; j > i; j--)
		tableCandWord[j].candWord.record = tableCandWord[j - 1].candWord.record;
	}
    }
    else {
	j = iLegendCandWordCount;
	if (iLegendCandWordCount == iMaxCandWord)
	    j--;
	for (; j > i; j--)
	    tableCandWord[j].candWord.record = tableCandWord[j - 1].candWord.record;
    }
    
    tableCandWord[i].flag = CT_NORMAL;
    tableCandWord[i].candWord.record = record;

    if (iLegendCandWordCount != iMaxCandWord)
	iLegendCandWordCount++;
}

char           *TableGetLegendCandWord (int iIndex)
{
    if (iLegendCandWordCount) {
	if (iIndex > (iLegendCandWordCount - 1))
	    iIndex = iLegendCandWordCount - 1;

	tableCandWord[iIndex].candWord.record->iHit++;
	strcpy (strTableLegendSource, tableCandWord[iIndex].candWord.record->strHZ + strlen (strTableLegendSource));
	TableGetLegendCandWords (SM_FIRST);

	return strTableLegendSource;
    }
    return NULL;
}

INPUT_RETURN_VALUE TableGetFHCandWords (SEARCH_MODE mode)
{
    char            strTemp[3];
    int             i;

    if (!iFH)
	return IRV_DISPLAY_MESSAGE;

    if (bPointAfterNumber) {
	strTemp[1] = '.';
	strTemp[2] = '\0';
    }
    else
	strTemp[1] = '\0';

    uMessageDown = 0;

    if (mode == SM_FIRST) {
	iCandPageCount = iFH / iMaxCandWord - ((iFH % iMaxCandWord) ? 0 : 1);
	iCurrentCandPage = 0;
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
	}
    }

    for (i = 0; i < iMaxCandWord; i++) {
	strTemp[0] = i + 1 + '0';
	if (i == 9)
	    strTemp[0] = '0';
	strcpy (messageDown[uMessageDown].strMsg, strTemp);
	messageDown[uMessageDown++].type = MSG_INDEX;
	strcpy (messageDown[uMessageDown].strMsg, fh[iCurrentCandPage * iMaxCandWord + i].strFH);
	if (i != (iMaxCandWord - 1)) {
	    strcat (messageDown[uMessageDown].strMsg, " ");
	}
	messageDown[uMessageDown++].type = ((i == 0) ? MSG_FIRSTCAND : MSG_OTHER);
	if ((iCurrentCandPage * iMaxCandWord + i) >= (iFH - 1)) {
	    i++;
	    break;
	}
    }

    iCandWordCount = i;
    return IRV_DISPLAY_CANDWORDS;
}

char           *TableGetFHCandWord (int iIndex)
{
    uMessageDown = 0;

    if (iCandWordCount) {
	if (iIndex > (iCandWordCount - 1))
	    iIndex = iCandWordCount - 1;

	return fh[iCurrentCandPage * iMaxCandWord + iIndex].strFH;
    }
    return NULL;
}

Bool TablePhraseTips (void)
{
    RECORD         *recTemp = NULL;
    char            strTemp[PHRASE_MAX_LENGTH * 2 + 1] = "\0";
    INT16           i, j;

    if (!recordHead)
	return False;

    //如果最近输入了一个词组，这个工作就不需要了
    if (lastIsSingleHZ != 1)
	return False;

    j = (iHZLastInputCount > PHRASE_MAX_LENGTH) ? iHZLastInputCount - PHRASE_MAX_LENGTH : 0;
    for (i = j; i < iHZLastInputCount; i++)
	strcat (strTemp, hzLastInput[i].strHZ);
    //如果只有一个汉字，这个工作也不需要了
    if (strlen (strTemp) < 4)
	return False;

    //首先要判断是不是已经在词库中
    for (i = 0; i < (iHZLastInputCount - j - 1); i++) {
	recTemp = TableFindPhrase (strTemp + i * 2);
	if (recTemp) {
	    strcpy (messageUp[0].strMsg, "词库中有词组 ");
	    messageUp[0].type = MSG_TIPS;
	    strcpy (messageUp[1].strMsg, strTemp + i * 2);
	    messageUp[1].type = MSG_INPUT;
	    uMessageUp = 2;

	    strcpy (messageDown[0].strMsg, "编码为 ");
	    messageDown[0].type = MSG_FIRSTCAND;
	    strcpy (messageDown[1].strMsg, recTemp->strCode);
	    messageDown[1].type = MSG_CODE;
	    strcpy (messageDown[2].strMsg, " ^DEL删除");
	    messageDown[2].type = MSG_TIPS;
	    uMessageDown = 3;
	    bTablePhraseTips = True;
	    bShowCursor = False;

	    return True;
	}
    }

    return False;
}

void TableCreateAutoPhrase (INT8 iCount)
{
    char            *strHZ;
    INT16           i, j, k;

    if (!autoPhrase)
	return;

    strHZ=(char *)malloc((table[iTableIMIndex].iAutoPhrase * 2 + 1)*sizeof(char));
    /*
     * 为了提高效率，此处只重新生成新录入字构成的词组
     */
    j = iHZLastInputCount - table[iTableIMIndex].iAutoPhrase - iCount;
    if (j < 0)
	j = 0;

    for (; j < iHZLastInputCount - 1; j++) {
	for (i = table[iTableIMIndex].iAutoPhrase; i >= 2; i--) {
	    if ((j + i - 1) > iHZLastInputCount)
		continue;

	    strcpy (strHZ, hzLastInput[j].strHZ);
	    for (k = 1; k < i; k++)
		strcat (strHZ, hzLastInput[j + k].strHZ);

	    //再去掉重复的词组
	    for (k = 0; k < iAutoPhrase; k++) {
		if (!strcmp (autoPhrase[k].strHZ, strHZ))
		    goto _next;
	    }
	    //然后去掉系统中已经有的词组
	    if (TableFindPhrase (strHZ))
	    	goto _next;

	    TableCreatePhraseCode (strHZ);
	    if (iAutoPhrase != AUTO_PHRASE_COUNT) {
		autoPhrase[iAutoPhrase].flag = False;
		strcpy (autoPhrase[iAutoPhrase].strCode, strNewPhraseCode);
		strcpy (autoPhrase[iAutoPhrase].strHZ, strHZ);
		autoPhrase[iAutoPhrase].iSelected = 0;
		iAutoPhrase++;
	    }
	    else {
		insertPoint->flag = False;
		strcpy (insertPoint->strCode, strNewPhraseCode);
		strcpy (insertPoint->strHZ, strHZ);
		insertPoint->iSelected = 0;
		insertPoint = insertPoint->next;
	    }

	  _next:
	    continue;
	}

    }

    free(strHZ);
}

void UpdateHZLastInput (char *str)
{
    int             i, j;

    for (i = 0; i < strlen (str) / 2; i++) {
	if (iHZLastInputCount < PHRASE_MAX_LENGTH)
	    iHZLastInputCount++;
	else {
	    for (j = 0; j < (iHZLastInputCount - 1); j++) {
		hzLastInput[j].strHZ[0] = hzLastInput[j + 1].strHZ[0];
		hzLastInput[j].strHZ[1] = hzLastInput[j + 1].strHZ[1];
	    }
	}
	hzLastInput[iHZLastInputCount - 1].strHZ[0] = str[2 * i];
	hzLastInput[iHZLastInputCount - 1].strHZ[1] = str[2 * i + 1];
	hzLastInput[iHZLastInputCount - 1].strHZ[2] = '\0';
    }

    if (table[iTableIMIndex].bRule && table[iTableIMIndex].bAutoPhrase)
	TableCreateAutoPhrase ((INT8) (strlen (str) / 2));
}
