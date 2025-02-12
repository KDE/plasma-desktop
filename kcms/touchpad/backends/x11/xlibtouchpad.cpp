#include <cmath>

#include "xlibtouchpad.h"
#include <X11/Xlib-xcb.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <xserver-properties.h>

XlibTouchpad::XlibTouchpad(Display *display, int deviceId)
    : m_display(display)
    , m_connection(XGetXCBConnection(display))
    , m_deviceId(deviceId)
{
    m_floatType.intern(m_connection, "FLOAT");
    m_enabledAtom.intern(m_connection, XI_PROP_ENABLED);
}

void XlibTouchpad::loadSupportedProperties(const Parameter *props)
{
    m_paramList = props;
    for (const Parameter *param = props; param->name; param++) {
        QLatin1String name(param->prop_name);

        if (!m_atoms.contains(name)) {
            m_atoms.insert(name, std::make_shared<XcbAtom>(m_connection, param->prop_name));
        }
    }
}

QVariant XlibTouchpad::getParameter(const Parameter *par)
{
    PropertyInfo *p = getDevProperty(QLatin1String(par->prop_name));
    if (!p || par->prop_offset >= p->nitems) {
        return QVariant();
    }

    return p->value(par->prop_offset);
}

void XlibTouchpad::flush()
{
    for (const QLatin1String &name : std::as_const(m_changed)) {
        m_props[name].set();
    }
    m_changed.clear();

    XFlush(m_display);
}

PropertyInfo *XlibTouchpad::getDevProperty(const QLatin1String &propName)
{
    if (m_props.contains(propName)) {
        return &m_props[propName];
    }

    if (!m_atoms.contains(propName) || !m_atoms[propName]) {
        return nullptr;
    }

    xcb_atom_t prop = m_atoms[propName]->atom();
    if (!prop) {
        return nullptr;
    }

    PropertyInfo p(m_display, m_deviceId, prop, m_floatType.atom());
    if (!p.b && !p.f && !p.i) {
        return nullptr;
    }
    return &m_props.insert(propName, p).value();
}

bool XlibTouchpad::setParameter(const Parameter *par, const QVariant &value)
{
    QLatin1String propName(par->prop_name);
    PropertyInfo *p = getDevProperty(propName);
    if (!p || par->prop_offset >= p->nitems) {
        return false;
    }

    QVariant converted(value);
    QMetaType::Type convType = QMetaType::Type::Int;
    if (p->f) {
        convType = QMetaType::Type::Double;
    } else if (value.typeId() == QMetaType::Type::Double) {
        converted = QVariant(qRound(static_cast<qreal>(value.toDouble())));
    }

    if (!converted.convert(QMetaType(convType))) {
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

void XlibTouchpad::setSuspended(bool suspend)
{
    PropertyInfo enabled(m_display, m_deviceId, m_enabledAtom.atom(), 0);
    if (enabled.b && *(enabled.b) == suspend) {
        *(enabled.b) = !suspend;
        enabled.set();
    }

    flush();
}

bool XlibTouchpad::isSuspended()
{
    PropertyInfo enabled(m_display, m_deviceId, m_enabledAtom.atom(), 0);
    return !enabled.value(0).toBool();
}

const Parameter *XlibTouchpad::findParameter(const QString &name)
{
    for (const Parameter *par = m_paramList; par->name; par++) {
        if (name == par->name) {
            return par;
        }
    }
    return nullptr;
}
