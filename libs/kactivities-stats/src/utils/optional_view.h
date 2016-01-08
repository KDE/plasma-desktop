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

#ifndef UTILS_OPTIONAL_H
#define UTILS_OPTIONAL_H

namespace kamd {
namespace utils {

struct none_t {};
inline const none_t none() { return none_t(); }

// A simple implementation of the optional class
// until we can rely on std::optional.
// It is not going to come close in the supported
// features to the std one.
// (we need it in the core library, so we don't
// want to use boost.optional)
template <typename T>
class optional_view {
public:
    explicit optional_view(const T &value)
        : m_value(&value)
    {
    }

    optional_view(const none_t &)
        : m_value(Q_NULLPTR)
    {
    }

    bool is_initialized() const
    {
        return m_value != Q_NULLPTR;
    }

    const T &get() const
    {
        return *m_value;
    }

    const T *operator->() const
    {
        return m_value;
    }

private:
    const T *const m_value;
};

template <typename T>
optional_view<T> make_optional_view(const T &value)
{
    return optional_view<T>(value);
}

} // namespace utils
} // namespace kamd


#endif // UTILS_OPTIONAL_H

