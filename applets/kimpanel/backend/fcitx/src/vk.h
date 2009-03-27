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
#ifndef _VK_WINDOW_H
#define _VK_WINDOW_H

#include <X11/Xlib.h>
#include "ime.h"

#define VK_FILE	"vk.conf"

#define VK_WINDOW_WIDTH		354
#define VK_WINDOW_HEIGHT	164
#define VK_NUMBERS		47
#define VK_MAX			50

typedef struct {
     char            strSymbol[VK_NUMBERS][2][3];	//ÏàÓ¦µÄ·ûºÅ
     char            strName[13];
} VKS;

Bool            CreateVKWindow (void);
void            InitVKWindowColor (void);
void            DisplayVKWindow (void);
void            DrawVKWindow (void);
char           *VKGetSymbol (char cChar);
void            LoadVKMapFile (void);
void            ChangVK (void);
INPUT_RETURN_VALUE DoVKInput (int iKey);
int             MyToLower (int iChar);
int             MyToUpper (int iChar);
void            SwitchVK (void);
Bool            VKMouseKey (int x, int y);

#endif
