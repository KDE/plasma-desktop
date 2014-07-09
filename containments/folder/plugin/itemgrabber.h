/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                   *
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

#ifndef ITEMGRABBER_H
#define ITEMGRABBER_H

#include <QImage>
#include <QObject>
#include <QPointer>
#include <QQuickItem>

class ItemGrabber : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool null READ null NOTIFY nullChanged)
    Q_PROPERTY(QImage image READ image NOTIFY imageChanged)
    Q_PROPERTY(QQuickItem* item READ item WRITE setItem NOTIFY itemChanged)

    public:
        ItemGrabber(QObject *parent = 0);
        ~ItemGrabber();

        bool null() const;

        QImage image() const;

        QQuickItem *item() const;
        void setItem(QQuickItem *item);

    Q_SIGNALS:
        void nullChanged() const;
        void imageChanged() const;
        void itemChanged() const;

    private Q_SLOTS:
        void grab();

    private:
        QPointer<QQuickItem> m_item;
        QImage m_image;
};

#endif
