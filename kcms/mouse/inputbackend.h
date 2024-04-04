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
#include <memory> // std::unique_ptr

class InputBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject *> inputDevices READ getDevices NOTIFY devicesChanged)
    Q_PROPERTY(bool isAnonymousInputDevice READ isAnonymousDevice CONSTANT)

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

    virtual bool isValid() const
    {
        return false;
    }

    virtual void load()
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
    virtual bool isAnonymousDevice() const
    {
        return false;
    }
    virtual QList<QObject *> getDevices() const
    {
        return QList<QObject *>();
    }

Q_SIGNALS:
    void devicesChanged();
    void deviceAdded(bool success);
    void deviceRemoved(int index);
};
