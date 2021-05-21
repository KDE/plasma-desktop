/*
    This file is part of the KDE Control Center Module for Joysticks

    SPDX-FileCopyrightText: 2003, 2005, 2006 Martin Koller <kollix@aon.at>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _POSWIDGET_H_
#define _POSWIDGET_H_

#include <QList>
#include <QWidget>
class QPaintEvent;

/**
  Widget to display the joystick-selected (x,y) position
*/
class PosWidget : public QWidget
{
    Q_OBJECT

public:
    PosWidget(QWidget *parent = nullptr);

    void changeX(int x);
    void changeY(int y);

    // define if a trace of the moving joystick shall be displayed
    // changing it will erase all previous marks from the widget
    void showTrace(bool t);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    int x, y;
    bool trace;
    QList<QPoint> tracePoints;
};

#endif
