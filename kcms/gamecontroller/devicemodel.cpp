/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicemodel.h"

#include <KLocalizedString>
#include <QTimer>

#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>

#include "gamepad.h"
#include "logging.h"

DeviceModel::DeviceModel()
{
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DeviceModel::poll);
    timer->start(1);
}

Gamepad *DeviceModel::device(int index) const
{
    if (index < 0 || index >= m_devices.count())
        return nullptr;

    const int sdlIndex = m_devices.keys().at(index);
    return m_devices.value(sdlIndex);
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_devices.count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    if (role == Qt::DisplayRole) {
        const int sdlIndex = m_devices.keys().at(index.row());

        return i18nc("Device name and path", "%1 (%2)", m_devices.value(sdlIndex)->name(), m_devices.value(sdlIndex)->path());
    }

    return {};
}

void DeviceModel::poll()
{
    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED:
            addDevice(event.cdevice.which);
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            removeDevice(event.cdevice.which);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            m_devices.value(event.cbutton.which)->onButtonEvent(event.cbutton);
            break;
        case SDL_CONTROLLERAXISMOTION:
            m_devices.value(event.caxis.which)->onAxisEvent(event.caxis);
            break;
        }
    }
}

void DeviceModel::addDevice(const int deviceIndex)
{
    const auto joystick = SDL_JoystickOpen(deviceIndex);
    const auto id = SDL_JoystickInstanceID(joystick);

    if (m_devices.contains(id)) {
        qCWarning(KCM_GAMECONTROLLER) << "Got a duplicate add event, ignoring. Index: " << deviceIndex;
        return;
    }

    const auto gamepad = SDL_GameControllerOpen(deviceIndex);
    if (SDL_GameControllerTypeForIndex(deviceIndex) == SDL_CONTROLLER_TYPE_VIRTUAL) {
        qCWarning(KCM_GAMECONTROLLER) << "Skipping gamepad since it is virtual. Index: " << deviceIndex;
        return;
    }

    beginInsertRows(QModelIndex(), m_devices.count(), m_devices.count());
    m_devices.insert(id, new Gamepad(joystick, gamepad, this));
    endInsertRows();

    Q_EMIT devicesChanged();
}

void DeviceModel::removeDevice(const int deviceIndex)
{
    if (!m_devices.contains(deviceIndex)) {
        qCWarning(KCM_GAMECONTROLLER) << "Invalid device index from removal event, ignoring";
        return;
    }

    const int index = m_devices.keys().indexOf(deviceIndex);

    beginRemoveRows(QModelIndex(), index, index);
    m_devices.value(deviceIndex)->deleteLater();
    m_devices.remove(deviceIndex);
    endRemoveRows();

    Q_EMIT devicesChanged();
}

int DeviceModel::count() const
{
    return m_devices.size();
}

#include "moc_devicemodel.cpp"
