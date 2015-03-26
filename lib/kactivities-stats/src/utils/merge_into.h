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

#ifndef UTILS_MERGE_INTO_H
#define UTILS_MERGE_INTO_H

namespace kamd {
namespace utils {

template <typename Container>
inline void merge_into(Container &into, const Container &from)
{
    typename Container::iterator into_begin = into.begin();
    typename Container::iterator into_end = into.end();
    typename Container::const_iterator from_begin = from.begin();
    typename Container::const_iterator from_end = from.end();

    while (from_begin != from_end) {
        while (into_begin != into_end && *from_begin >= *into_begin)
            into_begin++;

        into_begin = into.insert(into_begin, *from_begin);
        into_begin++;
        from_begin++;
    }
}

} // namespace utils
} // namespace kamd

#endif // UTILS_MERGE_INTO_H
