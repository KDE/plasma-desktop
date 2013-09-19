/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
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

#include <cstring>
#include <cmath>

#include <QtAlgorithms>
#include <QScopedPointer>
#include <QX11Info>

#include <KLocalizedString>

#include "touchpadparameters.h"

//Includes are ordered this way because of #defines in Xorg's headers
#include "xlibbackend.h" // krazy:exclude=includes

#include <X11/Xlib-xcb.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include <synaptics-properties.h>
#include <xserver-properties.h>

#include "synclientproperties.h"

static void XDeleter(void *p)
{
    if (p) {
        XFree(p);
    }
}

static void XIDeviceInfoDeleter(XIDeviceInfo *p)
{
    if (p) {
        XIFreeDeviceInfo(p);
    }
}

struct PropertyInfo
{
    Atom type;
    int format;
    QSharedPointer<unsigned char> data;
    unsigned long nitems;

    float *f;
    int *i;
    char *b;

    Display *display;
    int device;
    Atom prop;

    PropertyInfo() :
        type(0), format(0), nitems(0), f(0), i(0), b(0)
    {
    }

    PropertyInfo(Display *display, int device, Atom prop, Atom floatType)
        : type(0), format(0), nitems(0), f(0), i(0), b(0),
          display(display), device(device), prop(prop)
    {
        unsigned char *dataPtr = 0;
        unsigned long bytes_after;
        XIGetProperty(display, device, prop, 0, 1000, False,
                      AnyPropertyType, &type, &format, &nitems,
                      &bytes_after, &dataPtr);
        data = QSharedPointer<unsigned char>(dataPtr, XDeleter);

        if (format == CHAR_BIT && type == XA_INTEGER) {
            b = reinterpret_cast<char *>(dataPtr);
        }
        if (format == sizeof(int) * CHAR_BIT &&
                (type == XA_INTEGER || type == XA_CARDINAL))
        {
            i = reinterpret_cast<int *>(dataPtr);
        }
        if (format == sizeof(float) * CHAR_BIT &&
                floatType && type == floatType)
        {
            f = reinterpret_cast<float *>(dataPtr);
        }
    }

    QVariant value(unsigned offset) const
    {
        QVariant v;
        if (offset >= nitems) {
            return v;
        }

        if (b) {
            v = QVariant(static_cast<int>(b[offset]));
        }
        if (i) {
            v = QVariant(i[offset]);
        }
        if (f) {
            v = QVariant(f[offset]);
        }

        return v;
    }

    void set()
    {
        XIChangeProperty(display, device, prop, type, format,
                         XIPropModeReplace, data.data(), nitems);
    }
};

XlibBackend::~XlibBackend()
{
}

