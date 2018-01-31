/* This file is part of the KDE Project
   Copyright (c) 2008-2010 Sebastian Trueg <trueg@kde.org>

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

#ifndef _FILE_EXCLUDE_FILTERS_H_
#define _FILE_EXCLUDE_FILTERS_H_

#include <QStringList>

namespace Baloo
{
/**
 * \return A list of default exclude filters to be used
 * in the filewatch service to ignore temporary files
 * and folders that change a lot and as a basis for the
 * user configurable exclude filters in the strigi service.
 */
QStringList defaultExcludeFilterList();

/**
 * \return The version of the default exclude filter list.
 * This is increased whenever the list changes.
 */
int defaultExcludeFilterListVersion();

QStringList defaultExcludeMimetypes();
int defaultExcludeMimetypesVersion();

QStringList sourceCodeMimeTypes();
}

#endif
