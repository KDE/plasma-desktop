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

#ifndef SUBMENU_H
#define SUBMENU_H

#include <PlasmaQuick/Dialog>


class SubMenu : public PlasmaQuick::Dialog
{
    Q_OBJECT

    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(bool facingLeft READ facingLeft NOTIFY facingLeftChanged)

    public:
        explicit SubMenu(QQuickItem *parent = nullptr);
        ~SubMenu() override;

        Q_INVOKABLE QRect availableScreenRectForItem(QQuickItem *item) const;

        QPoint popupPosition(QQuickItem *item, const QSize &size) override;

        int offset() const;
        void setOffset(int offset);

        bool facingLeft() const { return m_facingLeft; }

    Q_SIGNALS:
        void offsetChanged() const;
        void facingLeftChanged() const;

    private:
        int m_offset;
        bool m_facingLeft;
};

#endif
