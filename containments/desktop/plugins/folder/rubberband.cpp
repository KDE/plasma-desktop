/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rubberband.h"

#include <QApplication>
#include <QStyleOptionRubberBand>

RubberBand::RubberBand(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

RubberBand::~RubberBand()
{
}

void RubberBand::paint(QPainter *painter)
{
    if (!qApp || !qApp->style()) {
        return;
    }

    QStyleOptionRubberBand opt;
    opt.state = QStyle::State_None;
    opt.direction = qApp->layoutDirection();
    opt.styleObject = this;
    opt.palette = qApp->palette();
    opt.shape = QRubberBand::Rectangle;
    opt.opaque = false;
    opt.rect = contentsBoundingRect().toRect();
    qApp->style()->drawControl(QStyle::CE_RubberBand, &opt, painter);
}

bool RubberBand::intersects(const QRectF &rect) const
{
    return m_geometry.intersects(rect);
}

void RubberBand::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_UNUSED(oldGeometry);

    m_geometry = newGeometry;

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}
