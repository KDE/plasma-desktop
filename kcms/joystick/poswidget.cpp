/*
    This file is part of the KDE Control Center Module for Joysticks

    SPDX-FileCopyrightText: 2003, 2005, 2006 Martin Koller <kollix@aon.at>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "poswidget.h"

#include <QPainter>

#define XY_WIDTH 220
#define MARK_WIDTH 10
#define MAX_POINTS 500

PosWidget::PosWidget(QWidget *parent)
    : QWidget(parent)
    , x(0)
    , y(0)
    , trace(false)
{
    setMinimumSize(XY_WIDTH, XY_WIDTH);
    setMaximumSize(XY_WIDTH, XY_WIDTH);

    QPalette palette;
    palette.setColor(backgroundRole(), Qt::white);
    setPalette(palette);
}

void PosWidget::paintEvent(QPaintEvent *)
{
    QPainter paint(this);

    // draw a frame
    paint.drawRect(0, 0, width() - 1, height() - 1);
    paint.setPen(Qt::gray);

    // draw a center grid
    paint.drawLine(XY_WIDTH / 2, 1, XY_WIDTH / 2, XY_WIDTH - 2);

    paint.drawLine(1, XY_WIDTH / 2, XY_WIDTH - 2, XY_WIDTH / 2);

    // draw the trace of previous points
    if (trace) {
        paint.setPen(Qt::black);

        for (int i = 0; i < tracePoints.count() - 2; i++)
            paint.drawLine(tracePoints[i], tracePoints[i + 1]);

        if (!tracePoints.isEmpty())
            paint.drawLine(tracePoints[tracePoints.count() - 1], QPoint(x, y));
    }

    // draw the current position marker
    paint.setPen(Qt::blue);

    paint.drawLine(x - MARK_WIDTH / 2, y - MARK_WIDTH / 2, x + MARK_WIDTH / 2, y + MARK_WIDTH / 2);

    paint.drawLine(x - MARK_WIDTH / 2, y + MARK_WIDTH / 2, x + MARK_WIDTH / 2, y - MARK_WIDTH / 2);
}

void PosWidget::changeX(int newX)
{
    // transform coordinates from joystick to widget coordinates
    newX = int((newX / 65535.0) * XY_WIDTH + XY_WIDTH / 2);

    if (x == newX)
        return; // avoid unnecessary redraw

    if (trace) {
        tracePoints.append(QPoint(x, y));
        if (tracePoints.count() == MAX_POINTS)
            tracePoints.removeFirst();
    }

    x = newX;
    update();
}

void PosWidget::changeY(int newY)
{
    // transform coordinates from joystick to widget coordinates
    newY = int((newY / 65535.0) * XY_WIDTH + XY_WIDTH / 2);

    if (y == newY)
        return; // avoid unnecessary redraw

    if (trace) {
        tracePoints.append(QPoint(x, y));
        if (tracePoints.count() == MAX_POINTS)
            tracePoints.removeFirst();
    }

    y = newY;
    update();
}

void PosWidget::showTrace(bool t)
{
    trace = t;
    tracePoints.clear();

    update();
}
