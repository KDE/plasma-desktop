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

#ifndef DRAGHELPER_H
#define DRAGHELPER_H

#include <QObject>

class QIcon;
class QQuickItem;
class QUrl;

class DragHelper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int dragIconSize READ dragIconSize WRITE setDragIconSize NOTIFY dragIconSizeChanged)

public:
    explicit DragHelper(QObject *parent = nullptr);
    ~DragHelper() override;

    int dragIconSize() const;
    void setDragIconSize(int size);

    Q_INVOKABLE bool isDrag(int oldX, int oldY, int newX, int newY) const;
    Q_INVOKABLE void startDrag(QQuickItem *item, const QString &mimeType, const QVariant &mimeData, const QUrl &url, const QIcon &icon);

Q_SIGNALS:
    void dragIconSizeChanged() const;
    void dropped() const;

private Q_SLOTS:
    void startDragInternal(QQuickItem *item, const QString &mimeType, const QVariant &mimeData, const QUrl &url, const QIcon &icon) const;

private:
    int m_dragIconSize;
};

#endif
