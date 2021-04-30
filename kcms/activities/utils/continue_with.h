/*
 *   Copyright (C) 2014 - 2016 by Ivan Cukic <ivan.cukic@kde.org>
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

#ifndef UTILS_CONTINUE_WITH_H
#define UTILS_CONTINUE_WITH_H

#include <QDebug>
#include <QFuture>
#include <QFutureWatcher>

#include "utils/optional_view.h"
// #include <boost/optional.hpp>

#ifdef ENABLE_QJSVALUE_CONTINUATION
#include <QJSValue>
#endif

namespace kamd
{
namespace utils
{
namespace detail
{ //_
#ifdef ENABLE_QJSVALUE_CONTINUATION
inline void test_continuation(const QJSValue &continuation)
{
    if (!continuation.isCallable()) {
        qWarning() << "Passed handler is not callable: " << continuation.toString();
    }
}

template<typename _ReturnType>
inline void pass_value(const QFuture<_ReturnType> &future, QJSValue continuation)
{
    auto result = continuation.call({future.result()});
    if (result.isError()) {
        qWarning() << "Handler returned this error: " << result.toString();
    }
}

inline void pass_value(const QFuture<void> &future, QJSValue continuation)
{
    Q_UNUSED(future);
    auto result = continuation.call({});
    if (result.isError()) {
        qWarning() << "Handler returned this error: " << result.toString();
    }
}
#endif

template<typename _Continuation>
inline void test_continuation(_Continuation &&continuation)
{
    Q_UNUSED(continuation);
}

template<typename _ReturnType, typename _Continuation>
inline void pass_value(const QFuture<_ReturnType> &future, _Continuation &&continuation)
{
    using namespace kamd::utils;
    continuation(future.resultCount() > 0 ? make_optional_view(future.result()) : none());
}

template<typename _Continuation>
inline void pass_value(_Continuation &&continuation)
{
    continuation();
}

} //^ namespace detail

template<typename _ReturnType, typename _Continuation>
inline void continue_with(const QFuture<_ReturnType> &future, _Continuation &&continuation)
{
    detail::test_continuation(continuation);

    auto watcher = new QFutureWatcher<_ReturnType>();
    QObject::connect(watcher, &QFutureWatcherBase::finished, [=]() mutable {
        detail::pass_value(future, continuation);
    });

    watcher->setFuture(future);
}

} // namespace utils
} // namespace kamd

#endif /* !UTILS_CONTINUE_WITH_H */
