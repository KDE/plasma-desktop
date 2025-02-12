/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "backends/libinputcommon.h"

#include <config-build-options.h>

#include <QList>
#include <QObject>
#include <QVariantHash>

enum class TouchpadInputBackendMode {
    Unset = 0,
#if BUILD_KCM_TOUCHPAD_KWIN_WAYLAND
    WaylandLibinput = 1,
#endif
#if BUILD_KCM_TOUCHPAD_X11
    XLibinput = 2,
#endif
};

class Q_DECL_EXPORT TouchpadBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<LibinputCommon *> inputDevices READ inputDevices NOTIFY inputDevicesChanged FINAL)

protected:
    explicit TouchpadBackend(QObject *parent)
        : QObject(parent)
        , m_mode(TouchpadInputBackendMode::Unset)
    {
    }
    void setMode(TouchpadInputBackendMode mode);

public:
    static TouchpadBackend *implementation();

    TouchpadInputBackendMode getMode() const
    {
        return m_mode;
    }

    virtual bool save()
    {
        return false;
    }
    virtual bool load()
    {
        return false;
    }
    virtual bool defaults()
    {
        return false;
    }
    virtual bool isSaveNeeded() const
    {
        return false;
    }

    virtual QString errorString() const
    {
        return QString();
    }

    virtual QList<LibinputCommon *> inputDevices() const
    {
        return {};
    }
    virtual int deviceCount() const
    {
        return 0;
    }

    virtual bool isTouchpadAvailable() const
    {
        return false;
    }
    virtual bool isTouchpadSuspended() const
    {
        return false;
    }
    virtual void setTouchpadSuspended(bool)
    {
    }

    virtual void watchForEvents()
    {
    }

private:
    TouchpadInputBackendMode m_mode;

Q_SIGNALS:
    void touchpadStateChanged();
    void touchpadReset();
    void keyboardActivityStarted();
    void keyboardActivityFinished();

    void inputDevicesChanged();
    void deviceAdded(bool success);
    void deviceRemoved(int index);
};
