/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RUBBERBAND_H
#define RUBBERBAND_H

#include <QQuickPaintedItem>

class RubberBand : public QQuickPaintedItem
{
    Q_OBJECT

public:
    explicit RubberBand(QQuickItem *parent = nullptr);
    ~RubberBand() override;

    void paint(QPainter *painter) override;

    Q_INVOKABLE bool intersects(const QRectF &rect) const;

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    QRectF m_geometry;
};

#endif
