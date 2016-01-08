/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DEBUG_AND_RETURN_H
#define DEBUG_AND_RETURN_H

#ifdef QT_DEBUG
#include <QDebug>
#endif

namespace kamd {
namespace utils {

template<typename T>
T debug_and_return(const char * message, T && value) {
    #ifdef QT_DEBUG
    qDebug() << message << " " << value;
    #endif

    return std::forward<T>(value);
}

template<typename T>
T debug_and_return(bool debug, const char * message, T && value) {
    #ifdef QT_DEBUG
    if (debug) {
        qDebug() << message << " " << value;
    }
    #endif

    return std::forward<T>(value);
}

} // namespace utils
} // namespace kamd

#endif // DEBUG_AND_RETURN_H

