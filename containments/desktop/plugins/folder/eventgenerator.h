/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EventGenerator_H
#define EventGenerator_H

#include <QObject>

class QQuickItem;

class EventGenerator : public QObject
{
    Q_OBJECT

public:
    enum MouseEvent {
        MouseButtonPress,
        MouseButtonRelease,
        MouseMove,
    };
    Q_ENUM(MouseEvent)

    EventGenerator(QObject *parent = nullptr);
    ~EventGenerator() override;

    /**
     * Send a mouse event of @type to the given @item
     */
    Q_INVOKABLE void
    sendMouseEvent(QQuickItem *item, EventGenerator::MouseEvent type, int x, int y, int button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
};

#endif
