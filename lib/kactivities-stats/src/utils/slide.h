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

#ifndef UTILS_SLIDE_H
#define UTILS_SLIDE_H

#include <algorithm>

// Inspired by C++ Seasoning talk by Sean Parent

namespace kamd {
namespace utils {

template <typename Iterator>
void slide(Iterator f, Iterator l, Iterator p)
{
    if (p < f) {
        std::rotate(p, f, l);
    } else if (l < p) {
        std::rotate(f, l, p);
    }
}

template <typename Iterator>
void slide_one(Iterator f, Iterator p)
{
    slide(f, f + 1, p);
}

} // namespace utils
} // namespace kamd


#endif // UTILS_SLIDE_H

