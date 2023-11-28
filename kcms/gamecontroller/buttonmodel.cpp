/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "buttonmodel.h"
#include "gamepad.h"

#include <SDL2/SDL_joystick.h>

#include <KLocalizedString>

ButtonModel::ButtonModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    connect(this, &ButtonModel::deviceChanged, this, &ButtonModel::initDeviceButtons);
}

void ButtonModel::initDeviceButtons()
{
    beginResetModel();
    m_buttons.clear();

    if (!m_device) {
        endResetModel();
        return;
    }

    const int numButtons = SDL_JoystickNumButtons(m_device->joystick());
    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
        const SDL_GameControllerButton button = static_cast<SDL_GameControllerButton>(i);
        if (SDL_GameControllerHasButton(m_device->gamecontroller(), button)) {
            m_buttons << button;
            if (m_buttons.count() == numButtons) {
                break;
            }
        }
    }

    endResetModel();

    connect(m_device, &Gamepad::buttonStateChanged, this, [this](SDL_GameControllerButton button) {
        const int row = m_buttons.indexOf(button);
        if (row >= 0) {
            const QModelIndex changedIndex = index(row, 0);
            Q_EMIT dataChanged(changedIndex, changedIndex, {Qt::DisplayRole});
        }
    });
}

int ButtonModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_buttons.count();
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
        const int pressed = SDL_GameControllerGetButton(m_device->gamecontroller(), m_buttons.at(index.row()));
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
            return QString::number(section + 1);
        }
    }

    return {};
}

#include "moc_buttonmodel.cpp"
