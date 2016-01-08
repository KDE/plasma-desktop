/*
 * Copyright (c) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "dbusfuture_p.h"

namespace DBusFuture {

namespace detail { //_

template <>
void DBusCallFutureInterface<void>::callFinished()
{
    deleteLater();

    // qDebug() << "This is call end";

    this->reportFinished();
}

ValueFutureInterface<void>::ValueFutureInterface()
{
}

QFuture<void> ValueFutureInterface<void>::start()
{
    auto future = this->future();

    this->reportFinished();

    deleteLater();

    return future;
}

} //^ namespace detail

QFuture<void> fromVoid()
{
    using namespace detail;

    auto valueFutureInterface = new ValueFutureInterface<void>();

    return valueFutureInterface->start();
}

} // namespace DBusFuture

