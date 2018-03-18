/*
 * Copyright 2017 Xuetian Weng <wengxt@gmail.com>
 * Copyright 2018 Roman Gilg <subdiff@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "x11_backend.h"

#include <config-X11.h>

#include <QFile>
#include <QX11Info>
#include <KLocalizedString>
#include <evdev-properties.h>
#include <libinput-properties.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI.h>
#include <X11/Xutil.h>
#ifdef HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/XInput.h>
#endif

static const char PROFILE_NONE[] = I18N_NOOP("None");
static const char PROFILE_ADAPTIVE[] = I18N_NOOP("Adaptive");
static const char PROFILE_FLAT[] = I18N_NOOP("Flat");

struct ScopedXDeleter {
    static inline void cleanup(void* pointer)
    {
        if (pointer) {
            XFree(pointer);
        }
    }
};

template<typename Callback>
static void XI2ForallPointerDevices(Display* dpy, const Callback& callback)
{
    int ndevices_return;
    XIDeviceInfo* info = XIQueryDevice(dpy, XIAllDevices, &ndevices_return);
    if (!info) {
        return;
    }
    for (int i = 0; i < ndevices_return; ++i) {
        if ((info + i)->use == XISlavePointer) {
            callback(info + i);
        }
    }
    XIFreeDeviceInfo(info);
}

template<typename Callback>
static void XIForallPointerDevices(Display* dpy, const Callback& callback)
{
    int ndevices_return;
    XDeviceInfo* info = XListInputDevices(dpy, &ndevices_return);
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

X11Backend::X11Backend(QObject* parent)
    : InputBackend(parent)
{
    m_mode = InputBackendMode::XEvdev;

    m_platformX11 = QX11Info::isPlatformX11();
    if (m_platformX11) {
        m_dpy = QX11Info::display();
    } else {
        // TODO: remove this - not needed anymore with Wayland backend!
        // let's hope we have a compatibility system like Xwayland ready
        m_dpy = XOpenDisplay(nullptr);
    }
    m_settings = new EvdevSettings();
    initAtom();
}

void X11Backend::initAtom()
{
    if (!m_dpy) {
        return;
    }

    m_libinputAccelProfileAvailableAtom = XInternAtom(m_dpy, LIBINPUT_PROP_ACCEL_PROFILES_AVAILABLE, True);
    m_libinputAccelProfileEnabledAtom = XInternAtom(m_dpy, LIBINPUT_PROP_ACCEL_PROFILE_ENABLED, True);
    m_libinputNaturalScrollAtom = XInternAtom(m_dpy, LIBINPUT_PROP_NATURAL_SCROLL, True);

    m_evdevScrollDistanceAtom = XInternAtom(m_dpy, EVDEV_PROP_SCROLL_DISTANCE, True);
    m_evdevWheelEmulationAtom = XInternAtom(m_dpy, EVDEV_PROP_WHEEL, True);
    m_evdevWheelEmulationAxesAtom = XInternAtom(m_dpy, EVDEV_PROP_WHEEL_AXES, True);

    m_touchpadAtom = XInternAtom(m_dpy, XI_TOUCHPAD, True);
}


X11Backend::~X11Backend()
{
    if (!m_platformX11 && m_dpy) {
        XCloseDisplay(m_dpy);
    }
    delete m_settings;
}

bool X11Backend::supportScrollPolarity()
{
    return m_numButtons >= 5;
}

QStringList X11Backend::supportedAccelerationProfiles()
{
    return m_supportedAccelerationProfiles;
}

QString X11Backend::accelerationProfile()
{
    return m_accelerationProfile;
}

double X11Backend::accelRate()
{
    return m_accelRate;
}

Handed X11Backend::handed()
{
    return m_handed;
}

int X11Backend::threshold()
{
    return m_threshold;
}

void X11Backend::load()
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

    m_supportedAccelerationProfiles.clear();
    bool adaptiveAvailable = false;
    bool flatAvailable = false;
    bool adaptiveEnabled = false;
    bool flatEnabled = false;
    if (m_libinputAccelProfileAvailableAtom != None && m_libinputAccelProfileEnabledAtom != None) {
        XI2ForallPointerDevices(m_dpy, [&] (XIDeviceInfo *info) {
            int deviceid = info->deviceid;
            Status status;
            Atom type_return;
            int format_return;
            unsigned long num_items_return;
            unsigned long bytes_after_return;

            unsigned char *_data = nullptr;
            //data returned is an 2 byte boolean
            status = XIGetProperty(m_dpy, deviceid, m_libinputAccelProfileAvailableAtom, 0, 2,
                                    False, XA_INTEGER, &type_return, &format_return,
                                    &num_items_return, &bytes_after_return, &_data);
            QScopedArrayPointer<unsigned char, ScopedXDeleter> data(_data);
            _data = nullptr;
            if (status != Success || type_return != XA_INTEGER || !data || format_return != 8 || num_items_return != 2) {
                return;
            }
            adaptiveAvailable = adaptiveAvailable || data[0];
            flatAvailable = flatAvailable || data[1];

            //data returned is an 2 byte boolean
            status = XIGetProperty(m_dpy, deviceid, m_libinputAccelProfileEnabledAtom, 0, 2,
                                    False, XA_INTEGER, &type_return, &format_return,
                                    &num_items_return, &bytes_after_return, &_data);
            data.reset(_data);
            _data = nullptr;
            if (status != Success || type_return != XA_INTEGER || !data || format_return != 8 || num_items_return != 2) {
                return;
            }
            adaptiveEnabled = adaptiveEnabled || data[0];
            flatEnabled = flatEnabled || data[1];
        });
    }

    if (adaptiveAvailable) {
        m_supportedAccelerationProfiles << PROFILE_ADAPTIVE;
    }
    if (flatAvailable) {
        m_supportedAccelerationProfiles << PROFILE_FLAT;
    }
    if (adaptiveAvailable || flatAvailable) {
        m_supportedAccelerationProfiles << PROFILE_NONE;
    }

    m_accelerationProfile = PROFILE_NONE;
    if (adaptiveEnabled) {
        m_accelerationProfile = PROFILE_ADAPTIVE;
    } else if (flatEnabled) {
        m_accelerationProfile = PROFILE_FLAT;
    }

    m_settings->load(this);
}

void X11Backend::apply(bool force)
{
    // 256 might seems extreme, but X has already been known to return 32,
    // and we don't want to truncate things. Xlib limits the table to 256 bytes,
    // so it's a good upper bound..
    unsigned char map[256];
    XGetPointerMapping(m_dpy, map, 256);

    if (m_settings->handedEnabled && (m_settings->handedNeedsApply || force)) {
        if (m_numButtons == 1) {
            map[0] = (unsigned char) 1;
        } else if (m_numButtons == 2) {
            if (m_settings->handed == Handed::Right) {
                map[0] = (unsigned char) 1;
                map[1] = (unsigned char) 3;
            } else {
                map[0] = (unsigned char) 3;
                map[1] = (unsigned char) 1;
            }
        } else { // 3 buttons and more
            if (m_settings->handed == Handed::Right) {
                map[0] = (unsigned char) 1;
                map[1] = (unsigned char) m_middleButton;
                map[2] = (unsigned char) 3;
            } else {
                map[0] = (unsigned char) 3;
                map[1] = (unsigned char) m_middleButton;
                map[2] = (unsigned char) 1;
            }
        }

        int retval;
        if (m_numButtons >= 1) {
            while ((retval = XSetPointerMapping(m_dpy, map,
                                                m_numButtons)) == MappingBusy)
                /* keep trying until the pointer is free */
            { };
        }

        // apply reverseScrollPolarity for all non-touchpad pointer, touchpad
        // are belong to kcm touchpad.
        XIForallPointerDevices(m_dpy, [this](XDeviceInfo * info) {
            int deviceid = info->id;
            if (info->type == m_touchpadAtom) {
                return;
            }
            if (libinputApplyReverseScroll(deviceid, m_settings->reverseScrollPolarity)) {
                return;
            }
            evdevApplyReverseScroll(deviceid, m_settings->reverseScrollPolarity);
        });

    }

    XI2ForallPointerDevices(m_dpy, [&] (XIDeviceInfo *info) {
        libinputApplyAccelerationProfile(info->deviceid, m_settings->currentAccelProfile);
    });

    XChangePointerControl(m_dpy,
                          true, true, int(qRound(m_settings->accelRate * 10)), 10, m_settings->thresholdMove);

    XFlush(m_dpy);
}

