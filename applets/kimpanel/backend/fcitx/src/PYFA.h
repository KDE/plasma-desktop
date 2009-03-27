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
#ifndef _PYFA_H
#define _PYFA_H

#include "main.h"

typedef struct MH_PY {
    char           *strMap;
    int             bMode;
} MHPY;

typedef struct {
    char            strPY[7];
    int            *pMH;
} PYTABLE;

int             GetMHIndex_C (char map);
//在输入词组时，比如，当用户输入“jiu's”时，应该可以出现“就是”这个词，而无论是否打开了模糊拼音
int             GetMHIndex_S (char map, Bool bMode);
Bool		IsZ_C_S (char map);

#endif
