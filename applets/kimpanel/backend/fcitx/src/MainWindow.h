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
/**
 * @file   MainWindow.h
 * @author Yuking yuking_net@sohu.com
 * @date   2008-1-16
 * 
 * @brief  Ö÷´°¿Ú
 * 
 * 
 */

#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <X11/Xlib.h>

#define MAINWND_STARTX	500

#define _MAINWND_VK_WIDTH	24
#define _MAINWND_WIDTH	104 + 18 +_MAINWND_VK_WIDTH
#define _MAINWND_WIDTH_COMPACT	32+_MAINWND_VK_WIDTH

#define MAINWND_STARTY	1
#define MAINWND_HEIGHT	20

typedef enum _HIDE_MAINWINDOW {
    HM_SHOW,
    HM_AUTO,
    HM_HIDE
} HIDE_MAINWINDOW;

Bool            CreateMainWindow (void);
void            DisplayMainWindow (void);
void            DrawMainWindow (void);
void            InitMainWindowColor (void);
void            ChangeLock (void);

#endif
