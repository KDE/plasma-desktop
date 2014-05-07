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
#include <QScopedPointer>
#include <QLatin1String>
#include <QStringList>
#include <QSharedPointer>
#include <QX11Info>

#include "touchpadbackend.h"

#include <xcb/xcb.h>

#include "xcbatom.h"

class XlibNotifications;
class XRecordKeyboardMonitor;

class XlibBackend : public TouchpadBackend
{
    Q_OBJECT

public:
    explicit XlibBackend(QObject *parent = 0);
    ~XlibBackend();

    bool applyConfig(const QVariantHash &);
    bool getConfig(QVariantHash &);
    const QStringList &supportedParameters() const { return m_supported; }
    const QString &errorString() const { return m_errorString; }

    void setTouchpadOff(TouchpadOffState);
    TouchpadOffState getTouchpadOff();

    void setTouchpadEnabled(bool);
    bool isTouchpadEnabled();

    void watchForEvents(bool keyboard);

    QStringList listMouses(const QStringList &blacklist);

private slots:
    void propertyChanged(xcb_atom_t);
    void touchpadDetached();
    void devicePlugged(int);

private:
    struct PropertyInfo *getDevProperty(const QLatin1String &propName);
    bool setParameter(const struct Parameter *, const QVariant &);
    QVariant getParameter(const struct Parameter *);
    void flush();
    double getPropertyScale(const QString &name) const;

    struct XDisplayCleanup {
        static void cleanup(Display *);
    };

    QScopedPointer<Display, XDisplayCleanup> m_display;
    xcb_connection_t *m_connection;

    XcbAtom m_floatType, m_capsAtom, m_enabledAtom, m_touchpadOffAtom,
    m_mouseAtom, m_keyboardAtom;

    int findTouchpad();
    int m_device;

    QMap<QLatin1String, QSharedPointer<XcbAtom> > m_atoms;
    QMap<QLatin1String, struct PropertyInfo> m_props;
    QSet<QLatin1String> m_changed;
    QStringList m_supported;
    QString m_errorString;
    int m_resX, m_resY;
    QStringList m_scaleByResX, m_scaleByResY, m_toRadians;
    QMap<QString, QString> m_negate;
    QScopedPointer<XlibNotifications> m_notifications;
    QScopedPointer<XRecordKeyboardMonitor> m_keyboard;
};

#endif // XLIBBACKEND_H
