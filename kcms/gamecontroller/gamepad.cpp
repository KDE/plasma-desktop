/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamepad.h"

#include <QTimer>
#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>

Gamepad::Gamepad(SDL_Joystick *joystick, SDL_GameController *controller, QObject *parent)
    : QObject(parent)
    , m_joystick(joystick)
    , m_gameController(controller)
{
    m_name = QString::fromLocal8Bit(SDL_JoystickName(m_joystick));
    m_path = QString::fromLocal8Bit(SDL_JoystickPath(m_joystick));
}

QString Gamepad::name() const
{
    return m_name;
}

QString Gamepad::path() const
{
    return m_path;
}

void Gamepad::onButtonEvent(const SDL_ControllerButtonEvent sdlEvent)
{
    Q_EMIT buttonStateChanged(static_cast<SDL_GameControllerButton>(sdlEvent.button));
}

void Gamepad::onAxisEvent(const SDL_ControllerAxisEvent sdlEvent)
{
    const float value = static_cast<float>(sdlEvent.value) / std::numeric_limits<Sint16>::max();
    if (sdlEvent.axis == SDL_CONTROLLER_AXIS_LEFTX) {
        m_axis.setX(value);
        Q_EMIT axisValueChanged();
    } else if (sdlEvent.axis == SDL_CONTROLLER_AXIS_LEFTY) {
        m_axis.setY(value);
        Q_EMIT axisValueChanged();
    }

    Q_EMIT axisStateChanged(sdlEvent.axis);
}

SDL_Joystick *Gamepad::joystick() const
{
    return m_joystick;
}

SDL_GameController *Gamepad::gamecontroller() const
{
    return m_gameController;
}

QVector2D Gamepad::axisValue() const
{
    return QVector2D(m_axis);
}

#include "moc_gamepad.cpp"
