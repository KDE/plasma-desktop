/*
    SPDX-FileCopyrightText: 2024 Arthur Kasimov <kodemeister@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QVector2D>
#include <QVector>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_joystick.h>

class Device : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVector2D leftAxis READ leftAxisValue NOTIFY leftAxisChanged)
    Q_PROPERTY(QVector2D rightAxis READ rightAxisValue NOTIFY rightAxisChanged)
    Q_PROPERTY(float leftTrigger READ leftTriggerValue NOTIFY leftTriggerChanged)
    Q_PROPERTY(float rigthTrigger READ rightTriggerValue NOTIFY rightTriggerChanged)

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

    int m_deviceIndex = -1;
    QVector2D m_leftAxis;
    QVector2D m_rightAxis;
    float m_leftTrigger;
    float m_rightTrigger;
    SDL_Joystick *m_joystick = nullptr;
};
