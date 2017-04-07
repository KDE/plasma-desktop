/***************************************************************************
 *   Copyright (C) 2014 by David Edmundson <kde@davidedmundson.co.uk>      *
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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

#include "subdialog.h"

#include <QGuiApplication>
#include <QScreen>

SubDialog::SubDialog(QQuickItem *parent) : PlasmaQuick::Dialog(parent)
{
}

SubDialog::~SubDialog()
{
}

QPoint SubDialog::popupPosition(QQuickItem* item, const QSize& size)
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

    foreach(QScreen *s, QGuiApplication::screens()) {
        if (s->geometry().contains(globalPosition)) {
            screen = s;
        }
    }

    return screen->availableGeometry();
}
