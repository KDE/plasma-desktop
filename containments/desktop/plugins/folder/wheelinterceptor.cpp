/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "wheelinterceptor.h"

#include <QCoreApplication>

WheelInterceptor::WheelInterceptor(QQuickItem *parent)
    : QQuickItem(parent)
{
}

WheelInterceptor::~WheelInterceptor()
{
}

QObject *WheelInterceptor::destination() const
{
    return m_destination;
}

void WheelInterceptor::setDestination(QObject *destination)
{
    if (m_destination != destination) {
        m_destination = destination;

        Q_EMIT destinationChanged();
    }
}

void WheelInterceptor::wheelEvent(QWheelEvent *event)
{
    if (m_destination) {
        QCoreApplication::sendEvent(m_destination, event);
    }
}