XlibBackend::XlibBackend(QObject *parent) :
    TouchpadBackend(parent),
    m_display(QX11Info::display()), m_connection(0), m_resX(1), m_resY(1)
{
    if (m_display) {
        m_connection = XGetXCBConnection(m_display);
    }

    if (!m_connection) {
        m_errorString = i18n("Can not connect to X server");
        return;
    }

    m_floatType.intern(m_connection, "FLOAT");
    m_capsAtom.intern(m_connection, SYNAPTICS_PROP_CAPABILITIES);
    m_enabledAtom.intern(m_connection, XI_PROP_ENABLED);
    m_touchpadOffAtom.intern(m_connection, SYNAPTICS_PROP_OFF);
    m_mouseAtom.intern(m_connection, XI_MOUSE);
    XcbAtom resolutionAtom(m_connection, SYNAPTICS_PROP_RESOLUTION);

    for (const Parameter *param = synapticsProperties; param->name; param++) {
        QLatin1String name(param->prop_name);

        if (!m_atoms.contains(name)) {
            m_atoms.insert(name, QSharedPointer<XcbAtom>(
                               new XcbAtom(m_connection, param->prop_name)));
        }
    }

    if (!m_capsAtom.atom()) {
        m_errorString =
                i18n("Synaptics driver is not installed (or is not used)");
        return;
    }

    m_device = findTouchpad();
    if (m_device == XIAllDevices) {
        m_errorString = i18n("No touchpads found");
        return;
    }

    for (const Parameter *p = synapticsProperties; p->name; p++) {
        if (getParameter(p).isValid()) {
            m_supported.append(p->name);
        }
    }

    if (m_supported.isEmpty()) {
        m_errorString = i18n("Can not read any of touchpad's properties");
        return;
    }

    m_toRadians.append("CircScrollDelta");

    PropertyInfo resolution(m_display, m_device, resolutionAtom, 0);
    if (!resolution.i || !resolution.nitems ||
            (resolution.nitems == 2 &&
             resolution.i[0] == 1 && resolution.i[1] == 1))
    {
        m_errorString = i18n("Can not read touchpad's resolution");
        //Non-fatal
    } else {
        m_resY = qMin(static_cast<unsigned long>(resolution.i[0]),
                static_cast<unsigned long>(INT_MAX));
        m_resX = qMin(static_cast<unsigned long>(resolution.i[1]),
                static_cast<unsigned long>(INT_MAX));

        m_scaleByResX.append("HorizScrollDelta");
        m_scaleByResY.append("VertScrollDelta");

        m_scaleByResX.append("MaxTapMove");
        m_scaleByResY.append("MaxTapMove");
    }
    m_resX = qMax(10, m_resX);
    m_resY = qMax(10, m_resY);
    m_negate["HorizScrollDelta"] = "InvertHorizScroll";
    m_negate["VertScrollDelta"] = "InvertVertScroll";
    m_supported.append(m_negate.values());
    m_supported.append("Coasting");

    PropertyInfo caps(m_display, m_device, m_capsAtom.atom(), 0);
    if (!caps.b) {
        m_errorString = i18n("Can not read touchpad's capabilities");
        return;
    }

    enum TouchpadCapabilitiy
    {
        TouchpadHasLeftButton,
        TouchpadHasMiddleButton,
        TouchpadHasRightButton,
        TouchpadTwoFingerDetect,
        TouchpadThreeFingerDetect,
        TouchpadPressureDetect,
        TouchpadPalmDetect,
        TouchpadCapsCount
    };

    QVector<bool> cap(TouchpadCapsCount, false);
    qCopy(caps.b, caps.b + qMin(cap.size(), static_cast<int>(caps.nitems)),
          cap.begin());

    if (!cap[TouchpadTwoFingerDetect]) {
        m_supported.removeAll("HorizTwoFingerScroll");
        m_supported.removeAll("VertTwoFingerScroll");
        m_supported.removeAll("TwoFingerTapButton");
    }

    if (!cap[TouchpadThreeFingerDetect]) {
        m_supported.removeAll("ThreeFingerTapButton");
    }

    if (!cap[TouchpadPressureDetect]) {
        m_supported.removeAll("FingerHigh");
        m_supported.removeAll("FingerLow");

        m_supported.removeAll("PalmMinZ");
        m_supported.removeAll("PressureMotionMinZ");
        m_supported.removeAll("PressureMotionMinFactor");
        m_supported.removeAll("PressureMotionMaxZ");
        m_supported.removeAll("PressureMotionMaxFactor");
        m_supported.removeAll("EmulateTwoFingerMinZ");
    }

    if (!cap[TouchpadPalmDetect]) {
        m_supported.removeAll("PalmDetect");
        m_supported.removeAll("PalmMinWidth");
        m_supported.removeAll("PalmMinZ");
        m_supported.removeAll("EmulateTwoFingerMinW");
    }

    for (QMap<QString, QString>::Iterator i = m_negate.begin();
         i != m_negate.end(); ++i)
    {
        if (!m_supported.contains(i.key())) {
            m_supported.removeAll(i.value());
        }
    }
}

int XlibBackend::findTouchpad()
{
    int nDevices = 0;
    QSharedPointer<XIDeviceInfo> deviceInfo(
                XIQueryDevice(m_display, XIAllDevices, &nDevices),
                XIDeviceInfoDeleter);

    for (XIDeviceInfo *info = deviceInfo.data();
         info < deviceInfo.data() + nDevices; info++)
    {
        int nProperties = 0;
        QSharedPointer<Atom> properties(
                    XIListProperties(m_display, info->deviceid, &nProperties),
                    XDeleter);

        if (std::count(properties.data(), properties.data() + nProperties,
                       m_capsAtom.atom()))
        {
            return info->deviceid;
        }
    }

    return XIAllDevices;
}

static const Parameter *findParameter(const QString &name)
{
    for (const Parameter *par = synapticsProperties; par->name; par++) {
        if (name == par->name) {
            return par;
        }
    }
    return 0;
}

