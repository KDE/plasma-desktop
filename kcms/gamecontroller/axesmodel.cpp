/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "axesmodel.h"

#include <KLocalizedString>

AxesModel::AxesModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

Device *AxesModel::device() const
{
    return m_device;
}

void AxesModel::setDevice(Device *device)
{
    if (device == m_device) {
        return;
    }

    beginResetModel();
    if (m_device != nullptr) {
        disconnect(m_device, &Device::leftAxisChanged, this, &AxesModel::onLeftAxisChanged);
        disconnect(m_device, &Device::rightAxisChanged, this, &AxesModel::onRightAxisChanged);
    }
    m_device = device;
    if (m_device != nullptr) {
        connect(m_device, &Device::leftAxisChanged, this, &AxesModel::onLeftAxisChanged);
        connect(m_device, &Device::rightAxisChanged, this, &AxesModel::onRightAxisChanged);
    }
    endResetModel();
}

void AxesModel::onLeftAxisChanged()
{
    const QModelIndex changedIndex = this->index(0, 0);
    Q_EMIT dataChanged(changedIndex, changedIndex, {Qt::DisplayRole});
    const QModelIndex changedIndex2 = this->index(1, 0);
    Q_EMIT dataChanged(changedIndex2, changedIndex2, {Qt::DisplayRole});
}

void AxesModel::onRightAxisChanged()
{
    const QModelIndex changedIndex = this->index(2, 0);
    Q_EMIT dataChanged(changedIndex, changedIndex, {Qt::DisplayRole});
    const QModelIndex changedIndex2 = this->index(3, 0);
    Q_EMIT dataChanged(changedIndex2, changedIndex2, {Qt::DisplayRole});
}

int AxesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (m_device == nullptr) {
        return 0;
    }

    return m_device->axisCount();
}

int AxesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant AxesModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index) || m_device == nullptr) {
        return {};
    }

    if (index.column() == 0 && role == Qt::DisplayRole) {
        // Use left axis for 0 and 1, right for 2 and 3
        const QVector2D position = (index.row() == 0 || index.row() == 1) ? m_device->leftAxisValue() : m_device->rightAxisValue();
        return QString::number(position[index.row() % 2]);
    }

    return {};
}

QVariant AxesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal && section == 0) {
            return i18nc("@label Axis value", "Value");
        } else if (orientation == Qt::Vertical) {
            return QString::number(section + 1);
        }
    }

    return {};
}

#include "moc_axesmodel.cpp"
