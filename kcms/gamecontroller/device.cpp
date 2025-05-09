/*
    SPDX-FileCopyrightText: 2024 Arthur Kasimov <kodemeister@outlook.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "device.h"

#include <KLocalizedString>

#include <SDL2/SDL_hidapi.h>

#include "logging.h"
#include "udevmatcher.h"

// NOTE: Keep in sync with SDL_JoystickType from SDL_joystick.h
static QStringList kJoystickTypeNames = {i18n("Unknown"),
                                         i18n("Game Controller"),
                                         i18n("Wheel"),
                                         i18n("Arcade Stick"),
                                         i18n("Flight Stick"),
                                         i18n("Dance Pad"),
                                         i18n("Guitar"),
                                         i18n("Drum Kit"),
                                         i18n("Arcade Pad"),
                                         i18n("Throttle")};

// NOTE: Keep in sync with SDL_gamecontrollertype from SDL_gamecontroller.h
static QStringList kControllerTypeNames = {
    i18n("Unknown"),
    i18n("XBox 360"),
    i18n("XBox One"),
    i18n("Sony Dualshock 3"),
    i18n("Sony Dualshock 4"),
    i18n("Switch Pro"),
    i18n("Virtual"),
    i18n("Sony Dualsense"),
    i18n("Amazon Luna"),
    i18n("Google Stadia"),
    i18n("NVidia Shield"),
    i18n("Switch Left Joycon"),
    i18n("Switch Right Joycon"),
    i18n("Switch Paired Joycon"),
};

// NOTE: Keep in sync with SDL_GameControllerButton from SDL_gamecontroller.h
static QStringList kButtonNames = {
    i18nc("The game controller button with a letter A on it", "A button"),
    i18nc("The game controller button with a letter B on it", "B button"),
    i18nc("The game controller button with a letter X on it", "X button"),
    i18nc("The game controller button with a letter Y on it", "Y button"),
    i18nc("The back game controller button", "Back button"),
    i18nc("The guide game controller button", "Guide button"),
    i18nc("The start game controller button", "Start button"),
    i18nc("The button under the game controller's left analog stick", "Left analog stick"),
    i18nc("The button under the game controller's right analog stick", "Right analog stick"),
    i18nc("The game controller's left shoulder button", "Left shoulder button"),
    i18nc("The game controller's right shoulder button", "Right shoulder button"),
    i18nc("The game controller's d-pad Up button", "D-pad up button"),
    i18nc("The game controller's d-pad down button", "D-pad down button"),
    i18nc("The game controller's d-pad left button", "D-pad left button"),
    i18nc("The game controller's d-pad right button", "D-pad right button"),
    i18nc("The game controller's miscellaneous button (Share on Xbox, mic on Playstation/Luna, capture on Switch)",
          "Miscellaneous button"), /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
    i18nc("The game controller's 1st paddle button", "Paddle button 1"), /* Xbox Elite paddle P1 (upper left, facing the back) */
    i18nc("The game controller's 2nd paddle button", "Paddle button 2"), /* Xbox Elite paddle P3 (upper right, facing the back) */
    i18nc("The game controller's 3rd paddle button", "Paddle button 3"), /* Xbox Elite paddle P2 (lower left, facing the back) */
    i18nc("The game controller's 4th paddle button", "Paddle button 4"), /* Xbox Elite paddle P4 (lower right, facing the back) */
    i18nc("The button under the game controller's touchpad", "Touchpad button"), /* PS4/PS5 touchpad button */
};

static QStringList kConnectionTypeString = {
    i18nc("Unknown controller connection type", "Unknown"),
    i18nc("USB connected controller", "USB"),
    i18nc("Bluetooth connected controller", "Bluetooth"),
};

QString ButtonToButtonName(SDL_GameControllerButton button)
{
    if (button > SDL_CONTROLLER_BUTTON_INVALID && button < SDL_CONTROLLER_BUTTON_MAX) {
        return kButtonNames.at(button);
    } else {
        return i18n("Invalid button value");
    }
}

