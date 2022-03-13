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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void RubberBand::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
#else
void RubberBand::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
#endif
{
    Q_UNUSED(oldGeometry);

    m_geometry = newGeometry;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
#else
    QQuickItem::geometryChange(newGeometry, oldGeometry);
#endif
}
