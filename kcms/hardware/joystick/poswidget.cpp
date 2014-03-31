/***************************************************************************
 *   Copyright (C) 2003,2005,2006 by Martin Koller                         *
 *   kollix@aon.at                                                         *
 *   This file is part of the KDE Control Center Module for Joysticks      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "poswidget.h"

#include <QPainter>

#define XY_WIDTH 220
#define MARK_WIDTH 10
#define MAX_POINTS 500

//-----------------------------------------------------------------

PosWidget::PosWidget(QWidget *parent)
  : QWidget(parent), x(0), y(0), trace(false)
{
  setMinimumSize(XY_WIDTH, XY_WIDTH);
  setMaximumSize(XY_WIDTH, XY_WIDTH);

  QPalette palette;
  palette.setColor(backgroundRole(), Qt::white);
  setPalette(palette);
}

//-----------------------------------------------------------------

void PosWidget::paintEvent(QPaintEvent *)
{
  QPainter paint(this);

  // draw a frame
  paint.drawRect(0, 0, width()-1, height()-1);
  paint.setPen(Qt::gray);

  // draw a center grid
  paint.drawLine(XY_WIDTH/2, 1,
                 XY_WIDTH/2, XY_WIDTH - 2);

  paint.drawLine(1,           XY_WIDTH/2,
                 XY_WIDTH - 2, XY_WIDTH/2);

  // draw the trace of previous points
  if ( trace )
  {
    paint.setPen(Qt::black);

    for (int i = 0; i < tracePoints.count()-2; i++)
      paint.drawLine(tracePoints[i], tracePoints[i+1]);

    if ( tracePoints.count() > 0 )
      paint.drawLine(tracePoints[tracePoints.count()-1], QPoint(x, y));
  }

  // draw the current position marker
  paint.setPen(Qt::blue);

  paint.drawLine(x - MARK_WIDTH/2, y - MARK_WIDTH/2,
                 x + MARK_WIDTH/2, y + MARK_WIDTH/2);

  paint.drawLine(x - MARK_WIDTH/2, y + MARK_WIDTH/2,
                 x + MARK_WIDTH/2, y - MARK_WIDTH/2);
}

//-----------------------------------------------------------------

void PosWidget::changeX(int newX)
{
  // transform coordinates from joystick to widget coordinates
  newX = int((newX/65535.0)*XY_WIDTH + XY_WIDTH/2);

  if ( x == newX ) return;  // avoid unnecessary redraw

  if ( trace )
  {
    tracePoints.append(QPoint(x, y));
    if ( tracePoints.count() == MAX_POINTS )
      tracePoints.removeFirst();
  }

  x = newX;
  update();
}

//-----------------------------------------------------------------

void PosWidget::changeY(int newY)
{
  // transform coordinates from joystick to widget coordinates
  newY = int((newY/65535.0)*XY_WIDTH + XY_WIDTH/2);

  if ( y == newY ) return;  // avoid unnecessary redraw

  if ( trace )
  {
    tracePoints.append(QPoint(x, y));
    if ( tracePoints.count() == MAX_POINTS )
      tracePoints.removeFirst();
  }

  y = newY;
  update();
}

//-----------------------------------------------------------------

void PosWidget::showTrace(bool t)
{
  trace = t;
  tracePoints.clear();

  update();
}

//-----------------------------------------------------------------

#include "poswidget.moc"
