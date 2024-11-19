/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputdevice.h"

#include <config-build-options.h>

#include <QList>
#include <QObject>
#include <QVariantHash>

class InputBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<InputDevice *> inputDevices READ inputDevices NOTIFY inputDevicesChanged FINAL)
    Q_PROPERTY(bool isAnonymousInputDevice READ isAnonymousInputDevice CONSTANT FINAL)

protected:
    explicit InputBackend() = default;

public:
    static std::unique_ptr<InputBackend> create();
    static void registerImplementationTypes(const char *uri);

    virtual void kcmInit()
    {
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

    virtual int deviceCount() const
    {
        return 0;
    }

    virtual bool isAnonymousInputDevice() const
    {
        return false;
    }

    virtual QList<InputDevice *> inputDevices() const
    {
        return {};
    }

    Q_INVOKABLE virtual QVariantMap buttonMapping([[maybe_unused]] const QString &deviceName) const
    {
        return {};
    }

    Q_INVOKABLE virtual void setButtonMapping([[maybe_unused]] const QString &deviceName, [[maybe_unused]] const QVariantMap &mapping)
    {
    }

    // Because QML can't QVariantMap::count() on its own.
    Q_INVOKABLE int buttonMappingCount(const QString &deviceName) const
    {
        return buttonMapping(deviceName).count();
    }

Q_SIGNALS:
    void needsSaveChanged();

    void inputDevicesChanged();
    void deviceAdded(bool success);
    void deviceRemoved(int index);
    void buttonMappingChanged();
};
