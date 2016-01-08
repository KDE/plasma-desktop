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

#ifndef UTILS_FOR_EACH_ASSOC_H
#define UTILS_FOR_EACH_ASSOC_H

#include <kactivities-features.h>

/********************************************************************
 *  Associative container's for_each (for hash, map, and similar )  *
 ********************************************************************/

namespace kamd {
namespace utils {

namespace details { //_

// Iterator Functions

template <typename Iterator, typename Function>
Function qt_for_each_assoc(Iterator start, Iterator end, Function f)
{
    for (; start != end; ++start)
        f(start.key(), start.value());

    return f;
}

template <typename Iterator, typename Function>
Function stl_for_each_assoc(Iterator start, Iterator end, Function f)
{
    for (; start != end; ++start)
        f(start->first, start->second);

    return f;
}

// Container functions

template <typename Container, typename Function>
Function _for_each_assoc_helper_container(const Container &c, Function f,
                                          decltype(&Container::constBegin) *)
{
    // STL will never have methods with camelCase :)
    return qt_for_each_assoc(c.constBegin(), c.constEnd(), f);
}

template <typename Container, typename Function, typename Default>
Function _for_each_assoc_helper_container(const Container &c, Function f,
                                          Default *)
{
    return stl_for_each_assoc(c.cbegin(), c.cend(), f);
}

} //^ namespace details

template <typename Container, typename Function>
Function for_each_assoc(const Container &c, Function f)
{
    return details::_for_each_assoc_helper_container
        <Container, Function>(c, f, Q_NULLPTR);
}

} // namespace utils
} // namespace kamd

#endif // UTILS_FOR_EACH_ASSOC_H
