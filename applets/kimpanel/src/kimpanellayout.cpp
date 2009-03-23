/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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

#include "kimpanellayout.h"
#include <kiconloader.h>

KIMPanelLayout::KIMPanelLayout(QGraphicsLayoutItem *parent) : QGraphicsLayout(parent) 
{
}

void KIMPanelLayout::addItem(Plasma::IconWidget *icon)
{
    m_icons << icon;
//    QGraphicsGridLayout::addItem(icon,0,count());
    updateGeometry();
    //smartLayout(m_cachedGeometry);
}

void KIMPanelLayout::addItems(const QList<Plasma::IconWidget *> &icons)
{
    m_icons << icons;
    updateGeometry();
    //smartLayout(m_cachedGeometry);
}

void KIMPanelLayout::setItems(const QList<Plasma::IconWidget *> &icons)
{
    m_icons = icons;
    updateGeometry();
    //smartLayout(m_cachedGeometry);
}

Plasma::IconWidget* KIMPanelLayout::itemAt(int i) const
{
    return m_icons.at(i);
}

void KIMPanelLayout::removeAt(int i)
{
    m_icons.removeAt(i);
//X     smartLayout(m_cachedGeometry);
    updateGeometry();
}

QSizeF KIMPanelLayout::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    Q_UNUSED(constraint)

    QSizeF sizeHint;
    switch (which) {
    case Qt::MinimumSize:
        sizeHint = QSizeF(KIconLoader::SizeSmall,KIconLoader::SizeSmall);
        break;
    case Qt::PreferredSize:
        sizeHint = m_cachedGeometry.size();
        break;
    case Qt::MaximumSize:
        return QSizeF(-1.0,-1.0);
        break;
    default:
        ;
    }
    return sizeHint;
}

void KIMPanelLayout::setGeometry(const QRectF &rect)
{
    smartLayout(rect);
    m_cachedGeometry = rect;
}

#if 0
void KIMPanelLayout::updateGeometry()
{
    kDebug() << "";
    QGraphicsGridLayout::updateGeometry();
}
#endif

void KIMPanelLayout::smartLayout(const QRectF &rect)
{
    if (m_icons.size() > 0) {
        qreal w;
        qreal h;
        int row;
        int best_row = 1;
        int best_col = 1;
        qreal max_w = -1;
        for (int col = 1; col <= m_icons.size(); col++) {
            w = rect.width()/(qreal)col;
            row = (m_icons.size() + col - 1) / (qreal)col;
            h = rect.height()/(qreal)row;

            if (qMin(w,h) >= max_w) {
                max_w = qMin(w,h);
                best_col = col;
                best_row = row;
            }
        }
        // do the layout
        int r = 0;
        int c = 0;
        qreal spacing_w = (rect.width()- best_col * max_w)/best_col;
        qreal spacing_h = (rect.height()- best_row * max_w)/best_row;
//X         kDebug() << "Spacing:" << spacing_h << spacing_w;
        foreach (Plasma::IconWidget *icon, m_icons) {
            QRectF new_geometry;
            new_geometry.setWidth(max_w);
            new_geometry.setHeight(max_w);
            new_geometry.moveTop(spacing_h/2 + r*(max_w + spacing_h) + rect.top());
            new_geometry.moveLeft(spacing_w/2 + c*(max_w + spacing_w) + rect.left());
            icon->setGeometry(new_geometry);
            c++;
            if (c>=best_col) {
                c=0;
                r++;
            }
        }
    }
}

// vim: sw=4 sts=4 et tw=100
