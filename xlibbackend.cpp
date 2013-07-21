#include <cstring>

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
        Q_ASSERT(d);
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

    QVariant value(int offset) const
    {
        Q_ASSERT(offset < nitems);

        QVariant v;
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

XlibBackend::XlibBackend(QObject *parent) :
    TouchpadBackend(parent), m_triedInit(false),
    m_display(XOpenDisplay(0), XDisplayDeleter), m_connection(0)
{
    if (!m_display) {
        return;
    }

    m_connection = XGetXCBConnection(m_display.data());

    if (!m_connection) {
        return;
    }

    m_devicesCookie = xcb_input_list_input_devices(m_connection);
    m_touchpadType.intern(m_connection, XI_TOUCHPAD);
    m_floatType.intern(m_connection, "FLOAT");
    m_capsAtom.intern(m_connection, SYNAPTICS_PROP_CAPABILITIES);

    for (const Parameter *param = synapticsProperties; param->name; param++) {
        QLatin1String name(param->prop_name);

        if (!m_atoms.contains(name)) {
            m_atoms.insert(name, QSharedPointer<XcbAtom>(
                               new XcbAtom(m_connection, param->prop_name)));
        }
    }
}

QSharedPointer<XDevice> XlibBackend::findTouchpad()
{
    QScopedPointer<xcb_input_list_input_devices_reply_t> devicesReply(
                xcb_input_list_input_devices_reply(m_connection,
                                                   m_devicesCookie, 0));
    if (!devicesReply) {
        Q_EMIT error("Can't get input device list");
        return QSharedPointer<XDevice>();
    }

    if (!m_touchpadType.atom()) {
        Q_EMIT error("XI_TOUCHPAD doesn't exist");
        return QSharedPointer<XDevice>();
    }

    if (!m_capsAtom.atom()) {
        Q_EMIT error("SYNAPTICS_PROP_CAPABILITIES doesn't exist");
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

    Q_EMIT error("Can't find touchpad");
    return QSharedPointer<XDevice>();
}

bool XlibBackend::init()
{
    if (m_triedInit) {
        return !m_supported.isEmpty();
    }
    m_triedInit = true;

    if (!m_display) {
        Q_EMIT error("No connection to X server");
        return false;
    }

    if (!m_connection) {
        Q_EMIT error("Can't get XCB connection");
        return false;
    }

    m_device = findTouchpad();
    if (!m_device) {
        return false;
    }

    for (const Parameter *p = synapticsProperties; p->name; p++) {
        QVariant value;
        if (getParameter(p, value)) {
            m_supported.append(p->name);
        }
    }

    PropertyInfo caps(m_display.data(), m_device.data(), m_capsAtom.atom(), 0);
    if (!caps.b) {
        Q_EMIT error("Can't read touchpad's capabilities");
        return false;
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
        m_supported.removeAll("TapButton2");
    }

    if (!cap[TouchpadThreeFingerDetect]) {
        m_supported.removeAll("TapButton3");
    }

    return true;
}

XlibBackend::~XlibBackend()
{
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

void XlibBackend::applyConfig(const TouchpadParameters *p)
{
    if (!init()) {
        return;
    }

    m_props.clear();

    Q_FOREACH(const QString &name, m_supported) {
        KConfigSkeletonItem *i = p->findItem(name);
        const Parameter *par = findParameter(name);
        if (i && par) {
            setParameter(par, i->property());
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
}

void XlibBackend::getConfig(TouchpadParameters *p)
{
    if (!init()) {
        return;
    }

    m_props.clear();

    Q_FOREACH(const QString &name, m_supported) {
        const Parameter *par = findParameter(name);
        if (!par) {
            continue;
        }

        QVariant value;
        if (!getParameter(par, value)) {
            Q_EMIT error(QString("Can't read parameter %1").arg(name));
            continue;
        }

        KConfigSkeletonItem *i = p->findItem(name);
        if (i) {
            i->setProperty(value);
        }
    }
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

bool XlibBackend::getParameter(const Parameter *par, QVariant &value)
{
    PropertyInfo *p = getDevProperty(QLatin1String(par->prop_name));
    if (!p || par->prop_offset >= p->nitems) {
        return false;
    }

    value = p->value(par->prop_offset);
    return value.isValid();
}

bool XlibBackend::setParameter(const Parameter *par, const QVariant &value)
{
    QLatin1String propName(par->prop_name);
    PropertyInfo *p = getDevProperty(propName);
    if (!p || par->prop_offset >= p->nitems) {
        Q_EMIT error(QString("Can't read property %1").arg(propName));
        return false;
    }

    QVariant converted(value);
    QVariant::Type convType = QVariant::Int;
    if (p->f) {
        convType = QVariant::Double;
    }

    if (!converted.convert(convType)) {
        Q_EMIT error(QString("Can't convert value \"%2\" to %3 (parameter %1)")
                     .arg(par->name, value.toString(), QVariant::typeToName(convType)));
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
