#include <cstring>
#include <cmath>

#include <QtAlgorithms>
#include <QScopedPointer>

#include "touchpadparameters.h"
#include "xlibbackend.h"

#include <X11/Xlib-xcb.h>
#include <X11/Xatom.h>
#include <xcb/xcb_atom.h>

#include <xorg/synaptics-properties.h>

#include "synclientproperties.h"

static void XDeleter(void *p)
{
    if (p) {
        XFree(p);
    }
}

static void XDisplayDeleter(Display *p)
{
    if (p) {
        XCloseDisplay(p);
    }
}

struct XDeviceDeleter
{
    XDeviceDeleter(Display *d) : m_display(d)
    {
    }

    void operator() (XDevice *d) const
    {
        if (d) {
            XCloseDevice(m_display, d);
        }
    }

private:
    Display *m_display;
};

struct PropertyInfo
{
    Atom type;
    int format;
    QSharedPointer<unsigned char> data;
    unsigned long nitems;

    union flong {
        long l;
        float f;
    } *f;
    long *i;
    char *b;

    PropertyInfo() :
        type(0), format(0), nitems(0), f(0), i(0), b(0)
    {
    }

    PropertyInfo(Display *display, XDevice *device, Atom prop, Atom floatType)
        : type(0), format(0), nitems(0), f(0), i(0), b(0)
    {
        unsigned char *dataPtr = 0;
        unsigned long bytes_after;
        XGetDeviceProperty(display, device, prop, 0, 1000, False,
                           AnyPropertyType, &type, &format, &nitems,
                           &bytes_after, &dataPtr);
        data = QSharedPointer<unsigned char>(dataPtr, XDeleter);

        if (format == 8 && type == XA_INTEGER) {
            b = reinterpret_cast<char *>(dataPtr);
        }
        if (format == 32 && (type == XA_INTEGER || type == XA_CARDINAL)) {
            i = reinterpret_cast<long *>(dataPtr);
        }
        if (floatType && format == 32 && type == floatType) {
            f = reinterpret_cast<flong *>(dataPtr);
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
            v = QVariant(static_cast<int>(i[offset]));
        }
        if (f) {
            v = QVariant(f[offset].f);
        }

        return v;
    }
};

XlibBackend::~XlibBackend()
{
}

