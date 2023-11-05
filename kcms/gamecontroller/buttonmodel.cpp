/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "buttonmodel.h"

#include "gamepad.h"

#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_joystick.h>

ButtonModel::ButtonModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    connect(this, &ButtonModel::deviceChanged, this, [this] {
        beginResetModel();
        endResetModel();

        if (m_device != nullptr) {
            connect(m_device, &Gamepad::buttonStateChanged, this, [this](int index) {
                const QModelIndex changedIndex = this->index(index, 0);
                Q_EMIT dataChanged(changedIndex, changedIndex, {ButtonStateRole});
            });
        }
    });
}

int ButtonModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (m_device == nullptr) {
        return 0;
    }

    return SDL_JoystickNumButtons(m_device->joystick());
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

    if (index.column() == 0 && role == ButtonStateRole) {
        return SDL_GameControllerGetButton(m_device->gamecontroller(), SDL_GameControllerButton(index.row()));
    }

    return {};
}

QVariant ButtonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal && section == 0) {
            return i18nc("@label Button state", "State");
        } else if (orientation == Qt::Vertical) {
            return QVariant::fromValue(section + 1);
        }
    }

    return {};
}

QHash<int, QByteArray> ButtonModel::roleNames() const
{
    return {{ButtonStateRole, QByteArrayLiteral("buttonState")}};
}

#include "moc_buttonmodel.cpp"
