/***************************************************************************
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "wheelinterceptor.h"

#include <QCoreApplication>

WheelInterceptor::WheelInterceptor(QQuickItem *parent) : QQuickItem(parent)
{
}

WheelInterceptor::~WheelInterceptor()
{
}

QQuickItem* WheelInterceptor::destination() const
{
    return m_destination;
}

void WheelInterceptor::setDestination(QQuickItem *destination)
{
    if (m_destination != destination) {
        m_destination = destination;

        emit destinationChanged();
    }
}

void WheelInterceptor::wheelEvent(QWheelEvent* event)
{
    if (m_destination) {
        QCoreApplication::sendEvent(m_destination, event);
    }

    emit wheelMoved(event->angleDelta());
}

QQuickItem *WheelInterceptor::findWheelArea(QQuickItem *parent) const
{
    if (!parent) {
        return nullptr;
    }

    foreach(QQuickItem *child, parent->childItems()) {
        // HACK: ScrollView adds the WheelArea below its flickableItem with
        // z==-1. This is reasonable non-risky considering we know about
        // everything else in there, and worst case we break the mouse wheel.
        if (child->z() == -1) {
            return child;
        }
    }

    return nullptr;
}

