/*
    SPDX-FileCopyrightText: 2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
