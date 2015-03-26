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

#ifndef D_PTR_H
#define D_PTR_H

#include <memory>

namespace kamd {
namespace utils {

template <typename T>
class d_ptr {
private:
    std::unique_ptr<T> d;

public:
    d_ptr();

    template <typename... Args>
    d_ptr(Args &&...);

    ~d_ptr();

    T *operator->() const;
};

#define D_PTR             \
    class Private;        \
    friend class Private; \
    const ::kamd::utils::d_ptr<Private> d

} // namespace utils
} // namespace kamd

#endif
