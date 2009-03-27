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
#ifndef _QUICKPHRASE_H
#define _QUICKPHRASE_H

#include"ime.h"

#define QUICKPHRASE_CODE_LEN	10
#define QUICKPHRASE_PHRASE_LEN  20

typedef struct _QUICK_PHRASE {
    char strCode[QUICKPHRASE_CODE_LEN+1];
    char strPhrase[QUICKPHRASE_PHRASE_LEN*2+1];
    struct _QUICK_PHRASE *prev;
    struct _QUICK_PHRASE *next;
} QUICK_PHRASE;

void LoadQuickPhrase(void);
void FreeQuickPhrase(void);
INPUT_RETURN_VALUE QuickPhraseDoInput (int iKey);
INPUT_RETURN_VALUE QuickPhraseGetCandWords (SEARCH_MODE mode);

#endif
