/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QVector2D>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>

class Gamepad : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVector2D axisValue READ axisValue NOTIFY axisValueChanged)

public:
    Gamepad(int deviceIndex, QObject *parent = nullptr);
    ~Gamepad();

    bool open();
    void close();

    QVector2D axisValue() const;

Q_SIGNALS:
    void axisValueChanged();

private:
    friend class DeviceModel;

    void onAxisEvent(const SDL_ControllerAxisEvent &event);

    int m_deviceIndex = -1;
    SDL_GameController *m_gameController = nullptr;
    QVector2D m_axis;
};
