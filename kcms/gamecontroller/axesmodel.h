/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractTableModel>

#include "device.h"

class AxesModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(Device *device READ device WRITE setDevice REQUIRED)

public:
    explicit AxesModel(QObject *parent = nullptr);

    Device *device() const;
    void setDevice(Device *device);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private Q_SLOTS:
    void onLeftAxisChanged();
    void onRightAxisChanged();

private:
    Device *m_device = nullptr;
};
