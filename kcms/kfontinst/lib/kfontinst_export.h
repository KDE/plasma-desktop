/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/


#ifndef KFONTINST_EXPORT_H
#define KFONTINST_EXPORT_H

#include <QtGlobal>

#if defined _WIN32 || defined _WIN64

#ifndef KFONTINST_EXPORT
# if defined(MAKE_KFONTINST_LIB)
#  define KFONTINST_EXPORT Q_DECL_EXPORT
# else
#  define KFONTINST_EXPORT Q_DECL_IMPORT
# endif
#endif


#else /* UNIX */

#define KFONTINST_EXPORT Q_DECL_EXPORT

#endif

#endif // KFONTINST_EXPORT_H
