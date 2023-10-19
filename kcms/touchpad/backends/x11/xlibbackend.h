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

#include "propertyinfo.h"
#include "xcbatom.h"

class XlibTouchpad;
class XlibNotifications;
class XRecordKeyboardMonitor;

class XlibBackend : public TouchpadBackend
{
    Q_OBJECT

    Q_PROPERTY(int touchpadCount READ touchpadCount CONSTANT)

public:
    static XlibBackend *initialize(QObject *parent = nullptr);
    ~XlibBackend();

    bool applyConfig(const QVariantHash &) override;
    bool applyConfig() override;
    bool getConfig(QVariantHash &) override;
    bool getConfig() override;
    bool getDefaultConfig() override;
    bool isChangedConfig() const override;
    QStringList supportedParameters() const override
    {
        return m_device ? m_device->supportedParameters() : QStringList();
    }
    QString errorString() const override
    {
        return m_errorString;
    }
    int touchpadCount() const override
    {
        return m_device ? 1 : 0;
    }

    void setTouchpadOff(TouchpadOffState) override;
    TouchpadOffState getTouchpadOff() override;

    bool isTouchpadAvailable() override;
    bool isTouchpadEnabled() override;
    void setTouchpadEnabled(bool) override;

    void watchForEvents(bool keyboard) override;

    QStringList listMouses(const QStringList &blacklist) override;
    QList<QObject *> getDevices() const override;

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

    XcbAtom m_enabledAtom, m_mouseAtom, m_keyboardAtom, m_touchpadAtom;
    XcbAtom m_libinputIdentifierAtom;

    XlibTouchpad *findTouchpad();
    std::unique_ptr<XlibTouchpad> m_device;

    QString m_errorString;
    std::unique_ptr<XlibNotifications> m_notifications;
    std::unique_ptr<XRecordKeyboardMonitor> m_keyboard;
};
