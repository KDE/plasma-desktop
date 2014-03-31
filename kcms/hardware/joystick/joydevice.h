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
#ifndef _JOYDEVICE_H_
#define _JOYDEVICE_H_

#include <QString>

#include <sys/types.h>

#ifdef Q_OS_LINUX
#undef __STRICT_ANSI__
#include <linux/joystick.h>
#define __STRICT_ANSI__
#endif

#ifdef Q_OS_FREEBSD
#include <sys/joystick.h>
#endif

// helper class which holds all current values, file descriptor, etc. for
// one device
class JoyDevice
{
  public:
    enum ErrorCode
    {
      SUCCESS,
      OPEN_FAILED,
      NO_JOYSTICK,
      WRONG_VERSION,
      ERR_GET_VERSION,
      ERR_GET_BUTTONS,
      ERR_GET_AXES,
      ERR_GET_CORR,
      ERR_RESTORE_CORR,
      ERR_INIT_CAL,
      ERR_APPLY_CAL
    };

    enum EventType
    {
      BUTTON,
      AXIS
    };

    // devicefile to use, e.g. "/dev/js0"
    JoyDevice(const QString &devicefile);
    ~JoyDevice();

    // returns one of the error-codes from above
    ErrorCode open();

    // return descriptive error text for given error code
    QString errText(ErrorCode code) const;

    int fd() const { return joyFd; }
    void close();
    ErrorCode restoreCorr();

    // return devicefilename from constructor
    const QString &device() const { return devName; }

    // descriptive text for this device read from the driver
    QString text() const { return descr; }

    int numButtons() const { return buttons; }
    int numAxes() const { return axes; }
    int axisMin(int axis) const;
    int axisMax(int axis) const;

    // read next event from device; returns true if there was an event during the short timeout
    bool getEvent(JoyDevice::EventType &type, int &number, int &value);

    // methods for calibration
    ErrorCode initCalibration();  // must be called first
    void calcPrecision();

    void resetMinMax(int axis, int value = 0);

    // calculate correction values
    // min[2], center[2], max[2], index 0 == minimum, index 1 == maximum
    void calcCorrection(int axis, int *min, int *center, int *max);
    ErrorCode applyCalibration();

  private:
    QString devName;  // device filename
    QString descr;    // descriptive text
    int joyFd;

    int buttons;
    int axes;
    int *amin;  // axes min values
    int *amax;  // axes max values

    struct js_corr *corr;  // calibration values during the calib. steps
    struct js_corr *origCorr;  // original calibration correction values
};

#endif
