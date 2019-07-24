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
#include <QQuickWindow>
#include <QTimer>

DragHelper::DragHelper(QObject* parent) : QObject(parent)
, m_dragIconSize(32)
, m_dragging(false)
{
}

DragHelper::~DragHelper()
{
}

int DragHelper::dragIconSize() const
{
    return m_dragIconSize;
}

void DragHelper::setDragIconSize(int size)
{
    if (m_dragIconSize != size) {
        m_dragIconSize = size;

        emit dragIconSizeChanged();
    }
}

bool DragHelper::isDrag(int oldX, int oldY, int newX, int newY) const
{
    return ((QPoint(oldX, oldY) - QPoint(newX, newY)).manhattanLength() >= QApplication::startDragDistance());
}

void DragHelper::startDrag(QQuickItem *item, const QUrl &url, const QIcon &icon,
    const QString &extraMimeType, const QString &extraMimeData)
{
    // This allows the caller to return, making sure we don't crash if
    // the caller is destroyed mid-drag (as can happen due to a sycoca
    // change).

    QMetaObject::invokeMethod(this, "doDrag", Qt::QueuedConnection,
        Q_ARG(QQuickItem*, item), Q_ARG(QUrl, url), Q_ARG(QIcon, icon),
        Q_ARG(QString, extraMimeType), Q_ARG(QString, extraMimeData));
}

void DragHelper::doDrag(QQuickItem *item, const QUrl &url, const QIcon &icon,
    const QString &extraMimeType, const QString &extraMimeData)
{
    setDragging(true);

    if (item && item->window() && item->window()->mouseGrabberItem()) {
        item->window()->mouseGrabberItem()->ungrabMouse();
    }

    QDrag *drag = new QDrag(item);

    QMimeData *mimeData = new QMimeData();

    if (!url.isEmpty()) {
        mimeData->setUrls(QList<QUrl>() << url);
    }

    if (!extraMimeType.isEmpty() && !extraMimeData.isEmpty()) {
        mimeData->setData(extraMimeType, extraMimeData.toLatin1());
    }

    drag->setMimeData(mimeData);

    if (!icon.isNull()) {
        drag->setPixmap(icon.pixmap(m_dragIconSize, m_dragIconSize));
    }

    drag->exec();

    emit dropped();

    // Ensure dragging is still true when onRelease is called.
    QTimer::singleShot(0, qApp, [this] {
        setDragging(false);
    });
}

void DragHelper::setDragging(bool dragging)
{
    if (m_dragging == dragging)
        return;
    m_dragging = dragging;
    emit draggingChanged();
}