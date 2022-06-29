/*
    SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
