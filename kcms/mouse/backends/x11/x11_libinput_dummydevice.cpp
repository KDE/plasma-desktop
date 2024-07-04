/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "x11_libinput_dummydevice.h"
#include "libinput_settings.h"

#include <libinput-properties.h>

#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

static Atom s_touchpadAtom;

template<typename Callback>
static void XIForallPointerDevices(Display *dpy, const Callback &callback)
{
    int ndevices_return;
    XDeviceInfo *info = XListInputDevices(dpy, &ndevices_return);
    if (!info) {
        return;
    }
    for (int i = 0; i < ndevices_return; ++i) {
        XDeviceInfo *dev = info + i;
        if ((dev->use == IsXPointer || dev->use == IsXExtensionPointer) && dev->type != s_touchpadAtom) {
            callback(dev);
        }
    }
    XFreeDeviceList(info);
}

struct ScopedXDeleter {
    static inline void cleanup(void *pointer)
    {
        if (pointer) {
            XFree(pointer);
        }
    }
};

namespace
{
template<typename T>
void valueWriterPart(T val, Atom valAtom, Display *dpy)
{
    Q_UNUSED(val);
    Q_UNUSED(valAtom);
    Q_UNUSED(dpy);
}

template<>
void valueWriterPart<bool>(bool val, Atom valAtom, Display *dpy)
{
    XIForallPointerDevices(dpy, [&](XDeviceInfo *info) {
        int deviceid = info->id;
        Status status;
        Atom type_return;
        int format_return;
        unsigned long num_items_return;
        unsigned long bytes_after_return;

        unsigned char *_data = nullptr;
        // data returned is an 1 byte boolean
        status = XIGetProperty(dpy, deviceid, valAtom, 0, 1, False, XA_INTEGER, &type_return, &format_return, &num_items_return, &bytes_after_return, &_data);
        if (status != Success) {
            return;
        }

        QScopedArrayPointer<unsigned char, ScopedXDeleter> data(_data);
        _data = nullptr;

        if (type_return != XA_INTEGER || !data || format_return != 8) {
            return;
        }

        unsigned char sendVal[3] = {0};
        if (num_items_return == 1) {
            sendVal[0] = val;
        } else {
            if (num_items_return < 2 || num_items_return > 3) {
                return;
            }

            if (valAtom == XInternAtom(dpy, LIBINPUT_PROP_ACCEL_PROFILE_ENABLED, True)) {
                // Special case for acceleration profile.
                sendVal[val] = 1;
            } else if (valAtom == XInternAtom(dpy, LIBINPUT_PROP_SCROLL_METHOD_ENABLED, True)) {
                // Special case for scroll on button down.
                sendVal[2] = val;
            } else {
                return;
            }
        }

        XIChangeProperty(dpy, deviceid, valAtom, XA_INTEGER, 8, XIPropModeReplace, sendVal, num_items_return);
    });
}

template<>
void valueWriterPart<qreal>(qreal val, Atom valAtom, Display *dpy)
{
    XIForallPointerDevices(dpy, [&](XDeviceInfo *info) {
        int deviceid = info->id;
        Status status;
        Atom float_type = XInternAtom(dpy, "FLOAT", False);
        Atom type_return;
        int format_return;
        unsigned long num_items_return;
        unsigned long bytes_after_return;

        unsigned char *_data = nullptr;
        // data returned is an 1 byte boolean
        status = XIGetProperty(dpy, deviceid, valAtom, 0, 1, False, float_type, &type_return, &format_return, &num_items_return, &bytes_after_return, &_data);
        if (status != Success) {
            return;
        }

        QScopedArrayPointer<unsigned char, ScopedXDeleter> data(_data);
        _data = nullptr;

        if (type_return != float_type || !data || format_return != 32 || num_items_return != 1) {
            return;
        }

        unsigned char buffer[4096];
        float *sendPtr = (float *)buffer;
        *sendPtr = val;

        XIChangeProperty(dpy, deviceid, valAtom, float_type, format_return, XIPropModeReplace, buffer, 1);
    });
}
}

X11LibinputDummyDevice::X11LibinputDummyDevice(QObject *parent, Display *dpy)
    : InputDevice(parent)
    , m_settings(new LibinputSettings())
    , m_dpy(dpy)
{
    m_leftHanded.atom = XInternAtom(dpy, LIBINPUT_PROP_LEFT_HANDED, True);
    m_middleEmulation.atom = XInternAtom(dpy, LIBINPUT_PROP_MIDDLE_EMULATION_ENABLED, True);
    m_naturalScroll.atom = XInternAtom(dpy, LIBINPUT_PROP_NATURAL_SCROLL, True);
    m_scrollOnButtonDown.atom = XInternAtom(dpy, LIBINPUT_PROP_SCROLL_METHOD_ENABLED, True);
    m_pointerAcceleration.atom = XInternAtom(dpy, LIBINPUT_PROP_ACCEL, True);
    m_pointerAccelerationProfileFlat.atom = XInternAtom(dpy, LIBINPUT_PROP_ACCEL_PROFILE_ENABLED, True);

    m_enabled.val = true;

    auto x11DefaultFlat = m_settings->load(QStringLiteral("X11LibInputXAccelProfileFlat"), false);
    m_defaultPointerAccelerationProfileFlat.val = x11DefaultFlat;
    m_defaultPointerAccelerationProfileAdaptive.val = !x11DefaultFlat;

    s_touchpadAtom = XInternAtom(m_dpy, XI_TOUCHPAD, True);
}

