/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "inputsequence.h"
#include "logging.h"

#include <KLocalizedString>

using namespace Qt::StringLiterals;

// KWin needs the button defined in linux's input-event-code.h
constexpr int BTN_LEFT = 0x110;
constexpr int BTN_RIGHT = 0x111;
constexpr int BTN_MIDDLE = 0x112;

constexpr int BTN_STYLUS = 0x14b;
constexpr int BTN_STYLUS2 = 0x14c;
constexpr int BTN_STYLUS3 = 0x149;

InputSequence::InputSequence() = default;

InputSequence::InputSequence(const QStringList &config)
{
    if (config.empty()) {
        return;
    }

    const QString &type = config.first();
    if (type == u"Disabled"_s) {
        setType(Type::Disabled);
    } else if (type == u"Key"_s) {
        setType(Type::Keyboard);

        if (config.size() == 2) {
            keyData() = config.last();
        }
    } else if (type == u"MouseButton"_s) {
        setType(Type::Mouse);

        // At least has a valid mouse button
        if (config.size() >= 2) {
            const auto linuxButton = config[1].toInt();
            if (linuxButton == BTN_LEFT) {
                mouseData().button = Qt::LeftButton;
            } else if (linuxButton == BTN_RIGHT) {
                mouseData().button = Qt::RightButton;
            } else if (linuxButton == BTN_MIDDLE) {
                mouseData().button = Qt::MiddleButton;
            }

            // Also has a keyboard modifier at the end
            if (config.size() >= 3) {
                const auto keyboardModsRaw = config[2].toInt();
                mouseData().modifiers = Qt::KeyboardModifiers{keyboardModsRaw};
            }
        }
    } else if (type == u"ApplicationDefined"_s) {
        setType(Type::ApplicationDefined);
    } else if (type == u"TabletToolButton"_s) {
        setType(Type::Pen);

        if (config.size() >= 2) {
            const auto linuxButton = config[1].toInt();
            if (linuxButton == BTN_STYLUS) {
                penData() = 0;
            } else if (linuxButton == BTN_STYLUS2) {
                penData() = 1;
            } else if (linuxButton == BTN_STYLUS3) {
                penData() = 2;
            }
        }
    } else {
        qCWarning(KCM_TABLET) << "Unknown input sequence type" << type;
    }
}

InputSequence::Type InputSequence::type() const
{
    return m_type;
}

void InputSequence::setType(const Type type)
{
    if (m_type != type) {
        m_type = type;

        // Make sure to reset the data when the type changed
        switch (m_type) {
        case Type::Disabled:
        case Type::ApplicationDefined:
            m_data = NoData{};
            break;
        case Type::Keyboard:
            m_data = KeyData{QString{}};
            break;
        case Type::Mouse:
            m_data = MouseData{.button = Qt::LeftButton, .modifiers = {}};
            break;
        case Type::Pen:
            m_data = PenData{0};
            break;
        default:
            Q_UNREACHABLE();
        }
    }
}

QStringList InputSequence::toConfigFormat() const
{
    switch (m_type) {
    case Type::Disabled:
        return QStringList{u"Disabled"_s};
    case Type::Keyboard: {
        const auto key = keyData().toString(QKeySequence::PortableText);
        return QStringList{u"Key"_s, key};
    }
    case Type::Mouse: {
        const auto mouse = mouseData();

        int linuxButton;
        switch (mouse.button) {
        case Qt::LeftButton:
            linuxButton = BTN_LEFT;
            break;
        case Qt::RightButton:
            linuxButton = BTN_RIGHT;
            break;
        case Qt::MiddleButton:
            linuxButton = BTN_MIDDLE;
            break;
        default:
            Q_UNREACHABLE();
        }

        QStringList values{u"MouseButton"_s, QString::number(linuxButton)};
        if (mouse.modifiers != Qt::NoModifier) {
            values.append(QString::number(mouse.modifiers.toInt()));
        }
        return values;
    }
    case Type::ApplicationDefined:
        return QStringList{}; // to pass events through, just don't give KWin anything
    case Type::Pen: {
        int linuxButton;
        switch (penData()) {
        case 0:
            linuxButton = BTN_STYLUS;
            break;
        case 1:
            linuxButton = BTN_STYLUS2;
            break;
        case 2:
            linuxButton = BTN_STYLUS3;
            break;
        default:
            Q_UNREACHABLE();
        }
        return QStringList{"TabletToolButton", QString::number(linuxButton)};
    }
    default:
        Q_UNREACHABLE();
    }
}

QString InputSequence::toString() const
{
    switch (m_type) {
    case Type::Disabled:
        return i18nc("@action:button This action is disabled", "Disabled");
    case Type::Keyboard: {
        if (keyData().isEmpty()) {
            return i18nc("@action:button There is no keybinding", "None");
        }
        return keyData().toString(QKeySequence::NativeText);
    }
    case Type::Mouse: {
        switch (mouseData().button) {
        case Qt::LeftButton:
            return i18nc("@action:button", "Left mouse button");
        case Qt::RightButton:
            return i18nc("@action:button", "Right mouse button");
        case Qt::MiddleButton:
            return i18nc("@action:button", "Middle mouse button");
        default:
            Q_UNREACHABLE();
        }
    }
    case Type::ApplicationDefined:
        return i18nc("@action:button", "Application-defined");
    case Type::Pen:
        return i18nc("@action:button", "Pen Button %1", penData() + 1);
    default:
        Q_UNREACHABLE();
    }
}

QKeySequence InputSequence::keySequence() const
{
    return keyData();
}

void InputSequence::setKeySequence(const QKeySequence &sequence)
{
    keyData() = sequence;
}

Qt::MouseButton InputSequence::mouseButton() const
{
    return mouseData().button;
}

void InputSequence::setMouseButton(const Qt::MouseButton button)
{
    mouseData().button = button;
}

Qt::KeyboardModifiers InputSequence::keyboardModifiers() const
{
    return mouseData().modifiers;
}

void InputSequence::setKeyboardModifiers(const Qt::KeyboardModifiers modifiers)
{
    mouseData().modifiers = modifiers;
}

int InputSequence::penButton() const
{
    return penData();
}

void InputSequence::setPenButton(const int button)
{
    penData() = button;
}

InputSequence::KeyData &InputSequence::keyData()
{
    Q_ASSERT(m_type == Type::Keyboard);
    return std::get<KeyData>(m_data);
}

InputSequence::MouseData &InputSequence::mouseData()
{
    Q_ASSERT(m_type == Type::Mouse);
    return std::get<MouseData>(m_data);
}

InputSequence::PenData &InputSequence::penData()
{
    Q_ASSERT(m_type == Type::Pen);
    return std::get<PenData>(m_data);
}

InputSequence::KeyData InputSequence::keyData() const
{
    Q_ASSERT(m_type == Type::Keyboard);
    return std::get<KeyData>(m_data);
}

InputSequence::MouseData InputSequence::mouseData() const
{
    Q_ASSERT(m_type == Type::Mouse);
    return std::get<MouseData>(m_data);
}

InputSequence::PenData InputSequence::penData() const
{
    Q_ASSERT(m_type == Type::Pen);
    return std::get<PenData>(m_data);
}
#include "moc_inputsequence.cpp"
