/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>

class GamepadButton : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool state READ state NOTIFY stateChanged)

public:
    enum SDL_Buttons {
        // From SDL_gamecontroller.h so we can use them in qml
        SDL_CONTROLLER_BUTTON_INVALID = -1,
        SDL_CONTROLLER_BUTTON_A,
        SDL_CONTROLLER_BUTTON_B,
        SDL_CONTROLLER_BUTTON_X,
        SDL_CONTROLLER_BUTTON_Y,
        SDL_CONTROLLER_BUTTON_BACK,
        SDL_CONTROLLER_BUTTON_GUIDE,
        SDL_CONTROLLER_BUTTON_START,
        SDL_CONTROLLER_BUTTON_LEFTSTICK,
        SDL_CONTROLLER_BUTTON_RIGHTSTICK,
        SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
        SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
        SDL_CONTROLLER_BUTTON_DPAD_UP,
        SDL_CONTROLLER_BUTTON_DPAD_DOWN,
        SDL_CONTROLLER_BUTTON_DPAD_LEFT,
        SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
        SDL_CONTROLLER_BUTTON_MISC1, /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
        SDL_CONTROLLER_BUTTON_PADDLE1, /* Xbox Elite paddle P1 */
        SDL_CONTROLLER_BUTTON_PADDLE2, /* Xbox Elite paddle P3 */
        SDL_CONTROLLER_BUTTON_PADDLE3, /* Xbox Elite paddle P2 */
        SDL_CONTROLLER_BUTTON_PADDLE4, /* Xbox Elite paddle P4 */
        SDL_CONTROLLER_BUTTON_TOUCHPAD, /* PS4/PS5 touchpad button */
    };
    Q_ENUMS(SDL_Buttons)

    enum SDL_Axes {
        // From SDL_gamecontroller.h to use in qml
        SDL_CONTROLLER_AXIS_INVALID = -1,
        SDL_CONTROLLER_AXIS_LEFTX, // Use LEFTX and RIGHTX for stick axes
        SDL_CONTROLLER_AXIS_LEFTY,
        SDL_CONTROLLER_AXIS_RIGHTX,
        SDL_CONTROLLER_AXIS_RIGHTY,
        SDL_CONTROLLER_AXIS_TRIGGERLEFT, // Use these for triggers
        SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
        SDL_CONTROLLER_AXIS_MAX
    };
    Q_ENUMS(SDL_Axes)

    explicit GamepadButton(QObject *parent = nullptr);

    bool state() const;

    void setState(bool state);

Q_SIGNALS:
    void stateChanged();

private:
    friend class Device;

    bool m_state;
};
