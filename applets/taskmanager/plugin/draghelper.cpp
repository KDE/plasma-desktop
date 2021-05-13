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
#include <QIcon>
#include <QMimeData>
#include <QPixmap>
#include <QPointer>
#include <QQuickItem>

#include "backend.h"

DragHelper::DragHelper(QObject *parent)
    : QObject(parent)
    , m_dragIconSize(32)
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

        Q_EMIT dragIconSizeChanged();
    }
}

bool DragHelper::isDrag(int oldX, int oldY, int newX, int newY) const
{
    return ((QPoint(oldX, oldY) - QPoint(newX, newY)).manhattanLength() >= QApplication::startDragDistance());
}

void DragHelper::startDrag(QQuickItem *item, const QString &mimeType, const QVariant &mimeData, const QUrl &url, const QIcon &icon)
{
    QMetaObject::invokeMethod(this,
                              "startDragInternal",
                              Qt::QueuedConnection,
                              Q_ARG(QQuickItem *, item),
                              Q_ARG(QString, mimeType),
                              Q_ARG(QVariant, mimeData),
                              Q_ARG(QUrl, url),
                              Q_ARG(QIcon, icon));
}

void DragHelper::startDragInternal(QQuickItem *item, const QString &mimeType, const QVariant &mimeData, const QUrl &url, const QIcon &icon) const
{
    QPointer<QQuickItem> grabber = item;

    QMimeData *dragData = new QMimeData();
    const QByteArray &taskUrlData = Backend::tryDecodeApplicationsUrl(url).toString().toUtf8();
    dragData->setData(QStringLiteral("text/x-orgkdeplasmataskmanager_taskurl"), taskUrlData);
    dragData->setData(mimeType, mimeData.toByteArray());
    dragData->setData(QStringLiteral("application/x-orgkdeplasmataskmanager_taskbuttonitem"), mimeData.toByteArray());

    QDrag *drag = new QDrag(static_cast<QQuickItem *>(parent()));
    drag->setMimeData(dragData);
    drag->setPixmap(icon.pixmap(QSize(m_dragIconSize, m_dragIconSize)));

    grabber->grabMouse();

    drag->exec();

    if (grabber) {
        grabber->ungrabMouse();
    }

    Q_EMIT dropped();
}
