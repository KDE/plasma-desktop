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
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef D_PTR_IMPLEMENTATION_H
#define D_PTR_IMPLEMENTATION_H

#include <utility>

namespace kamd {
namespace utils {

template <typename T>
d_ptr<T>::d_ptr()
    : d(new T())
{
}

template <typename T>
template <typename... Args>
d_ptr<T>::d_ptr(Args &&... args)
    : d(new T(std::forward<Args>(args)...))
{
}

template <typename T>
d_ptr<T>::~d_ptr()
{
}

template <typename T>
T *d_ptr<T>::operator->() const
{
    return d.get();
}

} // namespace utils
} // namespace kamd

#endif
