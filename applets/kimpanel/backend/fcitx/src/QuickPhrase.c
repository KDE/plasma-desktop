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
#include <stdio.h>
#include <limits.h>

#include "QuickPhrase.h"
#include "InputWindow.h"

uint uQuickPhraseCount;
QUICK_PHRASE *quickPhraseHead = NULL;
QUICK_PHRASE *currentQuickPhrase;
QUICK_PHRASE *quickPhraseCandWords[MAX_CAND_WORD];

extern int iCandPageCount;
extern int iCandWordCount;
extern int iCurrentCandPage;
extern char strCodeInput[];
extern uint uMessageDown;
extern MESSAGE         messageDown[];
extern char strStringGet[];

/**
 * @brief 加载快速输入词典
 * @param void
 * @return void
 * @note 快速输入是在;的行为定义为2,并且输入;以后才可以使用的。
 * 加载快速输入词典.如：输入“zg”就直接出现“中华人民共和国”等等。
 * 文件中每一行数据的定义为：<字符组合> <短语>
 * 如：“zg 中华人民共和国”
 */
void LoadQuickPhrase(void)
{
    FILE *fp;
    char            strPath[PATH_MAX];
    char strCode[QUICKPHRASE_CODE_LEN+1];
    char strPhrase[QUICKPHRASE_PHRASE_LEN*2+1];
    QUICK_PHRASE *tempQuickPhrase;
    QUICK_PHRASE *quickPhrase;

    uQuickPhraseCount=0;

    strcpy (strPath, (char *) getenv ("HOME"));
    strcat (strPath, "/.fcitx/");
    strcat (strPath, "QuickPhrase.mb");

    if (access (strPath, 0)) {
	strcpy (strPath, PKGDATADIR "/data/");
	strcat (strPath, "QuickPhrase.mb");
    }

    fp = fopen (strPath, "rt");
    if (!fp)
    	return;

    quickPhrase=quickPhraseHead=(QUICK_PHRASE *)malloc(sizeof(QUICK_PHRASE));
    quickPhraseHead->prev=NULL;

    // 这儿的处理比较简单。因为是单索引对单值。
    // 应该注意的是，它在内存中是以单链表保存的。
    for (;;) {
	if (EOF==fscanf (fp, "%s", strCode))
	    break;
	if (EOF==fscanf (fp, "%s", strPhrase))
	    break;

	tempQuickPhrase=(QUICK_PHRASE *)malloc(sizeof(QUICK_PHRASE));
	strcpy(tempQuickPhrase->strCode,strCode);
	strcpy(tempQuickPhrase->strPhrase,strPhrase);

	quickPhrase->next=tempQuickPhrase;
	tempQuickPhrase->prev=quickPhrase;
	quickPhrase=tempQuickPhrase;
    }
    quickPhrase->next=NULL;

    fclose(fp);
}

void FreeQuickPhrase(void)
{
    QUICK_PHRASE *tempQuickPhrase,*quickPhrase;

    if ( !quickPhraseHead )
	return;

    quickPhrase=quickPhraseHead->next;
    while (quickPhrase) {
	tempQuickPhrase=quickPhrase->next;
	free(quickPhrase);
	quickPhrase=tempQuickPhrase;
    }

    free(quickPhraseHead);
    quickPhraseHead = NULL;
}

INPUT_RETURN_VALUE QuickPhraseDoInput (int iKey)
{
    int retVal;

    if (iKey >= '0' && iKey <= '9') {
	if (!iCandWordCount)
	    return IRV_TO_PROCESS;

	iKey -= '0';
	if (iKey == 0)
	    iKey = 10;

	if (iKey > iCandWordCount)
	    retVal = IRV_DO_NOTHING;
	else {
	    strcpy (strStringGet, quickPhraseCandWords[iKey-1]->strPhrase);
	    retVal = IRV_GET_CANDWORDS;
	    uMessageDown = 0;
	}
    }
    else if (iKey==' ') {
	if (!iCandWordCount)
	    retVal = IRV_TO_PROCESS;
	else {
	    strcpy (strStringGet, quickPhraseCandWords[0]->strPhrase);
	    retVal = IRV_GET_CANDWORDS;
	    uMessageDown = 0;
	}
    }
    else
	retVal = IRV_TO_PROCESS;

    return retVal;
}

