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
#ifndef _PUNC_H
#define _PUNC_H

#include <X11/Xlib.h>

#define PUNC_DICT_FILENAME	"punc.mb"
#define MAX_PUNC_NO		2
#define MAX_PUNC_LENGTH		4

typedef struct TCHNPUNC {
    int             ASCII;
    char            strChnPunc[MAX_PUNC_NO][MAX_PUNC_LENGTH + 1];
    unsigned        iCount:2;
    unsigned        iWhich:2;
} ChnPunc;

Bool            LoadPuncDict (void);
char           *GetPunc (int iKey);
void		FreePunc (void);

#endif