QString X11Backend::currentCursorTheme()
{
    if (!m_dpy) {
        return QString();
    }

    QByteArray name = XGetDefault(m_dpy, "Xcursor", "theme");
#ifdef HAVE_XCURSOR
    if (name.isEmpty()) {
        name = QByteArray(XcursorGetTheme(m_dpy));
    }
#endif
    return QFile::decodeName(name);
}

void X11Backend::applyCursorTheme(const QString& theme, int size)
{
#ifdef HAVE_XCURSOR

    // Apply the KDE cursor theme to ourselves
    if (m_dpy) {
        return;
    }
    if (!theme.isEmpty()) {
        XcursorSetTheme(m_dpy, QFile::encodeName(theme));
    }

    if (size >= 0) {
        XcursorSetDefaultSize(m_dpy, size);
    }

    // Load the default cursor from the theme and apply it to the root window.
    Cursor handle = XcursorLibraryLoadCursor(m_dpy, "left_ptr");
    XDefineCursor(m_dpy, DefaultRootWindow(m_dpy), handle);
    XFreeCursor(m_dpy, handle); // Don't leak the cursor
    XFlush(m_dpy);
#endif
}

bool X11Backend::evdevApplyReverseScroll(int deviceid, bool reverse)
{
    // Check atom availability first.
    if (m_evdevWheelEmulationAtom == None || m_evdevScrollDistanceAtom == None ||
        m_evdevWheelEmulationAxesAtom == None) {
        return false;
    }
    Status status;
    Atom type_return;
    int format_return;
    unsigned long num_items_return;
    unsigned long bytes_after_return;

    unsigned char* _data = nullptr;
    //data returned is an 1 byte boolean
    status = XIGetProperty(m_dpy, deviceid, m_evdevWheelEmulationAtom, 0, 1,
                           False, XA_INTEGER, &type_return, &format_return,
                           &num_items_return, &bytes_after_return, &_data);
    QScopedArrayPointer<unsigned char, ScopedXDeleter> data(_data);
    _data = nullptr;
    if (status != Success) {
        return false;
    }

    // pointer device without wheel emulation
    if (type_return != XA_INTEGER || data == NULL || *data == False) {
        status = XIGetProperty(m_dpy, deviceid, m_evdevScrollDistanceAtom, 0, 3,
                               False, XA_INTEGER, &type_return, &format_return,
                               &num_items_return, &bytes_after_return, &_data);
        data.reset(_data);
        _data = nullptr;
        // negate scroll distance
        if (status == Success && type_return == XA_INTEGER &&
                format_return == 32 && num_items_return == 3) {
            int32_t* vals = (int32_t*)data.data();
            for (unsigned long i = 0; i < num_items_return; ++i) {
                int32_t val = *(vals + i);
                *(vals + i) = (int32_t)(reverse ? -abs(val) : abs(val));
            }
            XIChangeProperty(m_dpy, deviceid, m_evdevScrollDistanceAtom, XA_INTEGER,
                             32, XIPropModeReplace, data.data(), 3);
        }
    } else { // wheel emulation used, reverse wheel axes
        status = XIGetProperty(m_dpy, deviceid, m_evdevWheelEmulationAxesAtom, 0, 4,
                               False, XA_INTEGER, &type_return, &format_return,
                               &num_items_return, &bytes_after_return, &_data);
        data.reset(_data);
        _data = nullptr;
        if (status == Success && type_return == XA_INTEGER &&
                format_return == 8 && num_items_return == 4) {
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
            XIChangeProperty(m_dpy, deviceid, m_evdevWheelEmulationAxesAtom, XA_INTEGER,
                             8, XIPropModeReplace, data.data(), 4);
        }
    }

    return true;
}

