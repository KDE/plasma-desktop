/*
    SPDX-FileCopyrightText: 2024 Arthur Kasimov <kodemeister@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "axesproxymodel.h"

#include <KLocalizedString>

#include "axesmodel.h"
#include "device.h"
#include "hatmodel.h"

AxesProxyModel::AxesProxyModel(QObject *parent)
    : QConcatenateTablesProxyModel(parent)
    , m_axesModel(new AxesModel(this))
    , m_hatModel(new HatModel(this))
{
    addSourceModel(m_axesModel);
    addSourceModel(m_hatModel);
}

Device *AxesProxyModel::device() const
{
    return m_axesModel->device();
}

void AxesProxyModel::setDevice(Device *device)
{
    m_axesModel->setDevice(device);
    m_hatModel->setDevice(device);
}

QVariant AxesProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
        return QString::number(section + 1);
    }

    return QConcatenateTablesProxyModel::headerData(section, orientation, role);
}

#include "moc_axesproxymodel.cpp"
