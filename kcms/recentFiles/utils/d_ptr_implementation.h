/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef D_PTR_IMPLEMENTATION_H
#define D_PTR_IMPLEMENTATION_H

#include <utility>

namespace kamd
{
namespace utils
{
template<typename T>
d_ptr<T>::d_ptr()
    : d(new T())
{
}

template<typename T>
template<typename... Args>
d_ptr<T>::d_ptr(Args &&...args)
    : d(new T(std::forward<Args>(args)...))
{
}

template<typename T>
d_ptr<T>::~d_ptr()
{
}

template<typename T>
T *d_ptr<T>::operator->() const
{
    return d.get();
}

} // namespace utils
} // namespace kamd

#endif
