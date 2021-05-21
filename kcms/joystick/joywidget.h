/*
    This file is part of the KDE Control Center Module for Joysticks

    SPDX-FileCopyrightText: 2003, 2005, 2006 Martin Koller <kollix@aon.at>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _JOYWIDGET_H_
#define _JOYWIDGET_H_

#include <QWidget>

class JoyDevice;

class PosWidget;
class QTableWidget;
class QTimer;
class KComboBox;
class KMessageWidget;
class QPushButton;
class QCheckBox;

// the widget which displays all buttons, values, etc.
class JoyWidget : public QWidget
{
    Q_OBJECT

public:
    JoyWidget(QWidget *parent = nullptr);

    ~JoyWidget() override;

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
    void showDeviceProps(JoyDevice *joy); // fill widgets with given device parameters
    void restoreCurrDev(); // restores the content of the combobox to reflect the current open device

private:
    KMessageWidget *messageBox; // in case of no device, show here a message rather than in a dialog
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
