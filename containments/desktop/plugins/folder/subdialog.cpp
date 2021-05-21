/*
    SPDX-FileCopyrightText: 2014 David Edmundson <kde@davidedmundson.co.uk>
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "subdialog.h"

#include <QGuiApplication>
#include <QScreen>

SubDialog::SubDialog(QQuickItem *parent)
    : PlasmaQuick::Dialog(parent)
{
}

SubDialog::~SubDialog()
{
}

QPoint SubDialog::popupPosition(QQuickItem *item, const QSize &size)
{
    if (!item || !item->window()) {
        return QPoint(0, 0);
    }

    QPointF pos = item->mapToScene(QPointF(0, 0));
    pos = item->window()->mapToGlobal(pos.toPoint());

    pos.setX(pos.x() + item->width() / 2);
    pos.setY(pos.y() + item->height() / 2);

    if (QGuiApplication::layoutDirection() == Qt::RightToLeft) {
        pos.setX(pos.x() - size.width());
    }

    QRect avail = availableScreenRectForItem(item);

    if (pos.x() + size.width() > avail.right()) {
        pos.setX(pos.x() - size.width());
    }

    if (pos.x() < avail.left()) {
        pos.setX(pos.x() + size.width());
    }

    if (pos.y() + size.height() > avail.bottom()) {
        pos.setY(pos.y() - size.height());
    }

    return pos.toPoint();
}

QRect SubDialog::availableScreenRectForItem(QQuickItem *item) const
{
    QScreen *screen = QGuiApplication::primaryScreen();

    const QPoint globalPosition = item->window()->mapToGlobal(item->position().toPoint());

    foreach (QScreen *s, QGuiApplication::screens()) {
        if (s->geometry().contains(globalPosition)) {
            screen = s;
        }
    }

    return screen->availableGeometry();
}
