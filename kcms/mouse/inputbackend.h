/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <config-build-options.h>

#include <QList>
#include <QObject>
#include <QVariantHash>

class InputBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject *> inputDevices READ inputDevices NOTIFY inputDevicesChanged FINAL)
    Q_PROPERTY(bool isAnonymousInputDevice READ isAnonymousInputDevice CONSTANT FINAL)

protected:
    explicit InputBackend(QObject *parent)
        : QObject(parent)
    {
    }

public:
    static InputBackend *implementation(QObject *parent = nullptr);

    virtual void kcmInit()
    {
    }

    virtual bool applyConfig()
    {
        return false;
    }
    virtual bool getConfig()
    {
        return false;
    }

    virtual bool getDefaultConfig()
    {
        return false;
    }
    virtual bool isChangedConfig() const
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
    virtual QList<QObject *> inputDevices() const
    {
        return QList<QObject *>();
    }

Q_SIGNALS:
    void inputDevicesChanged();
    void deviceAdded(bool success);
    void deviceRemoved(int index);
};
