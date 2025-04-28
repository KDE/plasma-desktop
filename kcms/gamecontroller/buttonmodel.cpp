/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "buttonmodel.h"

#include <KLocalizedString>

ButtonModel::ButtonModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

Device *ButtonModel::device() const
{
    return m_device;
}

void ButtonModel::setDevice(Device *device)
{
    if (device == m_device) {
        return;
    }

    beginResetModel();
    if (m_device != nullptr) {
        disconnect(m_device, &Device::buttonStateChanged, this, &ButtonModel::onButtonStateChanged);
    }
    m_device = device;
    if (m_device != nullptr) {
        connect(m_device, &Device::buttonStateChanged, this, &ButtonModel::onButtonStateChanged);
    }
    endResetModel();
}

void ButtonModel::onButtonStateChanged(int index)
{
    const QModelIndex changedIndex = this->index(index, 0);
    Q_EMIT dataChanged(changedIndex, changedIndex, {Qt::DisplayRole});
}

int ButtonModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (m_device == nullptr) {
        return 0;
    }

    return m_device->buttonCount();
}

int ButtonModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ButtonModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index) || m_device == nullptr) {
        return {};
    }

    if (index.column() == 0 && role == Qt::DisplayRole) {
        const bool pressed = m_device->buttonState(index.row());
        return pressed ? i18nc("Status of a gamepad button", "PRESSED") : QStringLiteral("-");
    }

    return {};
}

QVariant ButtonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal && section == 0) {
            return i18nc("@label Button state", "State");
        } else if (orientation == Qt::Vertical) {
            return m_device->buttonName(section);
        }
    }

    return {};
}

#include "moc_buttonmodel.cpp"
