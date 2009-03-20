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
#ifndef KIMPANELLAYOUT_H
#define KIMPANELLAYOUT_H

#include <plasma/applet.h>
#include <plasma/widgets/iconwidget.h>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QGraphicsLayoutItem>
#include <QList>

class KIMPanelLayout : public QGraphicsLayout
{
public:
    KIMPanelLayout(QGraphicsLayoutItem *parent);
    void addItem(Plasma::IconWidget *icon);
    void addItems(const QList<Plasma::IconWidget *> &icons);
    void setItems(const QList<Plasma::IconWidget *> &icons);

    Plasma::IconWidget* itemAt(int i) const;
    void removeAt(int i);

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

    int count() const 
    {
        return m_icons.size();
    }

    void setGeometry(const QRectF &rect);
//X     void updateGeometry();

private:
    void smartLayout(const QRectF &rect);

private:
    QList<Plasma::IconWidget *> m_icons;

    QRectF m_cachedGeometry;

};

#endif // KIMPANELLAYOUT_H
