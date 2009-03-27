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
 * 区位的算法来自于rfinput-2.0
 */
#include <string.h>

#include "qw.h"
#include "InputWindow.h"

extern char     strCodeInput[];
extern int      iCodeInputCount;
extern int      iCandWordCount;
extern int      iCandPageCount;
extern int	iCurrentCandPage;
extern char     strStringGet[];
extern MESSAGE  messageUp[];
extern uint     uMessageUp;
extern MESSAGE  messageDown[];
extern uint     uMessageDown;
extern Bool	bPointAfterNumber;

INPUT_RETURN_VALUE DoQWInput(int iKey)
{
	INPUT_RETURN_VALUE retVal;
	
	retVal = IRV_TO_PROCESS;
	if ( iKey>='0' && iKey<='9') {
		if ( iCodeInputCount!= 4 ) {
			strCodeInput[iCodeInputCount++]=iKey;
			strCodeInput[iCodeInputCount]='\0';
			if ( iCodeInputCount==4 ) {
				strcpy(strStringGet, QWGetCandWord(iKey-'0'-1));
				retVal= IRV_GET_CANDWORDS;
			}
			else if (iCodeInputCount==3)
				retVal=QWGetCandWords(SM_FIRST);
			else
				retVal=IRV_DISPLAY_CANDWORDS;
		}
	}
	else if (iKey == (XK_BackSpace & 0x00FF)) {
		if (!iCodeInputCount)
		    return IRV_DONOT_PROCESS_CLEAN;
		iCodeInputCount--;
		strCodeInput[iCodeInputCount] = '\0';

		if (!iCodeInputCount)
		    retVal = IRV_CLEAN;
		else {
		    iCandPageCount = 0;
		    uMessageDown = 0;
		    retVal = IRV_DISPLAY_CANDWORDS;
		}
	}
	else if (iKey == ' ') {		
		if (!iCodeInputCount)
		    return IRV_TO_PROCESS;
		if (iCodeInputCount!=3)
		    return IRV_DO_NOTHING;
		    
		strcpy(strStringGet, QWGetCandWord(0));
		retVal= IRV_GET_CANDWORDS;
	}
	else
		return IRV_TO_PROCESS;
	
	uMessageUp = 1;	
    	strcpy (messageUp[0].strMsg, strCodeInput);
    	messageUp[0].type = MSG_INPUT;
	if ( iCodeInputCount!=3 )
		uMessageDown = 0;
		
	return retVal;
}

char *QWGetCandWord (int iIndex)
{
	if ( !iCandPageCount )
		return NULL;
		
	uMessageDown =0;
	if ( iIndex==-1 )
		iIndex=9;
	return GetQuWei((strCodeInput[0] - '0') * 10 + strCodeInput[1] - '0',iCurrentCandPage * 10+iIndex+1);
}

INPUT_RETURN_VALUE QWGetCandWords (SEARCH_MODE mode)
{
    int             iQu, iWei;
    int             i;
    char            strTemp[3];

    if ( bPointAfterNumber ) {
	    strTemp[1] = '.';
	    strTemp[2] = '\0';
    }
    else
	    strTemp[1]='\0';

    iQu = (strCodeInput[0] - '0') * 10 + strCodeInput[1] - '0';
    
    if (mode==SM_FIRST ) {
    	iCandPageCount = 9;
	iCurrentCandPage = strCodeInput[2]-'0';
    }
    else {
    	if ( !iCandPageCount )
		return IRV_TO_PROCESS;
    	if (mode==SM_NEXT) {
    		if ( iCurrentCandPage!=iCandPageCount )
			iCurrentCandPage++;
	}
	else {
    		if ( iCurrentCandPage )
			iCurrentCandPage--;
	}
    }
	
    iWei = iCurrentCandPage * 10;

    uMessageDown = 0;
    for (i = 0; i < 10; i++) {
	strTemp[0] = i + 1 + '0';
	if (i == 9)
		strTemp[0] = '0';
	strcpy (messageDown[uMessageDown].strMsg, strTemp);
	messageDown[uMessageDown++].type = MSG_INDEX;	
	
	strcpy (messageDown[uMessageDown].strMsg, GetQuWei (iQu, iWei + i + 1));
	if (i != 9)
		strcat (messageDown[uMessageDown].strMsg, " ");
	messageDown[uMessageDown++].type = (i)? MSG_OTHER:MSG_FIRSTCAND;	
    }    
    
    strCodeInput[2]=iCurrentCandPage+'0';
    uMessageUp = 1;	
    strcpy (messageUp[0].strMsg, strCodeInput);
    messageUp[0].type = MSG_INPUT;
    
    return IRV_DISPLAY_CANDWORDS;
}

char           *GetQuWei (int iQu, int iWei)
{
    static char     strHZ[3];

    if (iQu >= 95) {		/* Process extend Qu 95 and 96 */
	strHZ[0] = iQu - 95 + 0xA8;
	strHZ[1] = iWei + 0x40;

	/* skip 0xa87f and 0xa97f */
	if ((unsigned char) strHZ[1] >= 0x7f)
	    strHZ[1]++;
    }
    else {
	strHZ[0] = iQu + 0xa0;
	strHZ[1] = iWei + 0xa0;
    }

    strHZ[2] = '\0';

    return strHZ;
}
