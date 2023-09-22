/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "eventgenerator.h"

#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickWindow>

EventGenerator::EventGenerator(QObject *parent)
    : QObject(parent)
{
}

EventGenerator::~EventGenerator()
{
}

void EventGenerator::sendMouseEvent(QQuickItem *item,
                                    EventGenerator::MouseEvent type,
                                    int x,
                                    int y,
                                    int button,
                                    Qt::MouseButtons buttons,
                                    Qt::KeyboardModifiers modifiers)
{
    if (!item) {
        return;
    }

    QEvent::Type eventType;
    switch (type) {
    case MouseButtonPress:
        eventType = QEvent::MouseButtonPress;
        break;
    case MouseButtonRelease:
        eventType = QEvent::MouseButtonRelease;
        break;
    case MouseMove:
        eventType = QEvent::MouseMove;
        break;
    default:
        return;
    }
    QMouseEvent ev(eventType, QPointF(x, y), static_cast<Qt::MouseButton>(button), buttons, modifiers);

    QGuiApplication::sendEvent(item, &ev);
}

#include "moc_eventgenerator.cpp"