double XlibBackend::getPropertyScale(const QString &name) const
{
    if (m_scaleByResX.contains(name) && m_scaleByResY.contains(name)) {
        return std::sqrt(static_cast<double>(m_resX) * m_resX
                         + static_cast<double>(m_resY) * m_resY);
    } else if (m_scaleByResX.contains(name)) {
        return m_resX;
    } else if (m_scaleByResY.contains(name)) {
        return m_resY;
    } else if (m_toRadians.contains(name)) {
        return M_PI_4 / 45.0;
    }
    return 1.0;
}

static QVariant negateVariant(const QVariant &value)
{
    if (value.type() == QVariant::Double) {
        return QVariant(-value.toDouble());
    } else if (value.type() == QVariant::Int) {
        return QVariant(-value.toInt());
    }
    return value;
}

bool XlibBackend::applyConfig(const TouchpadParameters *p)
{
    if (m_supported.isEmpty()) {
        return false;
    }

    m_props.clear();

    bool error = false;
    Q_FOREACH(const QString &name, m_supported) {
        KConfigSkeletonItem *i = p->findItem(name);
        if (!i) {
            continue;
        }
        const Parameter *par = findParameter(name);
        if (par) {
            QVariant value(i->property());

            double k = getPropertyScale(name);
            if (k != 1.0) {
                bool ok = false;
                value = QVariant(value.toDouble(&ok) * k);
                if (!ok) {
                    error = true;
                    continue;
                }
            }

            if (m_negate.contains(name)) {
                KConfigSkeletonItem *i = p->findItem(m_negate[name]);
                if (i && i->property().toBool()) {
                    value = negateVariant(value);
                }
            }

            if (name == "CoastingSpeed") {
                KConfigSkeletonItem *coastingEnabled = p->findItem("Coasting");
                if (coastingEnabled && !coastingEnabled->property().toBool()) {
                    value = QVariant(0);
                }
            }

            if (!setParameter(par, value)) {
                error = true;
            }
        }
    }

    flush();

    if (error) {
        m_errorString = i18n("Can not set touchpad configuration");
    }
    return !error;
}

void XlibBackend::flush()
{
    Q_FOREACH(const QLatin1String &name, m_changed) {
        m_props[name].set();
    }
    m_changed.clear();

    XFlush(m_display);
}

bool XlibBackend::getConfig(TouchpadParameters *p)
{
    if (m_supported.isEmpty()) {
        return false;
    }

    m_props.clear();

    bool error = false;
    Q_FOREACH(const QString &name, m_supported) {
        const Parameter *par = findParameter(name);
        if (!par) {
            continue;
        }

        QVariant value(getParameter(par));
        if (!value.isValid()) {
            error = true;
            continue;
        }

        double k = getPropertyScale(name);
        if (k != 1.0) {
            bool ok = false;
            value = QVariant(value.toDouble(&ok) / k);
            if (!ok) {
                error = true;
                continue;
            }
        }

        if (m_negate.contains(name)) {
            KConfigSkeletonItem *i = p->findItem(m_negate[name]);
            if (i) {
                bool negative = value.toDouble() < 0.0;
                i->setProperty(negative);
                if (negative) {
                    value = negateVariant(value);
                }
            }
        }

        if (name == "CoastingSpeed") {
            KConfigSkeletonItem *coastingEnabled = p->findItem("Coasting");
            if (coastingEnabled) {
                coastingEnabled->setProperty(QVariant(value.toDouble() != 0.0));
                if (!coastingEnabled->property().toBool()) {
                    continue;
                }
            }
        }

        KConfigSkeletonItem *i = p->findItem(name);
        if (i) {
            i->setProperty(value);
        }
    }

    if (error) {
        m_errorString = i18n("Can not read touchpad configuration");
    }
    return !error;
}

QVariant XlibBackend::getParameter(const Parameter *par)
{
    PropertyInfo *p = getDevProperty(QLatin1String(par->prop_name));
    if (!p || par->prop_offset >= p->nitems) {
        return QVariant();
    }

    return p->value(par->prop_offset);
}

