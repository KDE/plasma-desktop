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
#include <string.h>

#define PARSE_INPUT_SYSTEM ' '

#include "pyParser.h"
#include "pyMapTable.h"
#include "PYFA.h"
#include "sp.h"

extern PYTABLE  PYTable[];

FILE           *fps, *fpt, *fp1, *fp2;
Bool            bSingleHZMode = False;
Bool            bFullPY = False;

typedef struct _PY {
    char            strPY[3];
    char            strHZ[3];
    struct _PY     *next, *prev;
} _PyStruct;

typedef struct _PyPhrase {
    char           *strPhrase;
    char           *strMap;
    struct _PyPhrase *next;
    unsigned int    uIndex;
} _PyPhrase;

typedef struct _PyBase {
    char            strHZ[3];
    struct _PyPhrase *phrase;
    int             iPhraseCount;
    unsigned int    iIndex;
} _PyBase;

typedef struct {
    char            strMap[3];
    struct _PyBase *pyBase;
    int             iHZCount;
    //char *strMohu;
} __PYFA;

int             iPYFACount;
__PYFA         *PYFAList;
int             YY[1000];
int             iAllCount;

Bool LoadPY (void)
{
    FILE           *fp;
    int             i, j;
    int             iSW;;

    fp = fopen ("pybase.mb", "rb");
    if (!fp)
	return False;

    fread (&iPYFACount, sizeof (int), 1, fp);
    PYFAList = (__PYFA *) malloc (sizeof (__PYFA) * iPYFACount);
    for (i = 0; i < iPYFACount; i++) {
	fread (PYFAList[i].strMap, sizeof (char) * 2, 1, fp);
	PYFAList[i].strMap[2] = '\0';
	fread (&(PYFAList[i].iHZCount), sizeof (int), 1, fp);
	PYFAList[i].pyBase = (_PyBase *) malloc (sizeof (_PyBase) * PYFAList[i].iHZCount);
	for (j = 0; j < PYFAList[i].iHZCount; j++) {
	    fread (PYFAList[i].pyBase[j].strHZ, sizeof (char) * 2, 1, fp);
	    PYFAList[i].pyBase[j].strHZ[2] = '\0';
	    PYFAList[i].pyBase[j].phrase = (_PyPhrase *) malloc (sizeof (_PyPhrase));
	    PYFAList[i].pyBase[j].phrase->next = NULL;
	    PYFAList[i].pyBase[j].iPhraseCount = 0;
	}
    }

    fclose (fp);

    i = 0;

    while (1) {
	iSW = 0;
	for (j = 0; j < iPYFACount; j++) {
	    if (i < PYFAList[j].iHZCount) {
		PYFAList[j].pyBase[i].iIndex = iAllCount--;
		iSW = 1;
	    }
	}
	if (!iSW)
	    break;
	i++;
    }

    fp = fopen ("pybase.mb", "wb");
    if (!fp)
	return False;

    fwrite (&iPYFACount, sizeof (int), 1, fp);
    for (i = 0; i < iPYFACount; i++) {
	fwrite (PYFAList[i].strMap, sizeof (char) * 2, 1, fp);
	fwrite (&(PYFAList[i].iHZCount), sizeof (int), 1, fp);
	for (j = 0; j < PYFAList[i].iHZCount; j++) {
	    fwrite (PYFAList[i].pyBase[j].strHZ, sizeof (char) * 2, 1, fp);
	    fwrite (&(PYFAList[i].pyBase[j].iIndex), sizeof (int), 1, fp);
	}
    }

    fclose (fp);

    return True;
}

