/*
 * Copyright 2018 Roman Gilg <subdiff@gmail.com>
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
    QString errorString() const override { return m_errorString; }

    virtual int deviceCount() const override { return 1; }
    virtual QVector<QObject*> getDevices() const override {
        return QVector<QObject*>(1, m_device);
    }

private:
    QObject *m_device;
    QString m_errorString = QString();
};

#endif // X11LIBINPUTBACKEND_H