// Get the SDL hid info for the joystick with the given vendor, and product
SDL_hid_device_info getJoystickHidInfo(short vendor, short product, std::string path)
{
    SDL_hid_device_info *info = SDL_hid_enumerate(vendor, product);
    SDL_hid_device_info *current = info;

    qCDebug(KCM_GAMECONTROLLER) << "Checking for hidapi with vendor: " << vendor << " and product: " << product;

    SDL_hid_device_info result;
    if (info) {
        if (!info->next) {
            // We found our device, so copy the data we are interested in.
            // currently only the interface number, but more later maybe
            result.interface_number = info->interface_number;

            qCDebug(KCM_GAMECONTROLLER) << "Found hidapi data for path " << info->path;
        } else {
            // Set to unknown in case we don't find a match
            result.interface_number = -2;

            // There are more than one device with the same vendor and product
            // so iterate through them looking for ours.
            qCDebug(KCM_GAMECONTROLLER) << "Found multiple hidapi data with that vendor and product id";
            auto to_match = UDevMatcher::deviceToUdevId(path);

            qCDebug(KCM_GAMECONTROLLER) << "Looking for device with this udev id: " << to_match;

            while (current) {
                auto test_id = UDevMatcher::deviceToUdevId(current->path);
                qCDebug(KCM_GAMECONTROLLER) << "Found device: " << current->path << " udev id: " << test_id;

                if (to_match == test_id) {
                    result.interface_number = current->interface_number;

                    qCDebug(KCM_GAMECONTROLLER) << "Found hidapi data for path " << current->path;
                    current = nullptr;
                } else {
                    current = current->next;
                }
            }
        }
        SDL_hid_free_enumeration(info);
    } else {
        result.interface_number = -2;
    }

    return result;
}

Device::Device(int deviceIndex, QObject *parent)
    : QObject(parent)
    , m_deviceIndex(deviceIndex)
    , m_connectionType(UnknownType)
{
}

Device::~Device()
{
    close();
}

