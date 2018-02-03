/*
 *   Copyright (C) 2013 - 2016 by Ivan Cukic <ivan.cukic@kde.org>
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

#ifndef ACTIVITIES_DBUSFUTURE_P_H
#define ACTIVITIES_DBUSFUTURE_P_H

#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDBusAbstractInterface>
#include <QDBusPendingCallWatcher>
#include <QFutureInterface>
#include <QFuture>
#include <QFutureWatcherBase>

namespace DBusFuture {

namespace detail { //_

template <typename _Result>
class DBusCallFutureInterface : public QObject,
                                public QFutureInterface<_Result> {
public:
    DBusCallFutureInterface(QDBusPendingReply<_Result> reply)
        : reply(reply),
          replyWatcher(nullptr)
    {
    }

    ~DBusCallFutureInterface()
    {
        delete replyWatcher;
    }

    void callFinished();

    QFuture<_Result> start()
    {
        replyWatcher = new QDBusPendingCallWatcher(reply);

        QObject::connect(replyWatcher,
                         &QDBusPendingCallWatcher::finished,
                         [this] () { callFinished(); });

        this->reportStarted();

        if (reply.isFinished()) {
            this->callFinished();
        }

        return this->future();
    }

private:
    QDBusPendingReply<_Result> reply;
    QDBusPendingCallWatcher * replyWatcher;
};

template <typename _Result>
void DBusCallFutureInterface<_Result>::callFinished()
{
    deleteLater();

    if (!reply.isError()) {
        this->reportResult(reply.value());
    }

    this->reportFinished();
}

template <>
void DBusCallFutureInterface<void>::callFinished();

template <typename _Result>
class ValueFutureInterface : public QObject, QFutureInterface<_Result> {
public:
    ValueFutureInterface(const _Result & value)
        : value(value)
    {
    }

    QFuture<_Result> start()
    {
        auto future = this->future();

        this->reportResult(value);
        this->reportFinished();

        deleteLater();

        return future;
    }

private:
    _Result value;

};

template <>
class ValueFutureInterface<void> : public QObject, QFutureInterface<void> {
public:
    ValueFutureInterface();

    QFuture<void> start();
    // {
    //     auto future = this->future();
    //     this->reportFinished();
    //     deleteLater();
    //     return future;
    // }
};

} //^ namespace detail

template <typename _Result>
QFuture<_Result>
asyncCall(QDBusAbstractInterface *interface, const QString &method,
          const QVariant &arg1 = QVariant(), const QVariant &arg2 = QVariant(),
          const QVariant &arg3 = QVariant(), const QVariant &arg4 = QVariant(),
          const QVariant &arg5 = QVariant(), const QVariant &arg6 = QVariant(),
          const QVariant &arg7 = QVariant(), const QVariant &arg8 = QVariant())
{
    using namespace detail;

    auto callFutureInterface = new DBusCallFutureInterface
        <_Result>(interface->asyncCall(method, arg1, arg2, arg3, arg4, arg5,
                                       arg6, arg7, arg8));

    return callFutureInterface->start();
}

template <typename _Result>
QFuture<_Result>
fromValue(const _Result & value)
{
    using namespace detail;

    auto valueFutureInterface = new ValueFutureInterface<_Result>(value);

    return valueFutureInterface->start();
}

template <typename _Result>
QFuture<_Result>
fromReply(const QDBusPendingReply<_Result> &reply)
{
    using namespace detail;

    auto callFutureInterface = new DBusCallFutureInterface<_Result>(reply);

    return callFutureInterface->start();
}

QFuture<void> fromVoid();

} // namespace DBusFuture

#endif /* DBUSFUTURE_P_H */

