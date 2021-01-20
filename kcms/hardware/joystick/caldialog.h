/***************************************************************************
 *   Copyright (C) 2003 by Martin Koller                                   *
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
#ifndef _CALDIALOG_H_
#define _CALDIALOG_H_

#include <QLabel>

#include <QDialog>

class JoyDevice;

// the dialog which tells the user all steps to calibrate the device

class CalDialog : public QDialog
{
    Q_OBJECT

public:
    CalDialog(QWidget *parent, JoyDevice *joy);

    void calibrate();

private:
    void waitButton(int axis, bool press, int &lastVal);

private Q_SLOTS:
    virtual void slotNext();

private:
    JoyDevice *joydev;

    QLabel *text;
    QLabel *valueLbl;
};

#endif
