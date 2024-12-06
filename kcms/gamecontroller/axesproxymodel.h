/*
    SPDX-FileCopyrightText: 2024 Arthur Kasimov <kodemeister@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QConcatenateTablesProxyModel>

class AxesModel;
class Device;
class HatModel;

// Concatenates rows of AxesModel and HatModel into a single model
class AxesProxyModel : public QConcatenateTablesProxyModel
{
    Q_OBJECT

    Q_PROPERTY(Device *device READ device WRITE setDevice REQUIRED)

public:
    explicit AxesProxyModel(QObject *parent = nullptr);

    Device *device() const;
    void setDevice(Device *device);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    AxesModel *m_axesModel = nullptr;
    HatModel *m_hatModel = nullptr;
};
