/*
   This file is part of the KDE base distribution
   Copyright (c) 2001 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KRDB_H_
#define _KRDB_H_
#include <KSharedConfig>

enum KRdbAction
{
   KRdbExportColors      = 0x0001,   // Export colors to non-(KDE/Qt) apps
   KRdbExportQtColors    = 0x0002,   // Export KDE's colors to qtrc
   KRdbExportQtSettings  = 0x0004,   // Export all possible qtrc settings, excluding colors
   KRdbExportXftSettings = 0x0008,   // Export KDE's Xft (anti-alias) settings
   KRdbExportGtkTheme    = 0x0010,   // Export KDE's widget style to Gtk if possible
};

void runRdb( uint flags );

#endif