void CreatePYPhrase (void)
{
    char            strPY[256];
    char            strPhrase[256];
    char            strMap[256];
    ParsePYStruct   strTemp;
    int             iIndex, i, s1, s2, j, k;
    _PyPhrase      *phrase, *t, *tt;
    FILE           *f = fopen ("pyERROR", "wt");
    FILE           *fg = fopen ("pyPhrase.ok", "wt");
    int             kkk;
    unsigned int    uIndex, uTemp;

    s1 = 0;
    s2 = 0;
    uIndex = 0;
    while (!feof (fpt)) {
	printf("Reading Phrase: %d\r", s2+1);
	fscanf (fpt, "%s", strPY);
	fscanf (fpt, "%s\n", strPhrase);
	if (strlen (strPhrase) < 3)
	    continue;

	ParsePY (strPY, &strTemp, PY_PARSE_INPUT_SYSTEM);
	s2++;
	kkk = 0;
	//printf("%s  %s  %s   %d\n",strPY,strPhrase,strTemp.strMap,strTemp.iHZCount);
	if (strTemp.iHZCount != strlen (strPhrase) / 2 || (strTemp.iMode & PARSE_ABBR)) {
	    //if ( strlen(strPhrase)==4 )
	    fprintf (f, "%s %s\n", strPY, strPhrase);	//"%s %s %s\n", strPY, strPhrase, strTemp.strPYParsed);
	    continue;
	}

	strMap[0] = '\0';
	for (iIndex = 0; iIndex < strTemp.iHZCount; iIndex++)
	    strcat (strMap, strTemp.strMap[iIndex]);

	for (iIndex = 0; iIndex < iPYFACount; iIndex++) {
	    if (!strncmp (PYFAList[iIndex].strMap, strMap, 2)) {
		for (i = 0; i < PYFAList[iIndex].iHZCount; i++) {
		    if (!strncmp (PYFAList[iIndex].pyBase[i].strHZ, strPhrase, 2)) {
			t = PYFAList[iIndex].pyBase[i].phrase;
			for (j = 0; j < PYFAList[iIndex].pyBase[i].iPhraseCount; j++) {
			    tt = t;
			    t = t->next;
			    if (!strcmp (t->strMap, strMap + 2) && !strcmp (t->strPhrase, strPhrase + 2)) {
				printf ("\n\t%d: %s %s ----->deleted.\n", s2, strPY, strPhrase);
				goto _next;
			    }
			    if (strcmp (t->strMap, strMap + 2) > 0) {
				t = tt;
				break;
			    }
			}

			phrase = (_PyPhrase *) malloc (sizeof (_PyPhrase));
			phrase->strPhrase = (char *) malloc (sizeof (char) * (strlen (strPhrase) - 1));
			phrase->strMap = (char *) malloc (sizeof (char) * (strTemp.iHZCount * 2 - 1));
			phrase->uIndex = uIndex++;
			strcpy (phrase->strPhrase, strPhrase + 2);
			strcpy (phrase->strMap, strMap + 2);

			tt = t->next;
			t->next = phrase;
			phrase->next = tt;
			PYFAList[iIndex].pyBase[i].iPhraseCount++;
			s1++;
			kkk = 1;
		      _next:
			;
		    }
		}
	    }
	}
	if (!kkk)
	    fprintf (f, "%s %s %s\n", strPY, strPhrase, (char *) (strTemp.strPYParsed));
	else
	    fprintf (fg, "%s %s\n", strPY, strPhrase);
    }
    printf ("\n%d Phrases£¬%d Converted!\nWriting Phrase file ...", s2, s1);
    for (i = 0; i < iPYFACount; i++) {
	for (j = 0; j < PYFAList[i].iHZCount; j++) {
	    iIndex = PYFAList[i].pyBase[j].iPhraseCount;
	    if (iIndex) {
		fwrite (&i, sizeof (int), 1, fp2);
		fwrite (PYFAList[i].pyBase[j].strHZ, sizeof (char) * 2, 1, fp2);
		
		fwrite (&iIndex, sizeof (int), 1, fp2);
		t = PYFAList[i].pyBase[j].phrase->next;
		for (k = 0; k < PYFAList[i].pyBase[j].iPhraseCount; k++) {
		    iIndex = strlen (t->strMap);
		    fwrite (&iIndex, sizeof (int), 1, fp2);
		    fwrite (t->strMap, sizeof (char), iIndex, fp2);
		    fwrite (t->strPhrase, sizeof (char), strlen (t->strPhrase), fp2);
		    uTemp = uIndex - 1 - t->uIndex;
		    fwrite (&uTemp, sizeof (unsigned int), 1, fp2);
		    t = t->next;
		}
	    }
	}
    }
    printf ("\nOK!\n");
    
    fclose (fp2);
    fclose (fpt);
}

