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
#ifndef _POSWIDGET_H_
#define _POSWIDGET_H_

#include <QWidget>
#include <QList>
class QPaintEvent;

/**
  Widget to display the joystick-selected (x,y) position
*/
class PosWidget : public QWidget
{
  Q_OBJECT

  public:
    PosWidget(QWidget *parent = 0);

    void changeX(int x);
    void changeY(int y);

    // define if a trace of the moving joystick shall be displayed
    // changing it will erase all previous marks from the widget
    void showTrace(bool t);

  protected:
    virtual void paintEvent(QPaintEvent *);

  private:
    int x, y;
    bool trace;
    QList<QPoint> tracePoints;
};

#endif
