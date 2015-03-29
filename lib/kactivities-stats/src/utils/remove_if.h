/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation,3 Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef UTILS_REMOVE_IF_H
#define UTILS_REMOVE_IF_H

#include <kactivities-features.h>
#include <algorithm>

/********************************************************************
 *  Syntactic sugar for the erase-remove idiom                      *
 ********************************************************************/

namespace kamd {
namespace utils {

template <typename Collection, typename Filter>
__inline void remove_if(Collection &collection, Filter filter)
{
    collection.erase(
        std::remove_if(
            collection.begin(), collection.end(), filter),
        collection.end());
}

} // namespace utils
} // namespace kamd

#endif // UTILS_REMOVE_IF_H
