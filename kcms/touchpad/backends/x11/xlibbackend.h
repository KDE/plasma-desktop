/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QLatin1String>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <QtGui/private/qtx11extras_p.h>

#include "libinputtouchpad.h"
#include "touchpadbackend.h"
#include "xlibtouchpad.h"

#include <xcb/xcb.h>

#include "xcbatom.h"

class XlibTouchpad;
class XlibNotifications;

class XlibBackend : public TouchpadBackend
{
    Q_OBJECT

public:
    static XlibBackend *initialize(QObject *parent = nullptr);
    ~XlibBackend();

    bool applyConfig() override;
    bool getConfig() override;
    bool getDefaultConfig() override;
    bool isChangedConfig() const override;
    QString errorString() const override
    {
        return m_errorString;
    }
    int deviceCount() const override
    {
        return m_device ? 1 : 0;
    }

    void setTouchpadOff(TouchpadOffState) override;
    TouchpadOffState getTouchpadOff() override;

    bool isTouchpadAvailable() override;
    bool isTouchpadEnabled() override;
    void setTouchpadEnabled(bool) override;

    void watchForEvents() override;

    QList<LibinputCommon *> inputDevices() const override;

private Q_SLOTS:
    void propertyChanged(xcb_atom_t);
    void touchpadDetached();
    void devicePlugged(int);

protected:
    explicit XlibBackend(QObject *parent);

    struct XDisplayCleanup {
        void operator()(Display *);
    };

    std::unique_ptr<Display, XDisplayCleanup> m_display;
    xcb_connection_t *m_connection;

    XcbAtom m_enabledAtom;
    XcbAtom m_touchpadAtom;
    XcbAtom m_libinputIdentifierAtom;

    LibinputTouchpad *findTouchpad();
    std::unique_ptr<LibinputTouchpad> m_device;

    QString m_errorString;
    std::unique_ptr<XlibNotifications> m_notifications;
};
