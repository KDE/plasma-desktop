/***************************************************************************
 *   Copyright (C) 2013 by Eike Hein <hein@kde.org>                        *
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

#include "draghelper.h"

#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QQuickItem>

DragHelper::DragHelper(QObject* parent) : QObject(parent)
{
}

DragHelper::~DragHelper()
{
}

bool DragHelper::isDrag(int oldX, int oldY, int newX, int newY) const
{
    return ((QPoint(oldX, oldY) - QPoint(newX, newY)).manhattanLength() >= QApplication::startDragDistance());
}

void DragHelper::startDrag(QQuickItem *item, const QUrl &url)
{
    // This allows the caller to return, making sure we don't crash if
    // the caller is destroyed mid-drag (as can happen due to a sycoca
    // change).

    QMetaObject::invokeMethod(this, "doDrag", Qt::QueuedConnection, Q_ARG(QQuickItem*, item), Q_ARG(QUrl, url));
}

void DragHelper::doDrag(QQuickItem *item, const QUrl &url) const
{
    QDrag *drag = new QDrag(item);

    QMimeData *mimeData = new QMimeData();

    if (!url.isEmpty()) {
        mimeData->setUrls(QList<QUrl>() << url);
    }

    drag->setMimeData(mimeData);

    drag->exec();

    emit dropped();
}

