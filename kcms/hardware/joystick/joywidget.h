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
#ifndef _JOYWIDGET_H_
#define _JOYWIDGET_H_

#include <QWidget>

class JoyDevice;

class PosWidget;
class QLabel;
class QTableWidget;
class QTimer;
class KComboBox;
class KMessageWidget;
class QPushButton;
class QCheckBox;
class QFrame;

// the widget which displays all buttons, values, etc.
class JoyWidget : public QWidget
{
  Q_OBJECT
  
  public:
    JoyWidget(QWidget *parent = 0);

    ~JoyWidget();

    // initialize list of possible devices and open the first available
    void init();

  public Q_SLOTS:
    // reset calibration values to their value when this KCM was started
    void resetCalibration();

  private Q_SLOTS:
    void checkDevice();
    void deviceChanged(const QString &dev);
    void traceChanged(bool);
    void calibrateDevice();

  private:
    void showDeviceProps(JoyDevice *joy);  // fill widgets with given device parameters
    void restoreCurrDev(); // restores the content of the combobox to reflect the current open device

  private:
    KMessageWidget *messageBox;  // in case of no device, show here a message rather than in a dialog
    KComboBox *device;
    PosWidget *xyPos;
    QTableWidget *buttonTbl;
    QTableWidget *axesTbl;
    QCheckBox *trace;
    QPushButton *calibrate;

    QTimer *idle;

    JoyDevice *joydev;
};

#endif
