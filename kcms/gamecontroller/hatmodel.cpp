/*
    SPDX-FileCopyrightText: 2024 Arthur Kasimov <kodemeister@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "hatmodel.h"

#include <KLocalizedString>

HatModel::HatModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

Device *HatModel::device() const
{
    return m_device;
}

void HatModel::setDevice(Device *device)
{
    if (device == m_device) {
        return;
    }

    beginResetModel();
    if (m_device != nullptr) {
        disconnect(m_device, &Device::hatPositionChanged, this, &HatModel::onHatPositionChanged);
    }
    m_device = device;
    if (m_device != nullptr) {
        connect(m_device, &Device::hatPositionChanged, this, &HatModel::onHatPositionChanged);
    }
    endResetModel();
}

void HatModel::onHatPositionChanged(int index)
{
    const QModelIndex firstIndex = this->index(index * 2, 0);
    const QModelIndex lastIndex = this->index(index * 2 + 1, 0);
    Q_EMIT dataChanged(firstIndex, lastIndex, {Qt::DisplayRole});
}

int HatModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (m_device == nullptr) {
        return 0;
    }

    return m_device->hatCount() * 2;
}

int HatModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant HatModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index) || m_device == nullptr) {
        return {};
    }

    if (index.column() == 0 && role == Qt::DisplayRole) {
        const QVector2D position = m_device->hatPosition(index.row() / 2);
        return QString::number(position[index.row() % 2]);
    }

    return {};
}

QVariant HatModel::headerData(int section, Qt::Orientation orientation, int role) const
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

#include "moc_hatmodel.cpp"