INPUT_RETURN_VALUE QuickPhraseGetCandWords (SEARCH_MODE mode)
{
    int i, iInputLen;
    char strTemp[2];

    if ( !quickPhraseHead )
        return IRV_DISPLAY_MESSAGE;

//    if (mode==SM_FIRST) {
	currentQuickPhrase=quickPhraseHead->next;
	iCandPageCount=0;
	iCurrentCandPage=0;
	iCandWordCount=0;
	iInputLen = strlen(strCodeInput);
/*
	while (currentQuickPhrase) {
	    if (!strcmp(strCodeInput, currentQuickPhrase->strCode))
		break;

	    currentQuickPhrase=currentQuickPhrase->next;
	}

	if ( !currentQuickPhrase ) {
	    uMessageDown = 0;
	    return IRV_DISPLAY_MESSAGE;
	    //return IRV_DISPLAY_CANDWORDS;
	}
    }
    else if (mode==SM_NEXT) {
	if (iCurrentCandPage>=iCandPageCount)
	    return IRV_DO_NOTHING;
	iCandWordCount=0;
	iCurrentCandPage++;
    }
    else {
	if (!iCurrentCandPage)
	    return IRV_DO_NOTHING;
	iCurrentCandPage--;
    }

    if ( mode!=SM_PREV) {
*/	while (currentQuickPhrase ) {
	    if (!strncmp(strCodeInput,currentQuickPhrase->strCode,iInputLen)) {
		quickPhraseCandWords[iCandWordCount++]=currentQuickPhrase;
		if (iCandPageCount==MAX_CAND_WORD) {
/*		    quickPhrase=currentQuickPhrase;
		    while (quickPhrase) {
			if (!strcmp(strCodeInput,quickPhrase->strCode)) {
			    iCandPageCount++;
			    break;
			}
		    }
*/		    break;
		}
	    }
	    currentQuickPhrase=currentQuickPhrase->next;
	}
/*    }
    else {
	i=0;

	for (;;) {
	    if (!strcmp(strCodeInput,currentQuickPhrase->strCode)) {
		i++;
		currentQuickPhrase=currentQuickPhrase->prev;
		if (i==MAX_CAND_WORD)
		    break;
	    }
	    iCandWordCount=0;
	    quickPhrase=currentQuickPhrase;
	    for (;;) {
		if (!strcmp(strCodeInput,currentQuickPhrase->strCode)) {
		    iCandWordCount++;
		    quickPhraseCandWords[MAX_CAND_WORD-iCandWordCount]=currentQuickPhrase;
		    currentQuickPhrase=currentQuickPhrase->prev;
		    if (iCandWordCount==MAX_CAND_WORD)
			break;
		}
	    }
	    currentQuickPhrase=quickPhrase;
	}
    }
*/
    if (!iCandWordCount)
        return IRV_DISPLAY_MESSAGE;

    uMessageDown = 0;
    strTemp[1]='\0';

    for (i = 0; i < iCandWordCount; i++) {
	strTemp[0] = i + 1 + '0';
	if (i == 9)
	    strTemp[0] = '0';
	strcpy (messageDown[uMessageDown].strMsg, strTemp);
	messageDown[uMessageDown++].type = MSG_INDEX;

	strcpy (messageDown[uMessageDown].strMsg, quickPhraseCandWords[i]->strPhrase);
	messageDown[uMessageDown++].type = ((i == 0) ? MSG_FIRSTCAND : MSG_OTHER);

	//编码提示
	strcpy (messageDown[uMessageDown].strMsg, quickPhraseCandWords[i]->strCode + iInputLen);
	if (i != (iCandWordCount - 1))
	    strcat (messageDown[uMessageDown].strMsg, " ");
	messageDown[uMessageDown++].type = MSG_CODE;
    }

    return IRV_DISPLAY_CANDWORDS;
}
