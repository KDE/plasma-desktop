#ifndef __UTILS_H__
#define __UTILS_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2009 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QtCore/QString>
#include "Family.h"

namespace KFI
{
namespace Utils
{

enum EFileType
{
    FILE_INVALID,
    FILE_BITMAP,
    FILE_SCALABLE,
    FILE_AFM,
    FILE_PFM
};

extern bool isAAfm(const QString &fname);
extern bool isAPfm(const QString &fname);
extern bool isAType1(const QString &fname);
extern void createAfm(const QString &file, EFileType type);
extern EFileType check(const QString &file, Family &fam);

}

}

#endif
