/***************************************************************************
 *   Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company *
 *                      <info@kdab.com>                                    *
 *   Author: Andras Mantia <andras.mantia@kdab.com>                        *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef FOLDERPLUGIN_PRIVATE_EXPORT_H
#define FOLDERPLUGIN_PRIVATE_EXPORT_H

#include "folderplugin_export.h"

/* Classes which are exported only for unit tests */
#ifdef BUILD_TESTING
#ifndef FOLDERPLUGIN_TESTS_EXPORT
#define FOLDERPLUGIN_TESTS_EXPORT FOLDERPLUGIN_EXPORT
#endif
#else /* not compiling tests */
#define FOLDERPLUGIN_TESTS_EXPORT
#endif

#endif // FOLDERPLUGIN_PRIVATE_EXPORT_H
