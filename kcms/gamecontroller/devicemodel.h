/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>
#include <QMap>
#include <QPointer>

#include <SDL2/SDL_joystick.h>

#include "device.h"

class QTimer;

class DeviceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY devicesChanged)

public:
    enum CustomRoles {
        TextRole = Qt::UserRole + 1,
        IDRole,
        TypeRole,
    };

    DeviceModel();
    virtual ~DeviceModel();

    Q_INVOKABLE Device *device(SDL_JoystickID id) const;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const;

Q_SIGNALS:
    void devicesChanged();

private Q_SLOTS:
    void poll();

private:
    void addDevice(const int deviceIndex);
    void removeDevice(const SDL_JoystickID id);

    // Map of sdl indexes to Gamepad devices
    QMap<SDL_JoystickID, Device *> m_devices;
    QPointer<QTimer> m_timer;
};
