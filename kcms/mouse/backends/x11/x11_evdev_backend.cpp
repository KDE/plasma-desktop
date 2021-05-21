/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "x11_evdev_backend.h"

#include <config-X11.h>

#include <KLocalizedString>
#include <QFile>
#include <evdev-properties.h>

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput2.h>
#ifdef HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/XInput.h>
#endif

struct ScopedXDeleter {
    static inline void cleanup(void *pointer)
    {
        if (pointer) {
            XFree(pointer);
        }
    }
};

template<typename Callback>
static void XIForallPointerDevices(Display *dpy, const Callback &callback)
{
    int ndevices_return;
    XDeviceInfo *info = XListInputDevices(dpy, &ndevices_return);
    if (!info) {
        return;
    }
    for (int i = 0; i < ndevices_return; ++i) {
        if (info[i].use == IsXPointer || info[i].use == IsXExtensionPointer) {
            callback(info + i);
        }
    }
    XFreeDeviceList(info);
}

X11EvdevBackend::X11EvdevBackend(QObject *parent)
    : X11Backend(parent)
{
    m_mode = InputBackendMode::XEvdev;
    m_settings = new EvdevSettings();
    initAtom();
}

void X11EvdevBackend::initAtom()
{
    if (!m_dpy) {
        return;
    }

    m_evdevScrollDistanceAtom = XInternAtom(m_dpy, EVDEV_PROP_SCROLL_DISTANCE, True);
    m_evdevWheelEmulationAtom = XInternAtom(m_dpy, EVDEV_PROP_WHEEL, True);
    m_evdevWheelEmulationAxesAtom = XInternAtom(m_dpy, EVDEV_PROP_WHEEL_AXES, True);

    m_touchpadAtom = XInternAtom(m_dpy, XI_TOUCHPAD, True);
}

X11EvdevBackend::~X11EvdevBackend()
{
    delete m_settings;
}

bool X11EvdevBackend::supportScrollPolarity()
{
    return m_numButtons >= 5;
}

double X11EvdevBackend::accelRate()
{
    return m_accelRate;
}

Handed X11EvdevBackend::handed()
{
    return m_handed;
}

int X11EvdevBackend::threshold()
{
    return m_threshold;
}

void X11EvdevBackend::load()
{
    if (!m_dpy) {
        return;
    }

    m_accelRate = 1.0;
    int accel_num, accel_den;
    XGetPointerControl(m_dpy, &accel_num, &accel_den, &m_threshold);
    m_accelRate = double(accel_num) / double(accel_den);

    // get settings from X server
    unsigned char map[256];
    m_numButtons = XGetPointerMapping(m_dpy, map, 256);
    m_middleButton = -1;

    m_handed = Handed::NotSupported;
    // ## keep this in sync with KGlobalSettings::mouseSettings
    if (m_numButtons == 2) {
        if (map[0] == 1 && map[1] == 2) {
            m_handed = Handed::Right;
        } else if (map[0] == 2 && map[1] == 1) {
            m_handed = Handed::Left;
        }
    } else if (m_numButtons >= 3) {
        m_middleButton = map[1];
        if (map[0] == 1 && map[2] == 3) {
            m_handed = Handed::Right;
        } else if (map[0] == 3 && map[2] == 1) {
            m_handed = Handed::Left;
        }
    }

    m_settings->load(this);
}