void CreatePYBase (void)
{
    _PyStruct      *head, *pyList, *temp, *t;
    char            strPY[7], strHZ[3], strMap[3];
    int             iIndex, iCount, i;
    int             iBaseCount;
    int             s = 0;
    int             tt = 0;

    head = (_PyStruct *) malloc (sizeof (_PyStruct));
    head->prev = head;
    head->next = head;

    iBaseCount = 0;
    while (PYTable[iBaseCount].strPY[0] != '\0')
	iBaseCount++;
    for (iIndex = 0; iIndex < iBaseCount; iIndex++)
	YY[iIndex] = 0;
    iIndex = 0;

    while (!feof (fps)) {
	fscanf (fps, "%s", strPY);
	fscanf (fps, "%s\n", strHZ);

	if (MapPY (strPY, strMap, PARSE_INPUT_SYSTEM)) {
	    for (i = 0; i < iBaseCount; i++)
		if ((!strcmp (PYTable[i].strPY, strPY)) && PYTable[i].pMH == NULL)
		    YY[i] += 1;
	    iIndex++;
	    temp = (_PyStruct *) malloc (sizeof (_PyStruct));
	    strcpy (temp->strHZ, strHZ);
	    strcpy (temp->strPY, strMap);
	    pyList = head->prev;

	    while (pyList != head) {
		if (strcmp (pyList->strPY, strMap) <= 0)
		    break;
		pyList = pyList->prev;
	    }

	    temp->next = pyList->next;
	    temp->prev = pyList;
	    pyList->next->prev = temp;
	    pyList->next = temp;
	}
	else
	    printf ("%s Error!!!!\n", strPY);
    }

    iCount = 0;
    for (i = 0; i < iBaseCount; i++) {
	if (YY[i])
	    iCount++;
    }

    fwrite (&iCount, sizeof (int), 1, fp1);
    printf ("Groups: %d\n", iCount);
    iAllCount = iIndex;

    pyList = head->next;

    strcpy (strPY, pyList->strPY);
    iCount = 0;
    t = pyList;

    while (pyList != head) {
	if (!strcmp (strPY, pyList->strPY)) {
	    iCount++;
	}
	else {
	    tt++;
	    fwrite (strPY, sizeof (char) * 2, 1, fp1);
	    fwrite (&iCount, sizeof (int), 1, fp1);
	    for (i = 0; i < iCount; i++) {
		fwrite (t->strHZ, sizeof (char) * 2, 1, fp1);

		t = t->next;
	    }
	    s += iCount;
	    t = pyList;
	    iCount = 1;
	    strcpy (strPY, pyList->strPY);
	}
	pyList = pyList->next;
    }
    fwrite (strPY, sizeof (char) * 2, 1, fp1);
    fwrite (&iCount, sizeof (int), 1, fp1);
    for (i = 0; i < iCount; i++) {
	fwrite (t->strHZ, sizeof (char) * 2, 1, fp1);
	t = t->next;
    }
    s += iCount;

    fclose (fp1);
    fclose (fps);
}

int main (int argc, char *argv[])
{
    fps = fopen (argv[1], "rt");
    fpt = fopen (argv[2], "rt");
    fp1 = fopen ("pybase.mb", "wb");
    fp2 = fopen ("pyphrase.mb", "wb");
    if (fps && fpt && fp1 && fp2) {
	CreatePYBase ();
	LoadPY ();
	CreatePYPhrase ();
    }

    return 0;
}