PropertyInfo *XlibBackend::getDevProperty(const QLatin1String &propName)
{
    if (m_props.contains(propName)) {
        return &m_props[propName];
    }

    if (!m_atoms.contains(propName) || !m_atoms[propName]) {
        return 0;
    }

    xcb_atom_t prop = m_atoms[propName]->atom();
    if (!prop) {
        return 0;
    }

    PropertyInfo p(m_display, m_device, prop, m_floatType.atom());
    if (!p.b && !p.f && !p.i) {
        return 0;
    }
    return &m_props.insert(propName, p).value();
}

bool XlibBackend::setParameter(const Parameter *par, const QVariant &value)
{
    QLatin1String propName(par->prop_name);
    PropertyInfo *p = getDevProperty(propName);
    if (!p || par->prop_offset >= p->nitems) {
        return false;
    }

    QVariant converted(value);
    QVariant::Type convType = QVariant::Int;
    if (p->f) {
        convType = QVariant::Double;
    } else if (value.type() == QVariant::Double) {
        converted = QVariant(qRound(static_cast<qreal>(value.toDouble())));
    }

    if (!converted.convert(convType)) {
        return false;
    }

    if (converted == p->value(par->prop_offset)) {
        return true;
    }

    if (p->b) {
        p->b[par->prop_offset] = static_cast<char>(converted.toInt());
    } else if (p->i) {
        p->i[par->prop_offset] = converted.toInt();
    } else if (p->f) {
        p->f[par->prop_offset] = converted.toDouble();
    }

    m_changed.insert(propName);
    return true;
}

void XlibBackend::setTouchpadState(TouchpadBackend::TouchpadState state)
{
    PropertyInfo enabled(m_display, m_device, m_enabledAtom.atom(), 0);
    if (enabled.b && *(enabled.b) != (state != TouchpadFullyDisabled)) {
        *(enabled.b) = (state != TouchpadFullyDisabled);
        enabled.set();
    }

    PropertyInfo off(m_display, m_device, m_touchpadOffAtom.atom(), 0);
    if (off.b && *(off.b) != state) {
        *(off.b) = static_cast<int>(state);
        off.set();
    }

    flush();
}

TouchpadBackend::TouchpadState XlibBackend::getTouchpadState()
{
    PropertyInfo enabled(m_display, m_device, m_enabledAtom.atom(), 0);
    if (enabled.value(0) == false) {
        return TouchpadFullyDisabled;
    }

    PropertyInfo off(m_display, m_device, m_touchpadOffAtom.atom(), 0);
    return static_cast<TouchpadBackend::TouchpadState>(off.value(0).toInt());
}

void XlibBackend::deviceChanged(int device)
{
    if (device != m_device) {
        Q_EMIT mousesChanged();
    }
}

void XlibBackend::propertyChanged(Atom prop)
{
    if (prop == m_touchpadOffAtom.atom()) {
        Q_EMIT touchpadStateChanged();
    }
}

struct DeviceListDeleter
{
    static void cleanup(XDeviceInfo *p)
    {
        if (p) {
            XFreeDeviceList(p);
        }
    }
};

bool XlibBackend::isMousePluggedIn()
{
    int nDevices = 0;
    QScopedPointer<XDeviceInfo, DeviceListDeleter>
            info(XListInputDevices(m_display, &nDevices));
    for (XDeviceInfo *i = info.data(); i != info.data() + nDevices; i++) {
        if (i->id != static_cast<XID>(m_device) &&
                i->type == m_mouseAtom.atom())
        {
            return true;
        }
    }

    return false;
}

void XlibBackend::watchForEvents(bool keyboard)
{
    if (!m_notifications) {
        m_notifications.reset(
                    new XlibNotifications(m_display, m_connection, m_device));
        connect(m_notifications.data(), SIGNAL(deviceChanged(int)),
                SLOT(deviceChanged(int)));
        connect(m_notifications.data(), SIGNAL(propertyChanged(Atom)),
                SLOT(propertyChanged(Atom)));
    }

    if (keyboard == !m_keyboard.isNull()) {
        return;
    }

    if (!keyboard) {
        m_keyboard.reset();
        return;
    }

    m_keyboard.reset(new XRecordKeyboardMonitor());
    connect(m_keyboard.data(), SIGNAL(keyboardActivityStarted()),
            SIGNAL(keyboardActivityStarted()));
    connect(m_keyboard.data(), SIGNAL(keyboardActivityFinished()),
            SIGNAL(keyboardActivityFinished()));
}

#include "moc_xlibbackend.cpp"
