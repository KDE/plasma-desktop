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

#ifndef XLIBBACKEND_H
#define XLIBBACKEND_H

#include <QMap>
#include <QSet>
#include <QSharedPointer>
#include <QLatin1String>

#include "touchpadbackend.h"

#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <X11/extensions/XInput.h>
#include <xcb/xinput.h>

#include "xcbatom.h"

class XlibBackend : public TouchpadBackend
{
    Q_OBJECT

public:
    explicit XlibBackend(QObject *parent = 0);
    ~XlibBackend();

    bool applyConfig(const TouchpadParameters *);
    bool getConfig(TouchpadParameters *);
    const QStringList &supportedParameters() const { return m_supported; }
    const QString &errorString() const { return m_errorString; }

private:
    struct PropertyInfo *getDevProperty(const QLatin1String &propName);
    bool setParameter(const struct Parameter *, const QVariant &);
    QVariant getParameter(const struct Parameter *);
    int getPropertyScale(const QString &name) const;

    QSharedPointer<Display> m_display;
    xcb_connection_t *m_connection;

    XcbAtom m_floatType, m_touchpadType, m_capsAtom;
    xcb_input_list_input_devices_cookie_t m_devicesCookie;

    QSharedPointer<XDevice> findTouchpad();
    QSharedPointer<XDevice> m_device;

    QMap<QLatin1String, QSharedPointer<XcbAtom> > m_atoms;
    QMap<QLatin1String, struct PropertyInfo> m_props;
    QSet<QLatin1String> m_changed;
    QStringList m_supported;
    QString m_errorString;
    int m_resX, m_resY;
    QStringList m_scaleByResX, m_scaleByResY;
    QMap<QString, QString> m_negate;
};

#endif // XLIBBACKEND_H
