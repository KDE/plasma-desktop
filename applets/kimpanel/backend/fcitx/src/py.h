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
#ifndef _PY_H
#define _PY_H

#include	"main.h"
#include	"ime.h"

#define PY_BASE_FILE	"pybase.mb"
#define PY_PHRASE_FILE	"pyphrase.mb"
#define PY_USERPHRASE_FILE "pyusrphrase.mb"
#define PY_INDEX_FILE	"pyindex.dat"
#define PY_FREQ_FILE	"pyfreq.mb"
#define PY_SYMBOL_FILE	"pySym.mb"

#define MAX_WORDS_USER_INPUT	32
#define MAX_PY_PHRASE_LENGTH	10
#define MAX_PY_LENGTH		6

#define AUTOSAVE_PHRASE_COUNT 	5
#define AUTOSAVE_ORDER_COUNT  	10
#define AUTOSAVE_FREQ_COUNT  	1

typedef enum FIND_MODE {
    FIND_PHRASE,
    FIND_BASE,
    FIND_FREQ,
    FIND_SYM
} FINDMODE;

typedef enum {
    PY_CAND_AUTO,
    PY_CAND_SYMBOL,
    PY_CAND_BASE,
    PY_CAND_SYMPHRASE,
    PY_CAND_USERPHRASE,
    PY_CAND_FREQ,
    PY_CAND_LEGEND
} PY_CAND_WORD_TYPE;

typedef struct _HZ {
    char            strHZ[MAX_PY_PHRASE_LENGTH * 2 + 1];
    int             iPYFA;
    uint            iHit;
    uint            iIndex;
    struct _HZ     *next;
    uint            flag:1;
} HZ;

typedef struct PYFREQ {
    HZ             *HZList;
    char            strPY[MAX_PY_PHRASE_LENGTH * MAX_PY_LENGTH + 1];
    uint            iCount;
    Bool            bIsSym;	//For special symbols
    struct PYFREQ  *next;
} PyFreq;

typedef struct PYPHRASE {
    char           *strPhrase;
    char           *strMap;
    struct PYPHRASE *next;
    uint            iIndex;
    uint            iHit;
    uint            flag:1;
} PyPhrase;

typedef struct PYBASE {
    char            strHZ[3];
    struct PYPHRASE *phrase;
    int             iPhrase;
    struct PYPHRASE *userPhrase;
    int             iUserPhrase;
    uint            iIndex;
    uint            iHit;
    uint            flag:1;
    uint	    iChangeCount;	//Whether we have changed the Index/Hit value of this group
} PyBase;

typedef struct _PYFA {
    char            strMap[3];
    struct PYBASE  *pyBase;
    int             iBase;
    uint	    iChangeCount;	//Whether we have changed the Index/Hit value of this group
} PYFA;

typedef struct PYINDEX {
    int             iPYFA;
    int             iBase;
    int             iPhrase;
    struct PYINDEXCANDWORD *next;
    struct PYINDEXCANDWORD *prev;
} PYIndex;

typedef struct PYFREQCANDWORD {
    HZ             *hz;
    char           *strPY;
} PYFreqCandWord;

typedef struct PYPHRASECANDWORD {
    int             iPYFA;
    int             iBase;
    struct PYPHRASE *phrase;
} PYPhraseCandWord;

typedef struct PYBASECANDWORD {
    int             iPYFA;
    int             iBase;
} PYBaseCandWord;

typedef struct PYLEGENDCANDWORD {
    PyPhrase       *phrase;
    int             iLength;
} PYLegendCandWord;

typedef union {
    PYFreqCandWord  sym;
    PYFreqCandWord  freq;
    PYBaseCandWord  base;
    PYPhraseCandWord phrase;
} PCand;

typedef struct PYCANDWORD {
    PCand           cand;
    uint            iWhich:3;	//0->Auto 1->System single HZ 2->System phrase 3->User phrase 4->frequent HZ
} PYCandWord;

typedef struct PYCANDINDEX {
    int             iPYFA;
    int             iBase;
    int             iPhrase;
} PYCandIndex;

typedef struct {
    char            strPY[(MAX_PY_LENGTH + 1) * MAX_PY_PHRASE_LENGTH + 1];
    char            strHZ[MAX_PY_PHRASE_LENGTH * 2 + 1];
    char            strMap[MAX_PY_PHRASE_LENGTH * 2 + 1];
} PY_SELECTED;

void            PYInit (void);
Bool            LoadPYBaseDict (void);
Bool            LoadPYOtherDict (void);
void            ResetPYStatus ();
int             GetBaseIndex (int iPYFA, char *strBase);
INPUT_RETURN_VALUE DoPYInput (int iKey);
void            UpdateCodeInputPY (void);
void            UpdateFindString (void);
void            CalculateCursorPosition (void);

void            PYResetFlags (void);
void            PYCreateAuto (void);
INPUT_RETURN_VALUE PYGetCandWords (SEARCH_MODE mode);
void		PYCreateCandString(void);
void		PYGetCandText(int iIndex, char *strText);
char           *PYGetCandWord (int iIndex);
void            PYGetSymCandWords (SEARCH_MODE mode);
Bool            PYAddSymCandWord (HZ * hz, SEARCH_MODE mode);
void            PYGetBaseCandWords (SEARCH_MODE mode);
Bool            PYAddBaseCandWord (PYCandIndex pos, SEARCH_MODE mode);
void            PYGetFreqCandWords (SEARCH_MODE mode);
Bool            PYAddFreqCandWord (HZ * hz, char *strPY, SEARCH_MODE mode);
void            PYGetPhraseCandWords (SEARCH_MODE mode);
Bool            PYAddPhraseCandWord (PYCandIndex pos, PyPhrase * phrase, SEARCH_MODE mode, Bool b);
void            PYGetCandWordsForward (void);
void            PYGetCandWordsBackward (void);
Bool            PYCheckNextCandPage (void);
void            PYSetCandWordFlag (int iIndex, Bool flag);
void            PYSetCandWordsFlag (Bool flag);
Bool            PYAddUserPhrase (char *phrase, char *map);
void            PYDelUserPhrase (int iPYFA, int iBase, PyPhrase * phrase);
int             GetBaseMapIndex (char *strMap);
void            SavePYUserPhrase (void);
void            SavePYFreq (void);
void            SavePYIndex (void);

void            PYAddFreq (int iIndex);
void            PYDelFreq (int iIndex);
Bool            PYIsInFreq (char *strHZ);

INPUT_RETURN_VALUE PYGetLegendCandWords (SEARCH_MODE iMode);
Bool            PYAddLengendCandWord (PyPhrase * phrase, SEARCH_MODE mode);
char           *PYGetLegendCandWord (int iIndex);
void            PYSetLegendCandWordsFlag (Bool flag);
void		PYGetPYByHZ(char *strHZ, char *strPY);

//void            PP ();
#endif
