/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector2D>

#include <KLocalizedString>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_joystick.h>

class Gamepad : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVector2D axisValue READ axisValue NOTIFY axisValueChanged)

public:
    Gamepad(SDL_Joystick *joystick, SDL_GameController *controller, QObject *parent = nullptr);

    QString name() const;
    QString path() const;

    SDL_Joystick *joystick() const;
    SDL_GameController *gamecontroller() const;

    QVector2D axisValue() const;

Q_SIGNALS:
    void buttonStateChanged(SDL_GameControllerButton button);
    void axisStateChanged(int index);
    void axisValueChanged();

private:
    friend class DeviceModel;

    void onButtonEvent(SDL_ControllerButtonEvent sdlEvent);
    void onAxisEvent(SDL_ControllerAxisEvent sdlEvent);

    SDL_Joystick *m_joystick = nullptr;
    SDL_GameController *m_gameController = nullptr;

    QString m_name;
    QString m_path;
    QPointF m_axis = QPointF(0.0, 0.0);
};
