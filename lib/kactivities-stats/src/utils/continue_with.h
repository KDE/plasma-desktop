/*
 * continue_with.h
 * Copyright (C) 2014 Ivan Čukić <ivan.cukic(at)kde.org>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef UTILS_CONTINUE_WITH_H
#define UTILS_CONTINUE_WITH_H

#include <QFuture>
#include <QFutureWatcher>
#include <QDebug>
#include <boost/optional.hpp>

#ifdef ENABLE_QJSVALUE_CONTINUATION
#include <QJSValue>
#endif

namespace kamd {
namespace utils {

namespace detail { //_
#ifdef ENABLE_QJSVALUE_CONTINUATION
    inline void test_continuation(const QJSValue &continuation)
    {
        if (!continuation.isCallable()) {
            qWarning() << "Passed handler is not callable: " << continuation.toString();
        }
    }

    template <typename _ReturnType>
    inline void pass_value(const QFuture<_ReturnType> &future,
                           QJSValue continuation)
    {
        auto result = continuation.call({ future.result() });
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

    template <typename _Continuation>
    inline void test_continuation(_Continuation &&continuation)
    {
        Q_UNUSED(continuation);
    }

    template <typename _ReturnType, typename _Continuation>
    inline void pass_value(const QFuture<_ReturnType> &future,
                           _Continuation &&continuation)
    {
        continuation(future.resultCount() > 0
                         ? boost::make_optional(future.result())
                         : boost::none);
    }

    template <typename _Continuation>
    inline void pass_value(_Continuation &&continuation)
    {
        continuation();
    }

} //^ namespace detail

template <typename _ReturnType, typename _Continuation>
inline void continue_with(const QFuture<_ReturnType> &future,
                          _Continuation &&continuation)
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


