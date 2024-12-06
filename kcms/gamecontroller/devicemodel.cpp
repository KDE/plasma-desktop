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

#include "device.h"
#include "gamepad.h"
#include "logging.h"

// 100 ms while we have devices
const int kShortPollTime = 100;
// 2 seconds while we don't have any devices
const int kLongPollTime = 2000;

static bool initialized = false;

DeviceModel::DeviceModel()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DeviceModel::poll);
    // Only poll once per 2 seconds until we have a device
    m_timer->start(kLongPollTime);

    // Also call it once after we have initialized in case
    // there are already controllers.
    QTimer::singleShot(kShortPollTime, this, &DeviceModel::poll);
}

DeviceModel::~DeviceModel()
{
    if (initialized) {
        qCDebug(KCM_GAMECONTROLLER) << "Calling SDL_Quit";
        qDeleteAll(m_devices);
        qDeleteAll(m_gamepads);
        SDL_Quit();
        initialized = false;
    }
}

Device *DeviceModel::device(SDL_JoystickID id) const
{
    return m_devices.value(id, nullptr);
}

Gamepad *DeviceModel::gamepad(SDL_JoystickID id) const
{
    return m_gamepads.value(id, nullptr);
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

    const SDL_JoystickID id = m_devices.keys().at(index.row());

    switch (role) {
    case CustomRoles::TextRole:
        return i18nc("Device name and path", "%1 (%2)", m_devices.value(id)->name(), m_devices.value(id)->path());
    case CustomRoles::IDRole:
        return id;
    }

    return {};
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    return {{CustomRoles::TextRole, QByteArrayLiteral("text")}, {CustomRoles::IDRole, QByteArrayLiteral("id")}};
}

void DeviceModel::poll()
{
    if (!initialized) {
        qCDebug(KCM_GAMECONTROLLER) << "Calling SDL_Init";
        SDL_Init(SDL_INIT_GAMECONTROLLER);
        initialized = true;
    }

    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_JOYDEVICEADDED:
            addDevice(event.jdevice.which);
            break;
        case SDL_JOYDEVICEREMOVED:
            removeDevice(event.jdevice.which);
            break;
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            if (m_devices.contains(event.jbutton.which)) {
                m_devices.value(event.jbutton.which)->onButtonEvent(event.jbutton);
            }
            break;
        case SDL_JOYAXISMOTION:
            if (m_devices.contains(event.jaxis.which)) {
                m_devices.value(event.jaxis.which)->onAxisEvent(event.jaxis);
            }
            break;
        case SDL_CONTROLLERAXISMOTION:
            if (m_gamepads.contains(event.caxis.which)) {
                m_gamepads.value(event.caxis.which)->onAxisEvent(event.caxis);
            }
            break;
        }
    }
}

void DeviceModel::addDevice(const int deviceIndex)
{
    auto device = std::make_unique<Device>(deviceIndex, this);
    if (!device->open()) {
        const QString error = QString::fromLocal8Bit(SDL_GetError());
        qCCritical(KCM_GAMECONTROLLER).nospace() << "Could not open device " << deviceIndex << ": " << error;
        return;
    }

    const SDL_JoystickID id = device->id();
    if (m_devices.contains(id)) {
        qCWarning(KCM_GAMECONTROLLER) << "Ignoring a duplicate device ID" << id << "from add event";
        return;
    }

    if (device->isVirtual()) {
        qCWarning(KCM_GAMECONTROLLER) << "Skipping device" << deviceIndex << "since it is virtual";
        return;
    }

    auto gamepad = std::make_unique<Gamepad>(deviceIndex, this);
    if (!gamepad->open()) {
        qCDebug(KCM_GAMECONTROLLER) << "Device" << deviceIndex << "is not a gamepad";
        gamepad.reset();
    }

    qCDebug(KCM_GAMECONTROLLER) << "Adding device" << deviceIndex << "with ID" << id;

    beginInsertRows(QModelIndex(), m_devices.count(), m_devices.count());
    m_devices.insert(id, device.release());
    if (gamepad) {
        m_gamepads.insert(id, gamepad.release());
    }
    endInsertRows();

    // Now that we have a device poll every short poll time
    m_timer->setInterval(kShortPollTime);
    Q_EMIT devicesChanged();
}

void DeviceModel::removeDevice(const SDL_JoystickID id)
{
    if (!m_devices.contains(id)) {
        qCWarning(KCM_GAMECONTROLLER) << "Ignoring an invalid device ID" << id << "from removal event";
        return;
    }

    const int index = m_devices.keys().indexOf(id);

    qCDebug(KCM_GAMECONTROLLER) << "Removing device with ID" << id;

    beginRemoveRows(QModelIndex(), index, index);
    m_devices.value(id)->deleteLater();
    m_devices.remove(id);
    if (m_gamepads.contains(id)) {
        m_gamepads.value(id)->deleteLater();
        m_gamepads.remove(id);
    }
    endRemoveRows();

    if (m_devices.count() == 0) {
        // Go back to long timeout polling now that we don't have a device
        m_timer->setInterval(kLongPollTime);
    }
    Q_EMIT devicesChanged();
}

int DeviceModel::count() const
{
    return m_devices.size();
}

#include "moc_devicemodel.cpp"
