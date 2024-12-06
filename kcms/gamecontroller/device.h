/*
    SPDX-FileCopyrightText: 2024 Arthur Kasimov <kodemeister@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_joystick.h>

class Device : public QObject
{
    Q_OBJECT

public:
    Device(int deviceIndex, QObject *parent = nullptr);
    ~Device();

    bool open();
    void close();

    SDL_JoystickID id() const;
    QString name() const;
    QString path() const;

    bool isVirtual() const;

    int buttonCount() const;
    bool buttonState(int index) const;

    int axisCount() const;
    int axisValue(int index) const;

Q_SIGNALS:
    void buttonStateChanged(int index);
    void axisValueChanged(int index);

private:
    friend class DeviceModel;

    void onButtonEvent(const SDL_JoyButtonEvent &event);
    void onAxisEvent(const SDL_JoyAxisEvent &event);

    int m_deviceIndex = -1;
    SDL_Joystick *m_joystick = nullptr;
};
