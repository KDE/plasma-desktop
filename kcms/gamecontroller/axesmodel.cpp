/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "axesmodel.h"

#include <SDL2/SDL_joystick.h>

AxesModel::AxesModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    connect(this, &AxesModel::deviceChanged, this, [this] {
        beginResetModel();
        endResetModel();

        if (m_device != nullptr) {
            connect(m_device, &Gamepad::axisStateChanged, this, [this](int index) {
                const QModelIndex changedIndex = this->index(index, 0);
                Q_EMIT dataChanged(changedIndex, changedIndex, {Qt::DisplayRole});
            });
        }
    });
}

int AxesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (m_device == nullptr) {
        return 0;
    }

    return SDL_JoystickNumAxes(m_device->joystick());
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
        return SDL_JoystickGetAxis(m_device->joystick(), index.row());
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
