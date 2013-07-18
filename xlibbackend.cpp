#include <cstring>

#include <QtAlgorithms>
#include <QScopedPointer>

#include "touchpadparameters.h"
#include "xlibbackend.h"

#include <X11/Xlib-xcb.h>
#include <X11/Xatom.h>
#include <xcb/xcb_atom.h>
#include <xcb/xinput.h>

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
    TouchpadBackend(parent), m_display(XOpenDisplay(0), XDisplayDeleter)
{
    qFill(m_caps, m_caps + TouchpadCapsCount, false);

    if (!m_display) {
        return;
    }

    m_connection = XGetXCBConnection(m_display.data());

    if (!m_connection) {
        return;
    }

    xcb_input_list_input_devices_cookie_t devicesCookie =
            xcb_input_list_input_devices(m_connection);

    XcbAtom touchpadType(m_connection, XI_TOUCHPAD);

    for (const Parameter *param = synapticsProperties; param->name; param++) {
        QLatin1String name(param->prop_name);

        if (!m_atoms.contains(name)) {
            m_atoms.insert(name, QSharedPointer<XcbAtom>(
                               new XcbAtom(m_connection, param->prop_name)));
        }
    }

    m_floatType.intern(m_connection, "FLOAT");
    XcbAtom capsAtom(m_connection, SYNAPTICS_PROP_CAPABILITIES);

    QScopedPointer<xcb_input_list_input_devices_reply_t> devicesReply(
                xcb_input_list_input_devices_reply(m_connection,
                                                   devicesCookie, 0));

    if (!touchpadType.atom() || !devicesReply) {
        return;
    }

    xcb_input_device_info_t *deviceInfo =
            xcb_input_list_input_devices_devices(devicesReply.data());
    if (!deviceInfo) {
        return;
    }

    int nDevices =
            xcb_input_list_input_devices_devices_length(devicesReply.data());

    for (int i = 0; i < nDevices && !m_device; i++) {
        if (deviceInfo[i].device_type == touchpadType.atom()) {
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

            Q_FOREACH(const QSharedPointer<XcbAtom> &atom, m_atoms) {
                if (!atom || !atom->atom()) {
                    continue;
                }
                for (int j = 0; j < nProperties; j++) {
                    if (properties.data()[j] == atom->atom()) {
                        m_device = device;
                        break;
                    }
                }
                if (m_device) {
                    break;
                }
            }
        }
    }

    if (!capsAtom.atom()) {
        return;
    }

    PropertyInfo caps(m_display.data(), m_device.data(), capsAtom.atom(), 0);
    if (!caps.b) {
        return;
    }

    for (unsigned i = 0; i < caps.nitems && i < TouchpadCapsCount; i++) {
        m_caps[i] = static_cast<bool>(caps.b[i]);
    }
}

XlibBackend::~XlibBackend()
{
}

void XlibBackend::applyConfig(const TouchpadParameters *p)
{
    if (!test()) {
        return;
    }

    m_props.clear();

    Q_FOREACH(KConfigSkeletonItem *i, p->items()) {
        setParameter(i->name(), i->property());
    }

    Q_FOREACH(const QLatin1String &name, m_changed) {
        if (!m_props.contains(name) || !m_atoms.contains(name) ||
                !m_atoms[name] || !m_atoms[name]->atom())
        {
            continue;
        }
        PropertyInfo &info = m_props[name];
        if (!info.type || !info.data) {
            continue;
        }
        XChangeDeviceProperty(m_display.data(), m_device.data(),
                              m_atoms[name]->atom(), info.type, info.format,
                              PropModeReplace, info.data.data(), info.nitems);
    }
    m_changed.clear();

    xcb_flush(m_connection);
}

void XlibBackend::getConfig(TouchpadParameters *p,
                            QStringList *supportedParameters)
{
    if (!test()) {
        return;
    }

    m_props.clear();

    int nRead = 0;
    Q_FOREACH(KConfigSkeletonItem *i, p->items()) {
        QString name(i->name());
        QVariant value;

        if (!getParameter(name, value)) {
            continue;
        }

        i->setProperty(value);
        nRead++;

        supportedParameters->append(name);
    }

    if (!nRead) {
        Q_EMIT error("Can't read X device properties");
        return;
    }

    if (!m_caps[TouchpadTwoFingerDetect]) {
        supportedParameters->removeAll("HorizTwoFingerScroll");
        supportedParameters->removeAll("VertTwoFingerScroll");
        supportedParameters->removeAll("TapButton2");
    }

    if (!m_caps[TouchpadThreeFingerDetect]) {
        supportedParameters->removeAll("TapButton3");
    }
}

bool XlibBackend::test()
{
    if (!m_display) {
        Q_EMIT error("Can't connect to X");
        return false;
    }

    if (!m_connection) {
        Q_EMIT error("Can't get XCB connection");
        return false;
    }

    if (!m_device) {
        Q_EMIT error("No touchpad found");
        return false;
    }

    return true;
}

static const Parameter *findParameter(const QString &name)
{
    const Parameter *par = synapticsProperties;
    while (par->name && name != par->name) {
        par++;
    }
    return par;
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
    if (p.data && !p.b && !p.f && !p.i) {
        Q_EMIT error(QString("Property \"%1\" has unknown type").arg(propName));
    }
    return &m_props.insert(propName, p).value();
}

bool XlibBackend::getParameter(const QString &name, QVariant &value)
{
    const Parameter *par = findParameter(name);
    if (!par) {
        return false;
    }

    PropertyInfo *p = getDevProperty(QLatin1String(par->prop_name));
    if (!p) {
        return false;
    }

    value = p->value(par->prop_offset);
    return value.isValid();
}

bool XlibBackend::setParameter(const QString &name, const QVariant &value)
{
    const Parameter *par = findParameter(name);
    if (!par) {
        return false;
    }

    QLatin1String propName(par->prop_name);
    PropertyInfo *p = getDevProperty(propName);
    if (!p) {
        return false;
    }

    QVariant old(p->value(par->prop_offset));
    if (!old.isValid()) {
        return false;
    }

    QVariant converted(value);
    QVariant::Type convType;
    if (p->b || p->i) {
        convType = QVariant::Int;
    } else if (p->f) {
        convType = QVariant::Double;
    } else {
        return false;
    }

    if (!converted.convert(convType)) {
        Q_EMIT error(QString("Can't convert value \"%2\" to %3 (parameter %1)")
                     .arg(name, value.toString(), QVariant::typeToName(convType)));
        return false;
    }

    if (converted == old) {
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