X11LibinputDummyDevice::~X11LibinputDummyDevice() = default;

bool X11LibinputDummyDevice::load()
{
    auto reset = [this](Prop<bool> &prop, bool defVal) {
        prop.reset(m_settings->load(prop.cfgName, defVal));
    };

    reset(m_leftHanded, false);

    reset(m_middleEmulation, false);
    reset(m_naturalScroll, false);
    reset(m_scrollOnButtonDown, s_scrollOnButtonDownEnabledByDefault);
    reset(m_pointerAccelerationProfileFlat, m_defaultPointerAccelerationProfileFlat.val);
    m_pointerAccelerationProfileAdaptive.reset(!m_pointerAccelerationProfileFlat.val);

    m_pointerAcceleration.reset(m_settings->load(m_pointerAcceleration.cfgName, 0.));

    return true;
}

bool X11LibinputDummyDevice::defaults()
{
    m_leftHanded.set(false);

    m_pointerAcceleration.set(s_defaultPointerAcceleration);
    m_pointerAccelerationProfileFlat.set(m_defaultPointerAccelerationProfileFlat);
    m_pointerAccelerationProfileAdaptive.set(m_defaultPointerAccelerationProfileAdaptive);

    m_middleEmulation.set(s_middleEmulationEnabledByDefault);
    m_naturalScroll.set(s_naturalScrollEnabledByDefault);
    m_scrollOnButtonDown.set(s_scrollOnButtonDownEnabledByDefault);

    return true;
}

bool X11LibinputDummyDevice::save()
{
    valueWriter(m_leftHanded);
    valueWriter(m_middleEmulation);
    valueWriter(m_naturalScroll);
    valueWriter(m_scrollOnButtonDown);
    valueWriter(m_pointerAcceleration);
    valueWriter(m_pointerAccelerationProfileFlat);

    return true;
}

void X11LibinputDummyDevice::defaultsFromX()
{
    // The user can override certain values in their X configuration. We want to
    // account for those in our default values, but if we just read this when
    // loading the KCM, we end up reading the current settings which may already
    // have been modified by us. So instead, read these defaults during startup
    // and write them to config, so we can later on read them again to know the
    // system-wide defaults.
    bool flatProfile = true;
    XIForallPointerDevices(m_dpy, [&](XDeviceInfo *info) {
        Atom property = m_pointerAccelerationProfileFlat.atom;
        Atom type_return;
        int format_return;
        unsigned long num_items_return;
        unsigned long bytes_after_return;
        unsigned char *_data = nullptr;

        auto status =
            XIGetProperty(m_dpy, info->id, property, 0, 1, False, XA_INTEGER, &type_return, &format_return, &num_items_return, &bytes_after_return, &_data);
        if (status != Success) {
            return;
        }

        QScopedArrayPointer<unsigned char, ScopedXDeleter> data(_data);
        _data = nullptr;

        if (type_return != XA_INTEGER || !data || format_return != 8 || num_items_return != 2) {
            return;
        }

        if (data[0] == 1 && data[1] == 0) {
            flatProfile = false;
        }
    });
    m_settings->save(QStringLiteral("X11LibInputXAccelProfileFlat"), flatProfile);
}

template<typename T>
bool X11LibinputDummyDevice::valueWriter(Prop<T> &prop)
{
    // Check atom availability first.
    if (prop.atom == None) {
        return false;
    }

    if (prop.val != prop.old) {
        m_settings->save(prop.cfgName, prop.val);
    }

    valueWriterPart(prop.val, prop.atom, m_dpy);

    prop.old = prop.val;

    return true;
}

bool X11LibinputDummyDevice::isSaveNeeded() const
{
    //     general & advanced
    return m_leftHanded.isSaveNeeded() //
        || m_middleEmulation.isSaveNeeded() //
        // acceleration
        || m_pointerAcceleration.isSaveNeeded() //
        || m_pointerAccelerationProfileFlat.isSaveNeeded() //
        // scrolling
        || m_pointerAccelerationProfileAdaptive.isSaveNeeded() //
        || m_naturalScroll.isSaveNeeded() //
        || m_scrollOnButtonDown.isSaveNeeded();
}

#include <fixx11h.h>

#include "moc_x11_libinput_dummydevice.cpp"
