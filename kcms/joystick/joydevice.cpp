/*
    This file is part of the KDE Control Center Module for Joysticks

    SPDX-FileCopyrightText: 2003 Martin Koller <kollix@aon.at>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joydevice.h"

#include <KLocalizedString>
#include <QDebug>

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

JoyDevice::JoyDevice(const QString &devicefile)
    : devName(devicefile)
    , joyFd(-1)
    , buttons(0)
    , axes(0)
    , amin(nullptr)
    , amax(nullptr)
    , corr(nullptr)
    , origCorr(nullptr)
{
}

QString JoyDevice::errText(ErrorCode code) const
{
    switch (code) {
    case SUCCESS:
        return QLatin1String("");

    case OPEN_FAILED: {
        return i18n("The given device %1 could not be opened: %2", devName, strerror(errno));
    }

    case NO_JOYSTICK: {
        return i18n("The given device %1 is not a joystick.", devName);
    }

    case ERR_GET_VERSION: {
        return i18n("Could not get kernel driver version for joystick device %1: %2", devName, strerror(errno));
    }

    case WRONG_VERSION: {
        int version = 0;
        int fd = ::open(devName.toLatin1(), O_RDONLY);
        if (fd != -1) {
            ::ioctl(fd, JSIOCGVERSION, &version);
            ::close(fd);
        }

        KLocalizedString loc = ki18n("The current running kernel driver version (%1.%2.%3) is not the one this module was compiled for (%4.%5.%6).");
        loc = loc.subs(version >> 16);
        loc = loc.subs((version >> 8) & 0xFF);
        loc = loc.subs(version & 0xFF);
        loc = loc.subs(JS_VERSION >> 16);
        loc = loc.subs((JS_VERSION >> 8) & 0xFF);
        loc = loc.subs(JS_VERSION & 0xFF);
        return loc.toString();
    }

    case ERR_GET_BUTTONS: {
        return i18n("Could not get number of buttons for joystick device %1: %2", devName, strerror(errno));
    }

    case ERR_GET_AXES: {
        return i18n("Could not get number of axes for joystick device %1: %2", devName, strerror(errno));
    }

    case ERR_GET_CORR: {
        return i18n("Could not get calibration values for joystick device %1: %2", devName, strerror(errno));
    }

    case ERR_RESTORE_CORR: {
        return i18n("Could not restore calibration values for joystick device %1: %2", devName, strerror(errno));
    }

    case ERR_INIT_CAL: {
        return i18n("Could not initialize calibration values for joystick device %1: %2", devName, strerror(errno));
    }

    case ERR_APPLY_CAL: {
        return i18n("Could not apply calibration values for joystick device %1: %2", devName, strerror(errno));
    }

    default:
        return i18n("internal error - code %1 unknown", int(code));
    }
}

JoyDevice::ErrorCode JoyDevice::open()
{
    if (joyFd != -1)
        return JoyDevice::SUCCESS; // already open

    int fd = ::open(devName.toLatin1(), O_RDONLY);

    if (fd == -1)
        return JoyDevice::OPEN_FAILED;

    // we could open the devicefile, now check if a joystick is attached
    char name[128];

    if (::ioctl(fd, JSIOCGNAME(sizeof(name)), &name) == -1) {
        ::close(fd);
        return JoyDevice::NO_JOYSTICK;
    }

    // check the kernel driver version
    int version;
    if (::ioctl(fd, JSIOCGVERSION, &version) == -1) {
        ::close(fd);
        return JoyDevice::ERR_GET_VERSION;
    }

    if (version != JS_VERSION) {
        ::close(fd);
        return JoyDevice::WRONG_VERSION;
    }

    char bt = 0, ax = 0;
    if (::ioctl(fd, JSIOCGBUTTONS, &bt) == -1) {
        ::close(fd);
        return JoyDevice::ERR_GET_BUTTONS;
    }

    if (::ioctl(fd, JSIOCGAXES, &ax) == -1) {
        ::close(fd);
        return JoyDevice::ERR_GET_AXES;
    }

    struct js_corr *oldCorr = new struct js_corr[ax];

    if (::ioctl(fd, JSIOCGCORR, oldCorr) == -1) {
        ::close(fd);
        delete[] oldCorr;
        return JoyDevice::ERR_GET_CORR;
    }

    if (bt < 0) {
        ::close(fd);
        delete[] oldCorr;
        return JoyDevice::ERR_GET_BUTTONS;
    }

    descr = name;
    joyFd = fd;
    axes = ax;
    buttons = bt;
    origCorr = oldCorr;
    corr = new struct js_corr[axes];

    amin = new int[axes];
    amax = new int[axes];

    int i;

    for (i = 0; i < axes; i++)
        resetMinMax(i);

    return JoyDevice::SUCCESS;
}

void JoyDevice::close()
{
    if (joyFd == -1)
        return;

    ::close(joyFd);

    joyFd = -1;
    descr = QLatin1String("");

    delete[] amin;
    delete[] amax;
    amin = nullptr;
    amax = nullptr;

    delete[] corr;
    corr = nullptr;
    delete[] origCorr;
    origCorr = nullptr;
}

int JoyDevice::axisMin(int axis) const
{
    if ((axis < 0) || (axis >= axes))
        return 0;

    return amin[axis];
}

int JoyDevice::axisMax(int axis) const
{
    if ((axis < 0) || (axis >= axes))
        return 0;

    return amax[axis];
}

JoyDevice::ErrorCode JoyDevice::initCalibration()
{
    if (joyFd == -1)
        return JoyDevice::ERR_INIT_CAL;

    int i;

    // Reset all current correction values
    for (i = 0; i < axes; i++) {
        corr[i].type = JS_CORR_NONE;
        corr[i].prec = 0;
    }

    if (::ioctl(joyFd, JSIOCSCORR, corr) == -1)
        return JoyDevice::ERR_INIT_CAL;

    for (i = 0; i < axes; i++)
        corr[i].type = JS_CORR_BROKEN;

    return JoyDevice::SUCCESS;
}

JoyDevice::ErrorCode JoyDevice::applyCalibration()
{
    if (joyFd == -1)
        return JoyDevice::ERR_APPLY_CAL;

    if (::ioctl(joyFd, JSIOCSCORR, corr) == -1)
        return JoyDevice::ERR_APPLY_CAL;

    return JoyDevice::SUCCESS;
}

void JoyDevice::resetMinMax(int axis, int value)
{
    amin[axis] = value;
    amax[axis] = value;
}

void JoyDevice::calcPrecision()
{
    if (!corr)
        return;

    int i;

    for (i = 0; i < axes; i++) {
        corr[i].prec = amax[i] - amin[i];
        qDebug() << "Precision for axis: " << i << ": " << corr[i].prec;
    }
}

JoyDevice::ErrorCode JoyDevice::restoreCorr()
{
    if (joyFd == -1)
        return JoyDevice::SUCCESS;

    if (::ioctl(joyFd, JSIOCSCORR, origCorr) == -1)
        return JoyDevice::ERR_RESTORE_CORR;
    else
        return JoyDevice::SUCCESS;
}

JoyDevice::~JoyDevice()
{
    close();
}

bool JoyDevice::getEvent(JoyDevice::EventType &type, int &number, int &value)
{
    number = value = 0;

    int ret;

    fd_set readSet;

    FD_ZERO(&readSet);
    FD_SET(joyFd, &readSet);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    ret = ::select(joyFd + 1, &readSet, nullptr, nullptr, &timeout);

    if (ret == 1) // got an event from the joystick
    {
        struct js_event e;

        if (::read(joyFd, &e, sizeof(struct js_event)) == sizeof(struct js_event)) {
            if (e.type & JS_EVENT_BUTTON) {
                type = JoyDevice::BUTTON;
                value = e.value;
                number = e.number;

                return true;
            }

            if (e.type & JS_EVENT_AXIS) {
                type = JoyDevice::AXIS;
                value = e.value;
                number = e.number;

                // store min, max values
                if (e.value < amin[number])
                    amin[number] = e.value;
                if (e.value > amax[number])
                    amax[number] = e.value;

                return true;
            }
        }
    }

    return false; // no event
}

void JoyDevice::calcCorrection(int axis, int *min, int *center, int *max)
{
    const int MIN = 0;
    const int MAX = 1;

    double a, b, c, d;

    a = center[MIN]; // inputs.cmin[1];
    b = center[MAX]; // inputs.cmax[1];
    c = 32767.0 / (center[MIN] - min[MAX]); // (inputs.cmin[1] - inputs.cmax[0]);
    d = 32767.0 / (max[MIN] - center[MAX]); // (inputs.cmin[2] - inputs.cmax[1]);

    corr[axis].coef[0] = (int)rint(a);
    corr[axis].coef[1] = (int)rint(b);
    corr[axis].coef[2] = (int)rint(c * 16384.0);
    corr[axis].coef[3] = (int)rint(d * 16384.0);

    qDebug() << "min min: " << min[0] << " max: " << min[1];
    qDebug() << "max min: " << max[0] << " max: " << max[1];
    qDebug() << "Correction values for axis: " << axis << ": " << corr[axis].coef[0] << ", " << corr[axis].coef[1] << ", " << corr[axis].coef[2] << ", "
             << corr[axis].coef[3] << Qt::endl;
}
