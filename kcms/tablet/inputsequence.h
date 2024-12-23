/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QKeySequence>
#include <QStringList>
#include <QtQml>

/**
 * @brief Represents an input sequence that happens on a tablet button action, which could be a variety of types.
 * Currently suppoerted is "Disabled", "Keyboard", "Mouse" or "Application-defined".
 */
class InputSequence
{
    Q_GADGET

    Q_PROPERTY(InputSequence::Type type READ type WRITE setType)
public:
    /**
     * @brief What kind of event will be emitted.
     */
    enum class Type {
        Disabled, /** Emits nothing. */
        Keyboard, /** Emits a keyboard event. */
        RelativeKeyboard, /** Emits a different keyboard event depending on up/down movement. */
        Mouse, /** Emits a mouse event. */
        Pen, /** Emits a stylus pen (tool button) event. */
        Scroll, /** Emits a scrollwheel event. Only applicable to devices that emit relative deltas. */
        ApplicationDefined /** The tablet button is passed directly to the application. */
    };
    Q_ENUM(Type)

    /**
     * @brief Constructs an empty InputSequence.
     */
    InputSequence();

    /**
     * @brief Constructs an InputSequence from the kcminputrc format read by KWin.
     */
    explicit InputSequence(const QStringList &config);

    /**
     * @return The type of event to be emitted.
     */
    Type type() const;

    /**
     * @brief Sets the type of event to be emitted to @p type.
     */
    void setType(Type type);

    /**
     * @return This InputSequence serialized to the kcminputrc format read by KWin.
     */
    QStringList toConfigFormat() const;

    /**
     * @return A human-readable and localized string to be displayed in the UI.
     */
    Q_INVOKABLE QString toString() const;

    /**
     * @return The keyboard sequence. Will assert on a non-Keyboard type sequence.
     */
    Q_INVOKABLE QKeySequence keySequence() const;

    /**
     * @brief Sets the keyboard sequence. Will assert on a non-Keyboard type sequence.
     */
    Q_INVOKABLE void setKeySequence(const QKeySequence &sequence);

    /**
     * @return The up movement keyboard sequence. Will assert on a non-RelativeKeyboard type sequence.
     */
    Q_INVOKABLE QKeySequence upKeySequence() const;

    /**
     * @brief Sets the up movement keyboard sequence. Will assert on a non-RelativeKeyboard type sequence.
     */
    Q_INVOKABLE void setUpKeySequence(const QKeySequence &sequence);

    /**
     * @return The down movement keyboard sequence. Will assert on a non-RelativeKeyboard type sequence.
     */
    Q_INVOKABLE QKeySequence downKeySequence() const;

    /**
     * @brief Sets the down movement keyboard sequence. Will assert on a non-RelativeKeyboard type sequence.
     */
    Q_INVOKABLE void setDownKeySequence(const QKeySequence &sequence);

    /**
     * @return The mouse buttons to be emitted. Will assert on a non-Mouse type sequence.
     */
    Q_INVOKABLE Qt::MouseButton mouseButton() const;

    /**
     * @brief Sets the mouse buttons to be emitted. Will assert on a non-Mouse type sequence.
     */
    Q_INVOKABLE void setMouseButton(Qt::MouseButton button);

    /**
     * @return The keyboard modifiers to be emitted. Will assert on a non-Mouse type sequence.
     */
    Q_INVOKABLE Qt::KeyboardModifiers keyboardModifiers() const;

    /**
     * @brief Sets the keyboard modifiers to be emitted. Will assert on a non-Mouse type sequence.
     */
    Q_INVOKABLE void setKeyboardModifiers(Qt::KeyboardModifiers modifiers);

    /**
     * @return The pen (tablet tool) button to be emitted. Will assert on a non-Pen type sequence.
     */
    Q_INVOKABLE int penButton() const;

    /**
     * @brief Sets the pen (tablet tool) button to be emitted. Will assert on a non-Pen type sequence.
     */
    Q_INVOKABLE void setPenButton(int button);

private:
    using KeyData = QKeySequence;
    struct RelativeKeyData {
        QKeySequence up;
        QKeySequence down;
    };

    struct MouseSequence {
        Qt::MouseButton button;
        Qt::KeyboardModifiers modifiers;
    };
    using MouseData = MouseSequence;
    using PenData = int;
    using NoData = std::monostate;

    KeyData &keyData();
    RelativeKeyData &relativeKeyData();
    MouseData &mouseData();
    PenData &penData();

    KeyData keyData() const;
    RelativeKeyData relativeKeyData() const;
    MouseData mouseData() const;
    PenData penData() const;

    Type m_type = Type::ApplicationDefined;
    std::variant<KeyData, RelativeKeyData, MouseData, PenData, NoData> m_data;
};
