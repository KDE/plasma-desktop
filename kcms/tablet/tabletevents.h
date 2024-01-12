/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QQuickItem>

class TabletEvents : public QQuickItem
{
    Q_OBJECT
public:
    TabletEvents(QQuickItem *parent = nullptr);

Q_SIGNALS:
    void padButtonsChanged(const QString &path, uint buttonCount);
    void padButtonReceived(const QString &path, uint button, bool pressed);
    void toolButtonReceived(uint32_t hardware_serial_hi, uint32_t hardware_serial_lo, uint button, bool pressed);
    void toolDown(uint32_t hardware_serial_hi, uint32_t hardware_serial_lo, double x, double y);
    void toolMotion(uint32_t hardware_serial_hi, uint32_t hardware_serial_lo, double x, double y);
    void toolUp(uint32_t hardware_serial_hi, uint32_t hardware_serial_lo, double x, double y);
};
