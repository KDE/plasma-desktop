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
#include "xlibtouchpad.h"

#include <xcb/xcb.h>

#include "xcbatom.h"
#include "propertyinfo.h"

class XlibTouchpad;
class XlibNotifications;
class XRecordKeyboardMonitor;

class XlibBackend : public TouchpadBackend
{
    Q_OBJECT

public:
    static XlibBackend* initialize(QObject *parent = 0);
    ~XlibBackend();

    bool applyConfig(const QVariantHash &) Q_DECL_OVERRIDE;
    bool getConfig(QVariantHash &) Q_DECL_OVERRIDE;
    QStringList supportedParameters() const Q_DECL_OVERRIDE {
        return m_device ? m_device->supportedParameters() : QStringList();
    }
    const QString &errorString() const { return m_errorString; }

    void setTouchpadOff(TouchpadOffState) Q_DECL_OVERRIDE;
    TouchpadOffState getTouchpadOff() Q_DECL_OVERRIDE;

    bool isTouchpadAvailable() Q_DECL_OVERRIDE;
    bool isTouchpadEnabled() Q_DECL_OVERRIDE;
    void setTouchpadEnabled(bool) Q_DECL_OVERRIDE;

    void watchForEvents(bool keyboard) Q_DECL_OVERRIDE;

    QStringList listMouses(const QStringList &blacklist) Q_DECL_OVERRIDE;

private slots:
    void propertyChanged(xcb_atom_t);
    void touchpadDetached();
    void devicePlugged(int);

protected:
    explicit XlibBackend(QObject *parent);

    struct XDisplayCleanup {
        static void cleanup(Display *);
    };

    QScopedPointer<Display, XDisplayCleanup> m_display;
    xcb_connection_t *m_connection;

    XcbAtom m_enabledAtom, m_mouseAtom, m_keyboardAtom, m_touchpadAtom;
    XcbAtom m_synapticsIdentifierAtom;
    XcbAtom m_libinputIdentifierAtom;

    XlibTouchpad *findTouchpad();
    QScopedPointer<XlibTouchpad> m_device;

    QString m_errorString;
    QScopedPointer<XlibNotifications> m_notifications;
    QScopedPointer<XRecordKeyboardMonitor> m_keyboard;
};

#endif // XLIBBACKEND_H
