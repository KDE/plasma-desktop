/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamepad.h"

Gamepad::Gamepad(int deviceIndex, QObject *parent)
    : QObject(parent)
    , m_deviceIndex(deviceIndex)
{
}

Gamepad::~Gamepad()
{
    close();
}

bool Gamepad::open()
{
    if (m_gameController != nullptr) {
        return true;
    }

    m_gameController = SDL_GameControllerOpen(m_deviceIndex);
    return m_gameController != nullptr;
}

void Gamepad::close()
{
    if (m_gameController == nullptr) {
        return;
    }

    SDL_GameControllerClose(m_gameController);
    m_gameController = nullptr;
}

void Gamepad::onAxisEvent(const SDL_ControllerAxisEvent &event)
{
    const float value = static_cast<float>(event.value) / std::numeric_limits<Sint16>::max();
    if (event.axis == SDL_CONTROLLER_AXIS_LEFTX) {
        m_axis.setX(value);
        Q_EMIT axisValueChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_LEFTY) {
        m_axis.setY(value);
        Q_EMIT axisValueChanged();
    }
}

QVector2D Gamepad::axisValue() const
{
    return m_axis;
}

#include "moc_gamepad.cpp"
