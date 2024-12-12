/*
    SPDX-FileCopyrightText: 2024 Arthur Kasimov <kodemeister@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractTableModel>

#include "device.h"

// Provides position data from joystick POV hats. Each POV hat is mapped to a pair of X/Y model rows.
// On many gamepads D-pad is actually exposed as a POV hat instead of four regular buttons.
class HatModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(Device *device READ device WRITE setDevice REQUIRED)

public:
    explicit HatModel(QObject *parent = nullptr);

    Device *device() const;
    void setDevice(Device *device);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private Q_SLOTS:
    void onHatPositionChanged(int index);

private:
    Device *m_device = nullptr;
};
