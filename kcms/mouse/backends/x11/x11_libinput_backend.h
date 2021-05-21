/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef X11LIBINPUTBACKEND_H
#define X11LIBINPUTBACKEND_H

#include "x11_backend.h"

#include <QVector>

class X11LibinputBackend : public X11Backend
{
    Q_OBJECT

    Q_PROPERTY(int deviceCount READ deviceCount CONSTANT)

public:
    explicit X11LibinputBackend(QObject *parent = nullptr);
    ~X11LibinputBackend() = default;

    void kcmInit() override;

    bool applyConfig() override;
    bool getConfig() override;
    bool getDefaultConfig() override;
    bool isChangedConfig() const override;
    QString errorString() const override
    {
        return m_errorString;
    }

    virtual int deviceCount() const override
    {
        return 1;
    }
    virtual QVector<QObject *> getDevices() const override
    {
        return QVector<QObject *>(1, m_device);
    }

private:
    QObject *m_device;
    QString m_errorString = QString();
};

#endif // X11LIBINPUTBACKEND_H
