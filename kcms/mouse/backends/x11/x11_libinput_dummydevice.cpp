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

        unsigned char sendVal[2] = {0};
        if (num_items_return == 1) {
            sendVal[0] = val;
        } else {
            // Special case for acceleration profile.
            const Atom accel = XInternAtom(dpy, LIBINPUT_PROP_ACCEL_PROFILE_ENABLED, True);
            if (num_items_return != 2 || valAtom != accel) {
                return;
            }
            sendVal[val] = 1;
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
    : QObject(parent)
    , m_settings(new LibinputSettings())
    , m_dpy(dpy)
{
    m_leftHanded.atom = XInternAtom(dpy, LIBINPUT_PROP_LEFT_HANDED, True);
    m_middleEmulation.atom = XInternAtom(dpy, LIBINPUT_PROP_MIDDLE_EMULATION_ENABLED, True);
    m_naturalScroll.atom = XInternAtom(dpy, LIBINPUT_PROP_NATURAL_SCROLL, True);
    m_pointerAcceleration.atom = XInternAtom(dpy, LIBINPUT_PROP_ACCEL, True);
    m_pointerAccelerationProfileFlat.atom = XInternAtom(dpy, LIBINPUT_PROP_ACCEL_PROFILE_ENABLED, True);

    m_supportsDisableEvents.val = false;
    m_enabled.val = true;
    m_supportedButtons.val = Qt::LeftButton | Qt::MiddleButton | Qt::RightButton;
    m_supportsLeftHanded.val = true;
    m_supportsMiddleEmulation.val = true;
    m_middleEmulationEnabledByDefault.val = false;

    m_supportsPointerAcceleration.val = true;
    m_defaultPointerAcceleration.val = 0;

    m_supportsPointerAccelerationProfileAdaptive.val = true;
    m_supportsPointerAccelerationProfileFlat.val = true;

    m_defaultPointerAccelerationProfileAdaptive.val = true;
    m_defaultPointerAccelerationProfileFlat.val = false;

    m_supportsNaturalScroll.val = true;
    m_naturalScrollEnabledByDefault.val = false;

    s_touchpadAtom = XInternAtom(m_dpy, XI_TOUCHPAD, True);
}

X11LibinputDummyDevice::~X11LibinputDummyDevice()
{
    delete m_settings;
}

bool X11LibinputDummyDevice::getConfig()
{
    auto reset = [this](Prop<bool> &prop, bool defVal) {
        prop.reset(m_settings->load(prop.cfgName, defVal));
    };

    reset(m_leftHanded, false);

    reset(m_middleEmulation, false);
    reset(m_naturalScroll, false);
    reset(m_pointerAccelerationProfileFlat, false);

    m_pointerAccelerationProfileAdaptive.reset(!m_settings->load(m_pointerAccelerationProfileFlat.cfgName, false));
    m_pointerAcceleration.reset(m_settings->load(m_pointerAcceleration.cfgName, 0.));

    return true;
}

bool X11LibinputDummyDevice::getDefaultConfig()
{
    m_leftHanded.set(false);

    m_pointerAcceleration.set(m_defaultPointerAcceleration);
    m_pointerAccelerationProfileFlat.set(m_defaultPointerAccelerationProfileFlat);
    m_pointerAccelerationProfileAdaptive.set(m_defaultPointerAccelerationProfileAdaptive);

    m_middleEmulation.set(m_middleEmulationEnabledByDefault);
    m_naturalScroll.set(m_naturalScrollEnabledByDefault);

    return true;
}

bool X11LibinputDummyDevice::applyConfig()
{
    valueWriter(m_leftHanded);
    valueWriter(m_middleEmulation);
    valueWriter(m_naturalScroll);
    valueWriter(m_pointerAcceleration);
    valueWriter(m_pointerAccelerationProfileFlat);

    return true;
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

bool X11LibinputDummyDevice::isChangedConfig() const
{
    return m_leftHanded.changed() || m_pointerAcceleration.changed() || m_pointerAccelerationProfileFlat.changed()
        || m_pointerAccelerationProfileAdaptive.changed() || m_middleEmulation.changed() || m_naturalScroll.changed();
}
