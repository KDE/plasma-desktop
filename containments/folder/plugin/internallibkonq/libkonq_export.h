/*
   This file is part of the KDE project
   Copyright (C) 2004 Jarosław Staniek <staniek@kde.org>
	     (C) 2004 Dirk Mueller <mueller@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef LIBKONQ_EXPORT_H
#define LIBKONQ_EXPORT_H

/* needed for KDE_EXPORT macros */
#include <kdemacros.h>

/* needed, because e.g. Q_OS_UNIX is so frequently used */
#include <QtCore/qglobal.h>

#ifdef MAKE_KONQ_LIB
# define LIBKONQ_EXPORT KDE_EXPORT
#else
# ifdef Q_OS_WIN
#  define LIBKONQ_EXPORT KDE_IMPORT
# else
#  define LIBKONQ_EXPORT KDE_EXPORT
# endif
#endif

# ifndef LIBKONQ_EXPORT_DEPRECATED
#  define LIBKONQ_EXPORT_DEPRECATED KDE_DEPRECATED LIBKONQ_EXPORT
# endif

#endif
