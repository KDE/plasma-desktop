/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>

#include <QDebug>
#include <QVector2D>
#include <Solid/Device>
#include <libevdev-1.0/libevdev/libevdev.h>

class JoyDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(int numButtons MEMBER m_numButtons CONSTANT)
    Q_PROPERTY(int numAxes MEMBER m_numAxes CONSTANT)
    Q_PROPERTY(bool hasRumble MEMBER m_hasRumble CONSTANT)
    Q_PROPERTY(QStringList buttonState READ getButtonState NOTIFY buttonStateChanged)
    Q_PROPERTY(QStringList axisState READ getAxisState NOTIFY axisStateChanged)
public:
    explicit JoyDevice(const Solid::Device &device, QObject *parent = nullptr);

    QString getName() const
    {
        return m_name;
    }

    QStringList getButtonState()
    {
        QStringList data;
        for (auto button : m_buttonState) {
            data.push_back(button ? "1" : "0");
        }
        return data;
    }

    QStringList getAxisState()
    {
        QStringList data;
        for (auto axis : m_axisState) {
            data.push_back("(" + QString::number(axis.x()) + ", " + QString::number(axis.y()) + ")");
        }
        return data;
    }

    void poll();

signals:
    void buttonStateChanged();
    void axisStateChanged();

private:
    void processEvent(struct input_event &ev);
    float normalize(int code, __s32 value);

    libevdev *m_device = nullptr;

    QString m_name;
    int m_numButtons = 0;
    int m_numAxes = 0;
    bool m_hasRumble = false;

    QVector<bool> m_buttonState;
    QVector<QVector2D> m_axisState;
};