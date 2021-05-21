/*
    This file is part of the KDE Control Center Module for Joysticks

    SPDX-FileCopyrightText: 2003 Martin Koller <kollix@aon.at>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