XlibBackend::XlibBackend(QObject *parent) :
    TouchpadBackend(parent),
    m_display(XOpenDisplay(0), XDisplayDeleter), m_connection(0),
    m_resX(1), m_resY(1)
{
    if (m_display) {
        m_connection = XGetXCBConnection(m_display.data());
    }

    if (!m_connection) {
        m_errorString = "Can not connect to X server";
        return;
    }

    m_devicesCookie = xcb_input_list_input_devices(m_connection);
    m_touchpadType.intern(m_connection, XI_TOUCHPAD);
    m_floatType.intern(m_connection, "FLOAT");
    m_capsAtom.intern(m_connection, SYNAPTICS_PROP_CAPABILITIES);
    XcbAtom resolutionAtom(m_connection, SYNAPTICS_PROP_RESOLUTION);

    for (const Parameter *param = synapticsProperties; param->name; param++) {
        QLatin1String name(param->prop_name);

        if (!m_atoms.contains(name)) {
            m_atoms.insert(name, QSharedPointer<XcbAtom>(
                               new XcbAtom(m_connection, param->prop_name)));
        }
    }

    if (!m_touchpadType.atom() || !m_capsAtom.atom()) {
        m_errorString = "Synaptics driver is not installed (or is not used)";
        return;
    }

    m_device = findTouchpad();
    if (!m_device) {
        m_errorString = "No touchpads found";
        return;
    }

    for (const Parameter *p = synapticsProperties; p->name; p++) {
        if (getParameter(p).isValid()) {
            m_supported.append(p->name);
        }
    }

    if (m_supported.isEmpty()) {
        m_errorString = "Can not read any of touchpad's properties";
        return;
    }

    PropertyInfo resolution(m_display.data(), m_device.data(),
                            resolutionAtom, 0);
    if (!resolution.i || !resolution.nitems ||
            (resolution.nitems == 2 &&
             resolution.i[0] == 1 && resolution.i[1] == 1))
    {
        m_errorString = "Can not read touchpad's resolution";
        //Non-fatal
    } else {
        m_resY = qMin(static_cast<unsigned long>(resolution.i[0]),
                static_cast<unsigned long>(INT_MAX));
        m_resX = qMin(static_cast<unsigned long>(resolution.i[1]),
                static_cast<unsigned long>(INT_MAX));
    }
    m_resX = qMax(10, m_resX);
    m_resY = qMax(10, m_resY);

    PropertyInfo caps(m_display.data(), m_device.data(), m_capsAtom.atom(), 0);
    if (!caps.b) {
        m_errorString = "Can not read touchpad's capabilities";
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
}

QSharedPointer<XDevice> XlibBackend::findTouchpad()
{
    QScopedPointer<xcb_input_list_input_devices_reply_t> devicesReply(
                xcb_input_list_input_devices_reply(m_connection,
                                                   m_devicesCookie, 0));
    if (!devicesReply) {
        return QSharedPointer<XDevice>();
    }

    xcb_input_device_info_t *deviceInfo =
            xcb_input_list_input_devices_devices(devicesReply.data());
    int nDevices =
            xcb_input_list_input_devices_devices_length(devicesReply.data());

    for (int i = 0; i < nDevices && !m_device; i++) {
        if (deviceInfo[i].device_type == m_touchpadType.atom()) {
            XDevice *devicePtr = XOpenDevice(m_display.data(),
                                             deviceInfo[i].device_id);
            if (!devicePtr) {
                continue;
            }
            QSharedPointer<XDevice> device(devicePtr,
                                           XDeviceDeleter(m_display.data()));

            int nProperties = 0;
            QSharedPointer<Atom> properties(
                        XListDeviceProperties(m_display.data(), device.data(),
                                              &nProperties), XDeleter);
            if (!properties) {
                continue;
            }

            if (std::count(properties.data(), properties.data() + nProperties,
                           m_capsAtom.atom()))
            {
                return device;
            }
        }
    }

    return QSharedPointer<XDevice>();
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

int XlibBackend::getPropertyScale(const QString &name) const
{
    if (m_scaleByResX.contains(name) && m_scaleByResY.contains(name)) {
        return qRound(std::sqrt(static_cast<qreal>(m_resX) * m_resX
                                + static_cast<qreal>(m_resY) * m_resY));
    } else if (m_scaleByResX.contains(name)) {
        return m_resX;
    } else if (m_scaleByResY.contains(name)) {
        return m_resY;
    }
    return 1;
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
        const Parameter *par = findParameter(name);
        if (i && par) {
            QVariant value(i->property());

            int k = getPropertyScale(name);
            if (k != 1) {
                bool ok = false;
                value = QVariant(qRound(value.toDouble(&ok) * k));
                if (!ok) {
                    error = true;
                    continue;
                }
            }

            if (!setParameter(par, value)) {
                error = true;
            }
        }
    }

    Q_FOREACH(const QLatin1String &name, m_changed) {
        PropertyInfo &info = m_props[name];
        XChangeDeviceProperty(m_display.data(), m_device.data(),
                              m_atoms[name]->atom(), info.type, info.format,
                              PropModeReplace, info.data.data(), info.nitems);
    }
    m_changed.clear();

    XFlush(m_display.data());

    if (error) {
        m_errorString = "Can not set touchpad configuration";
    }
    return !error;
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

        int k = getPropertyScale(name);
        if (k != 1) {
            bool ok = false;
            value = QVariant(value.toDouble(&ok) / k);
            if (!ok) {
                error = true;
                continue;
            }
        }

        KConfigSkeletonItem *i = p->findItem(name);
        if (i) {
            i->setProperty(value);
        }
    }

    if (error) {
        m_errorString = "Can not read touchpad configuration";
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

    PropertyInfo p(m_display.data(), m_device.data(), prop, m_floatType.atom());
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
        p->f[par->prop_offset].f = converted.toDouble();
    }

    m_changed.insert(propName);
    return true;
}
