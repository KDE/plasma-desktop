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

#include "submenu.h"

#include <cmath>

#include <QScreen>

#include <KWindowSystem>

SubMenu::SubMenu(QQuickItem *parent) : PlasmaQuick::Dialog(parent)
, m_offset(0)
, m_facingLeft(false)
{
   KWindowSystem::setType(winId(), NET::Menu);
}

SubMenu::~SubMenu()
{
}

int SubMenu::offset() const
{
    return m_offset;
}

void SubMenu::setOffset(int offset)
{
    if (m_offset != offset) {
        m_offset = offset;

        emit offsetChanged();
    }
}

QPoint SubMenu::popupPosition(QQuickItem* item, const QSize& size)
{
    if (!item || !item->window()) {
        return QPoint(0, 0);
    }

    QPointF pos = item->mapToScene(QPointF(0, 0));
    pos = item->window()->mapToGlobal(pos.toPoint());

    pos.setX(pos.x() + m_offset + item->width());

    QRect avail = availableScreenRectForItem(item);

    if (pos.x() + size.width() > avail.right()) {
        pos.setX(pos.x() - m_offset - item->width() - size.width());

        m_facingLeft = true;
        emit facingLeftChanged();
    }

    pos.setY(pos.y() - margins()->property("top").toInt());

    if (pos.y() + size.height() > avail.bottom()) {
       int overshoot = std::ceil(((avail.bottom() - (pos.y() + size.height())) * -1) / item->height()) * item->height();

        pos.setY(pos.y() - overshoot);
    }

    return pos.toPoint();
}

QRect SubMenu::availableScreenRectForItem(QQuickItem *item) const
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
