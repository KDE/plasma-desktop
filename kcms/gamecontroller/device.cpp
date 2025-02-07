/*
    SPDX-FileCopyrightText: 2024 Arthur Kasimov <kodemeister@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "device.h"

Device::Device(int deviceIndex, QObject *parent)
    : QObject(parent)
    , m_deviceIndex(deviceIndex)
{
}

Device::~Device()
{
    close();
}

bool Device::open()
{
    if (m_joystick != nullptr) {
        return true;
    }

    m_joystick = SDL_JoystickOpen(m_deviceIndex);
    m_leftTrigger = SDL_JoystickGetAxis(m_joystick, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    m_rightTrigger = SDL_JoystickGetAxis(m_joystick, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

    return m_joystick != nullptr;
}

void Device::close()
{
    if (m_joystick == nullptr) {
        return;
    }

    SDL_JoystickClose(m_joystick);
    m_joystick = nullptr;
}

SDL_JoystickID Device::id() const
{
    return SDL_JoystickInstanceID(m_joystick);
}

QString Device::name() const
{
    return QString::fromLocal8Bit(SDL_JoystickName(m_joystick));
}

QString Device::path() const
{
    return QString::fromLocal8Bit(SDL_JoystickPath(m_joystick));
}

bool Device::isVirtual() const
{
    return SDL_JoystickIsVirtual(m_deviceIndex);
}

int Device::buttonCount() const
{
    return SDL_JoystickNumButtons(m_joystick);
}

bool Device::buttonState(int index) const
{
    return SDL_JoystickGetButton(m_joystick, index) != 0;
}

int Device::axisCount() const
{
    return SDL_JoystickNumAxes(m_joystick);
}

QVector2D Device::leftAxisValue() const
{
    return m_leftAxis;
}

QVector2D Device::rightAxisValue() const
{
    return m_rightAxis;
}

float Device::leftTriggerValue() const
{
    return m_leftTrigger;
}

float Device::rightTriggerValue() const
{
    return m_rightTrigger;
}

int Device::hatCount() const
{
    return SDL_JoystickNumHats(m_joystick);
}

QVector2D Device::hatPosition(int index) const
{
    const Uint8 hat = SDL_JoystickGetHat(m_joystick, index);
    QVector2D position;

    if ((hat & SDL_HAT_LEFT) != 0) {
        position.setX(std::numeric_limits<Sint16>::min());
    } else if ((hat & SDL_HAT_RIGHT) != 0) {
        position.setX(std::numeric_limits<Sint16>::max());
    }

    if ((hat & SDL_HAT_UP) != 0) {
        position.setY(std::numeric_limits<Sint16>::min());
    } else if ((hat & SDL_HAT_DOWN) != 0) {
        position.setY(std::numeric_limits<Sint16>::max());
    }

    return position;
}

void Device::onButtonEvent(const SDL_JoyButtonEvent &event)
{
    Q_EMIT buttonStateChanged(event.button);
}

void Device::onAxisEvent(const SDL_JoyAxisEvent &event)
{
    const float value = static_cast<float>(event.value) / std::numeric_limits<Sint16>::max();
    if (event.axis == SDL_CONTROLLER_AXIS_LEFTX) {
        m_leftAxis.setX(value);
        Q_EMIT leftAxisChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_LEFTY) {
        m_leftAxis.setY(value);
        Q_EMIT leftAxisChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
        m_rightAxis.setX(value);
        Q_EMIT rightAxisChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
        m_rightAxis.setY(value);
        Q_EMIT rightAxisChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
        m_leftTrigger = value;
        Q_EMIT leftTriggerChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
        m_rightTrigger = value;
        Q_EMIT rightTriggerChanged();
    }
}

void Device::onHatEvent(const SDL_JoyHatEvent &event)
{
    Q_EMIT hatPositionChanged(event.hat);
}

#include "moc_device.cpp"
