#include <cmath>

#include "xlibtouchpad.h"
#include <X11/Xlib-xcb.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <xserver-properties.h>

static QVariant negateVariant(const QVariant &value)
{
    if (value.type() == QVariant::Double) {
        return QVariant(-value.toDouble());
    } else if (value.type() == QVariant::Int) {
        return QVariant(-value.toInt());
    }
    return value;
}

XlibTouchpad::XlibTouchpad(Display *display, int deviceId)
    : m_display(display)
    , m_connection(XGetXCBConnection(display))
    , m_deviceId(deviceId)
{
    m_floatType.intern(m_connection, "FLOAT");
    m_enabledAtom.intern(m_connection, XI_PROP_ENABLED);
}

bool XlibTouchpad::applyConfig(const QVariantHash &p)
{
    m_props.clear();

    bool error = false;
    for (const QString &name : qAsConst(m_supported)) {
        QVariantHash::ConstIterator i = p.find(name);
        if (i == p.end()) {
            continue;
        }
        const Parameter *par = findParameter(name);
        if (par) {
            QVariant value(i.value());

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
                QVariantHash::ConstIterator i = p.find(m_negate[name]);
                if (i != p.end() && i.value().toBool()) {
                    value = negateVariant(value);
                }
            }

            if (name == "CoastingSpeed") {
                QVariantHash::ConstIterator coastingEnabled = p.find("Coasting");
                if (coastingEnabled != p.end() && !coastingEnabled.value().toBool()) {
                    value = QVariant(0);
                }
            }

            if (!setParameter(par, value)) {
                error = true;
            }
        }
    }

    flush();

    return !error;
}

bool XlibTouchpad::getConfig(QVariantHash &p)
{
    if (m_supported.isEmpty()) {
        return false;
    }

    m_props.clear();

    bool error = false;
    for (const QString &name : qAsConst(m_supported)) {
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
            bool negative = value.toDouble() < 0.0;
            p[m_negate[name]] = QVariant(negative);
            if (negative) {
                value = negateVariant(value);
            }
        }

        if (name == "CoastingSpeed") {
            bool coasting = value.toDouble() != 0.0;
            p["Coasting"] = QVariant(coasting);
            if (!coasting) {
                continue;
            }
        }

        p[name] = value;
    }

    return !error;
}

void XlibTouchpad::loadSupportedProperties(const Parameter *props)
{
    m_paramList = props;
    for (const Parameter *param = props; param->name; param++) {
        QLatin1String name(param->prop_name);

        if (!m_atoms.contains(name)) {
            m_atoms.insert(name, QSharedPointer<XcbAtom>(new XcbAtom(m_connection, param->prop_name)));
        }
    }

    for (const Parameter *p = props; p->name; p++) {
        if (getParameter(p).isValid()) {
            m_supported.append(p->name);
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
    for (const QLatin1String &name : qAsConst(m_changed)) {
        m_props[name].set();
    }
    m_changed.clear();

    XFlush(m_display);
}

double XlibTouchpad::getPropertyScale(const QString &name) const
{
    Q_UNUSED(name);
    return 1.0;
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

void XlibTouchpad::setEnabled(bool enable)
{
    PropertyInfo enabled(m_display, m_deviceId, m_enabledAtom.atom(), 0);
    if (enabled.b && *(enabled.b) != enable) {
        *(enabled.b) = enable;
        enabled.set();
    }

    flush();
}

bool XlibTouchpad::enabled()
{
    PropertyInfo enabled(m_display, m_deviceId, m_enabledAtom.atom(), 0);
    return enabled.value(0).toBool();
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
