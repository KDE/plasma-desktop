/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamepadbutton.h"

GamepadButton::GamepadButton(QObject *parent)
    : QObject(parent)
    , m_state(false)
{
}

void GamepadButton::setState(const bool state)
{
    if (state != m_state) {
        m_state = state;
        Q_EMIT stateChanged();
    }
}

bool GamepadButton::state() const
{
    return m_state;
}
