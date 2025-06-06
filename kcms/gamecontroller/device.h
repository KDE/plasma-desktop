/*
    SPDX-FileCopyrightText: 2024 Arthur Kasimov <kodemeister@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QVector2D>
#include <QVector>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_joystick.h>

class Device : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVector2D leftAxis READ leftAxisValue NOTIFY leftAxisChanged)
    Q_PROPERTY(QVector2D rightAxis READ rightAxisValue NOTIFY rightAxisChanged)
    Q_PROPERTY(float leftTrigger READ leftTriggerValue NOTIFY leftTriggerChanged)
    Q_PROPERTY(float rightTrigger READ rightTriggerValue NOTIFY rightTriggerChanged)

public:
    Device(int deviceIndex, QObject *parent = nullptr);
    ~Device();

    bool open();
    void close();

    // Whether or not this device is a GameController
    bool isController() const;
    SDL_JoystickID id() const;
    QString name() const;
    QString path() const;

    bool isVirtual() const;

    int buttonCount() const;
    bool buttonState(int index) const;

    int axisCount() const;
    QVector2D leftAxisValue() const;
    QVector2D rightAxisValue() const;
    float leftTriggerValue() const;
    float rightTriggerValue() const;

    int hatCount() const;
    QVector2D hatPosition(int index) const;

Q_SIGNALS:
    void buttonStateChanged(int index);
    void leftAxisChanged();
    void rightAxisChanged();
    void leftTriggerChanged();
    void rightTriggerChanged();
    void hatPositionChanged(int index);

private:
    friend class DeviceModel;

    void onButtonEvent(const SDL_JoyButtonEvent &event);
    void onAxisEvent(const SDL_JoyAxisEvent &event);
    void onHatEvent(const SDL_JoyHatEvent &event);
    // The same as above but using SDL_Controller*Event types
    void onControllerButtonEvent(const SDL_ControllerButtonEvent &event);
    void onControllerAxisEvent(const SDL_ControllerAxisEvent &event);

    int m_deviceIndex = -1;
    QVector2D m_leftAxis;
    QVector2D m_rightAxis;
    float m_leftTrigger;
    float m_rightTrigger;
    SDL_Joystick *m_joystick = nullptr;
    SDL_GameController *m_controller = nullptr;
};
