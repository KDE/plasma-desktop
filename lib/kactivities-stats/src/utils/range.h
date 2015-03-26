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

#ifndef UTILS_RANGE_H
#define UTILS_RANGE_H

#include <boost/range/algorithm/copy.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/filtered.hpp>

/********************************************************************
 *  Syntactic sugar for converting ranges to collections            *
 ********************************************************************/

namespace kamd {
namespace utils {

template <typename Collection, typename Range>
__inline Collection as_collection(Range range)
{
    Collection result;

    boost::copy(range, std::back_inserter(result));

    return result;
}

template <typename Member, typename ...Args>
__inline auto transformed(Member member, Args... args)
    -> decltype(boost::adaptors::transformed(
                std::bind(member, args..., std::placeholders::_1)))
{
    return boost::adaptors::transformed(
        std::bind(member, args..., std::placeholders::_1)
    );

}

template <typename Member, typename ...Args>
__inline auto filtered(Member member, Args... args)
    -> decltype(boost::adaptors::filtered(
                std::bind(member, args..., std::placeholders::_1)))
{
    return boost::adaptors::filtered(
        std::bind(member, args..., std::placeholders::_1)
    );

}

template <typename Class, typename Member>
__inline auto filtered(Class *const self, Member member)
    -> decltype(boost::adaptors::filtered(
                std::bind(member, self, std::placeholders::_1)))
{
    return boost::adaptors::filtered(
        std::bind(member, self, std::placeholders::_1)
    );

}


} // namespace utils
} // namespace kamd

#endif // UTILS_RANGE_H
