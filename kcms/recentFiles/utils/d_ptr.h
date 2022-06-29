/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef D_PTR_H
#define D_PTR_H

#include <memory>

namespace kamd
{
namespace utils
{
template<typename T>
class d_ptr
{
private:
    std::unique_ptr<T> d;

public:
    d_ptr();

    template<typename... Args>
    d_ptr(Args &&...);

    ~d_ptr();

    T *operator->() const;
};

#define D_PTR                                                                                                                                                  \
    class Private;                                                                                                                                             \
    friend class Private;                                                                                                                                      \
    const ::kamd::utils::d_ptr<Private> d

} // namespace utils
} // namespace kamd

#endif
