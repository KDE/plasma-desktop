/*
 *   Copyright (C) 2015 - 2016 by Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTILS_OPTIONAL_VIEW_H
#define UTILS_OPTIONAL_VIEW_H

namespace kamd
{
namespace utils
{
struct none_t {
};
inline const none_t none()
{
    return none_t();
}

// A simple implementation of the optional class
// until we can rely on std::optional.
// It is not going to come close in the supported
// features to the std one.
// (we need it in the core library, so we don't
// want to use boost.optional)
template<typename T>
class optional_view
{
public:
    explicit optional_view(const T &value)
        : m_value(&value)
    {
    }

    optional_view(const none_t &)
        : m_value(nullptr)
    {
    }

    bool is_initialized() const
    {
        return m_value != nullptr;
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

template<typename T>
optional_view<T> make_optional_view(const T &value)
{
    return optional_view<T>(value);
}

} // namespace utils
} // namespace kamd

#endif // UTILS_OPTIONAL_VIEW_H