bool Device::open()
{
    if (m_joystick != nullptr) {
        return true;
    }

    m_joystick = SDL_JoystickOpen(m_deviceIndex);

    short vendor = SDL_JoystickGetVendor(m_joystick);
    short product = SDL_JoystickGetProduct(m_joystick);
    auto path = SDL_JoystickPath(m_joystick);
    qCDebug(KCM_GAMECONTROLLER) << "Device path: " << path;
    SDL_hid_device_info info = getJoystickHidInfo(vendor, product, path);
    if (info.interface_number == -2) {
        // Couldn't find hidapi device, so unknown
        m_connectionType = UnknownType;
    } else if (info.interface_number == -1) {
        m_connectionType = BluetoothType;
    } else {
        m_connectionType = USBType;
    }

    m_controller = SDL_GameControllerOpen(m_deviceIndex);
    if (!m_controller) {
        m_leftTrigger = SDL_JoystickGetAxis(m_joystick, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        m_rightTrigger = SDL_JoystickGetAxis(m_joystick, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
        qCDebug(KCM_GAMECONTROLLER) << "Device" << m_deviceIndex << "is not a gamepad. using as joystick";
        m_buttonCount = SDL_JoystickNumButtons(m_joystick);
    } else {
        m_leftTrigger = SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        m_rightTrigger = SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

        // We have a controller, so check which buttons we have
        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
            // If we have this button type, add it to our map and increase count.
            if (SDL_GameControllerHasButton(m_controller, static_cast<SDL_GameControllerButton>(i))) {
                m_buttonType[m_buttonCount] = i;
                m_buttonCount++;
            }
        }
    }
    return m_joystick != nullptr;
}

void Device::close()
{
    if (m_joystick == nullptr) {
        return;
    }

    if (m_controller) {
        SDL_GameControllerClose(m_controller);
        m_controller = nullptr;
    }

    SDL_JoystickClose(m_joystick);
    m_joystick = nullptr;
}

bool Device::isController() const
{
    return m_controller != nullptr;
}

SDL_JoystickID Device::id() const
{
    return SDL_JoystickInstanceID(m_joystick);
}

QString Device::name() const
{
    return QString::fromLocal8Bit(SDL_JoystickName(m_joystick));
}

QString Device::path() const
{
    return QString::fromLocal8Bit(SDL_JoystickPath(m_joystick));
}

QString Device::type() const
{
    return kJoystickTypeNames.at(SDL_JoystickGetType(m_joystick));
}

SDL_GameControllerType Device::controllerType() const
{
    if (m_controller) {
        return SDL_GameControllerGetType(m_controller);
    }
    return SDL_CONTROLLER_TYPE_UNKNOWN;

}

QString Device::controllerTypeName() const
{
    if (m_controller) {
        return kControllerTypeNames.at(SDL_GameControllerGetType(m_controller));
    } else {
        return i18n("Not a controller");
    }
}

QString Device::connectionType() const
{
    return kConnectionTypeString.at(m_connectionType);
}

bool Device::isVirtual() const
{
    return SDL_JoystickIsVirtual(m_deviceIndex);
}

int Device::buttonCount() const
{
    return m_buttonCount;
}

bool Device::buttonState(int index) const
{
    // If we are a game controller, use that api to get button state
    if (m_controller) {
        return SDL_GameControllerGetButton(m_controller, static_cast<SDL_GameControllerButton>(m_buttonType.value(index))) != 0;
    }
    return SDL_JoystickGetButton(m_joystick, index) != 0;
}

QString Device::buttonName(int index) const
{
    if (m_controller) {
        return ButtonToButtonName(static_cast<SDL_GameControllerButton>(m_buttonType.value(index)));
    }
    return QString::number(index + 1);
}

int Device::axisCount() const
{
    return SDL_JoystickNumAxes(m_joystick);
}

QVector2D Device::leftAxisValue() const
{
    return m_leftAxis;
}

QVector2D Device::rightAxisValue() const
{
    return m_rightAxis;
}

float Device::leftTriggerValue() const
{
    return m_leftTrigger;
}

float Device::rightTriggerValue() const
{
    return m_rightTrigger;
}

int Device::hatCount() const
{
    return SDL_JoystickNumHats(m_joystick);
}

QVector2D Device::hatPosition(int index) const
{
    const Uint8 hat = SDL_JoystickGetHat(m_joystick, index);
    QVector2D position;

    if ((hat & SDL_HAT_LEFT) != 0) {
        position.setX(std::numeric_limits<Sint16>::min());
    } else if ((hat & SDL_HAT_RIGHT) != 0) {
        position.setX(std::numeric_limits<Sint16>::max());
    }

    if ((hat & SDL_HAT_UP) != 0) {
        position.setY(std::numeric_limits<Sint16>::min());
    } else if ((hat & SDL_HAT_DOWN) != 0) {
        position.setY(std::numeric_limits<Sint16>::max());
    }

    return position;
}

void Device::onButtonEvent(const SDL_JoyButtonEvent &event)
{
    Q_EMIT buttonStateChanged(event.button);
}

void Device::onControllerButtonEvent(const SDL_ControllerButtonEvent &event)
{
    Q_EMIT buttonStateChanged(event.button);
}

void Device::onAxisEvent(const SDL_JoyAxisEvent &event)
{
    const float value = static_cast<float>(event.value) / std::numeric_limits<Sint16>::max();
    if (event.axis == SDL_CONTROLLER_AXIS_LEFTX) {
        m_leftAxis.setX(value);
        Q_EMIT leftAxisChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_LEFTY) {
        m_leftAxis.setY(value);
        Q_EMIT leftAxisChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
        m_rightAxis.setX(value);
        Q_EMIT rightAxisChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
        m_rightAxis.setY(value);
        Q_EMIT rightAxisChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
        m_leftTrigger = value;
        Q_EMIT leftTriggerChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
        m_rightTrigger = value;
        Q_EMIT rightTriggerChanged();
    }
}

void Device::onControllerAxisEvent(const SDL_ControllerAxisEvent &event)
{
    const float value = static_cast<float>(event.value) / std::numeric_limits<Sint16>::max();
    if (event.axis == SDL_CONTROLLER_AXIS_LEFTX) {
        m_leftAxis.setX(value);
        Q_EMIT leftAxisChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_LEFTY) {
        m_leftAxis.setY(value);
        Q_EMIT leftAxisChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
        m_rightAxis.setX(value);
        Q_EMIT rightAxisChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
        m_rightAxis.setY(value);
        Q_EMIT rightAxisChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
        m_leftTrigger = value;
        Q_EMIT leftTriggerChanged();
    } else if (event.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
        m_rightTrigger = value;
        Q_EMIT rightTriggerChanged();
    }
}

void Device::onHatEvent(const SDL_JoyHatEvent &event)
{
    Q_EMIT hatPositionChanged(event.hat);
}

#include "moc_device.cpp"