bool X11Backend::libinputApplyReverseScroll(int deviceid, bool reverse)
{
    // Check atom availability first.
    if (m_libinputNaturalScrollAtom == None) {
        return false;
    }
    Status status;
    Atom type_return;
    int format_return;
    unsigned long num_items_return;
    unsigned long bytes_after_return;

    unsigned char *_data = nullptr;
    //data returned is an 1 byte boolean
    status = XIGetProperty(m_dpy, deviceid, m_libinputNaturalScrollAtom, 0, 1,
                            False, XA_INTEGER, &type_return, &format_return,
                            &num_items_return, &bytes_after_return, &_data);
    if (status != Success) {
        return false;
    }
    QScopedArrayPointer<unsigned char, ScopedXDeleter> data(_data);
    _data = nullptr;
    if (type_return != XA_INTEGER || !data || format_return != 8 || num_items_return != 1) {
        return false;
    }
    unsigned char natural = reverse ? 1 : 0;
    XIChangeProperty(m_dpy, deviceid, m_libinputNaturalScrollAtom, XA_INTEGER,
                        8, XIPropModeReplace, &natural, 1);
    return true;
}

void X11Backend::libinputApplyAccelerationProfile(int deviceid, QString profile)
{
    // Check atom availability first.
    if (m_libinputAccelProfileAvailableAtom == None || m_libinputAccelProfileEnabledAtom == None) {
        return;
    }

    unsigned char profileData[2];
    if (profile == PROFILE_NONE) {
        profileData[0] = profileData[1] = 0;
    } else if (profile == PROFILE_ADAPTIVE) {
        profileData[0] = 1;
        profileData[1] = 0;
    } else if (profile == PROFILE_FLAT) {
        profileData[0] = 0;
        profileData[1] = 1;
    }
    Status status;
    Atom type_return;
    int format_return;
    unsigned long num_items_return;
    unsigned long bytes_after_return;

    unsigned char *_data = nullptr;
    //data returned is an 1 byte boolean
    status = XIGetProperty(m_dpy, deviceid, m_libinputAccelProfileAvailableAtom, 0, 2,
                            False, XA_INTEGER, &type_return, &format_return,
                            &num_items_return, &bytes_after_return, &_data);
    if (status != Success) {
        return;
    }
    QScopedArrayPointer<unsigned char, ScopedXDeleter> data(_data);
    _data = nullptr;
    if (type_return != XA_INTEGER || !data || format_return != 8 || num_items_return != 2) {
        return;
    }
    // Check availability for profile.
    if (profileData[0] > data[0] || profileData[1] > data[1]) {
        return;
    }
    XIChangeProperty(m_dpy, deviceid, m_libinputAccelProfileEnabledAtom, XA_INTEGER,
                        8, XIPropModeReplace, profileData, 2);
}