void X11EvdevBackend::apply(bool force)
{
    // 256 might seems extreme, but X has already been known to return 32,
    // and we don't want to truncate things. Xlib limits the table to 256 bytes,
    // so it's a good upper bound..
    unsigned char map[256];
    XGetPointerMapping(m_dpy, map, 256);

    if (m_settings->handedEnabled && (m_settings->handedNeedsApply || force)) {
        if (m_numButtons == 1) {
            map[0] = (unsigned char)1;
        } else if (m_numButtons == 2) {
            if (m_settings->handed == Handed::Right) {
                map[0] = (unsigned char)1;
                map[1] = (unsigned char)3;
            } else {
                map[0] = (unsigned char)3;
                map[1] = (unsigned char)1;
            }
        } else { // 3 buttons and more
            if (m_settings->handed == Handed::Right) {
                map[0] = (unsigned char)1;
                map[1] = (unsigned char)m_middleButton;
                map[2] = (unsigned char)3;
            } else {
                map[0] = (unsigned char)3;
                map[1] = (unsigned char)m_middleButton;
                map[2] = (unsigned char)1;
            }
        }

        int retval;
        if (m_numButtons >= 1) {
            while ((retval = XSetPointerMapping(m_dpy, map, m_numButtons)) == MappingBusy)
            /* keep trying until the pointer is free */
            { };
        }

        // apply reverseScrollPolarity for all non-touchpad pointer, touchpad
        // are belong to kcm touchpad.
        XIForallPointerDevices(m_dpy, [this](XDeviceInfo *info) {
            int deviceid = info->id;
            if (info->type == m_touchpadAtom) {
                return;
            }
            evdevApplyReverseScroll(deviceid, m_settings->reverseScrollPolarity);
        });
    }

    XChangePointerControl(m_dpy, true, true, int(qRound(m_settings->accelRate * 10)), 10, m_settings->thresholdMove);

    XFlush(m_dpy);
}

bool X11EvdevBackend::evdevApplyReverseScroll(int deviceid, bool reverse)
{
    // Check atom availability first.
    if (m_evdevWheelEmulationAtom == None || m_evdevScrollDistanceAtom == None || m_evdevWheelEmulationAxesAtom == None) {
        return false;
    }
    Status status;
    Atom type_return;
    int format_return;
    unsigned long num_items_return;
    unsigned long bytes_after_return;

    unsigned char *_data = nullptr;
    // data returned is an 1 byte boolean
    status = XIGetProperty(m_dpy,
                           deviceid,
                           m_evdevWheelEmulationAtom,
                           0,
                           1,
                           False,
                           XA_INTEGER,
                           &type_return,
                           &format_return,
                           &num_items_return,
                           &bytes_after_return,
                           &_data);
    QScopedArrayPointer<unsigned char, ScopedXDeleter> data(_data);
    _data = nullptr;
    if (status != Success) {
        return false;
    }

    // pointer device without wheel emulation
    if (type_return != XA_INTEGER || data == NULL || *data == False) {
        status = XIGetProperty(m_dpy,
                               deviceid,
                               m_evdevScrollDistanceAtom,
                               0,
                               3,
                               False,
                               XA_INTEGER,
                               &type_return,
                               &format_return,
                               &num_items_return,
                               &bytes_after_return,
                               &_data);
        data.reset(_data);
        _data = nullptr;
        // negate scroll distance
        if (status == Success && type_return == XA_INTEGER && format_return == 32 && num_items_return == 3) {
            int32_t *vals = (int32_t *)data.data();
            for (unsigned long i = 0; i < num_items_return; ++i) {
                int32_t val = *(vals + i);
                *(vals + i) = (int32_t)(reverse ? -abs(val) : abs(val));
            }
            XIChangeProperty(m_dpy, deviceid, m_evdevScrollDistanceAtom, XA_INTEGER, 32, XIPropModeReplace, data.data(), 3);
        }
    } else { // wheel emulation used, reverse wheel axes
        status = XIGetProperty(m_dpy,
                               deviceid,
                               m_evdevWheelEmulationAxesAtom,
                               0,
                               4,
                               False,
                               XA_INTEGER,
                               &type_return,
                               &format_return,
                               &num_items_return,
                               &bytes_after_return,
                               &_data);
        data.reset(_data);
        _data = nullptr;
        if (status == Success && type_return == XA_INTEGER && format_return == 8 && num_items_return == 4) {
            // when scroll direction is not reversed,
            // up button id should be smaller than down button id,
            // up/left are odd elements, down/right are even elements
            for (int i = 0; i < 2; ++i) {
                unsigned char odd = data[i * 2];
                unsigned char even = data[i * 2 + 1];
                unsigned char max_elem = std::max(odd, even);
                unsigned char min_elem = std::min(odd, even);
                data[i * 2] = reverse ? max_elem : min_elem;
                data[i * 2 + 1] = reverse ? min_elem : max_elem;
            }
            XIChangeProperty(m_dpy, deviceid, m_evdevWheelEmulationAxesAtom, XA_INTEGER, 8, XIPropModeReplace, data.data(), 4);
        }
    }

    return true;
}

void X11EvdevBackend::kcmInit()
{
    load();
    apply(true); // force
    X11Backend::kcmInit();
}
