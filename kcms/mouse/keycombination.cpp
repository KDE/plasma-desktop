/*
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keycombination.h"

KeyCombination::KeyCombination(QObject *parent)
    : QObject(parent)
{
}

KeyCombination::~KeyCombination() = default;

/* Basically, an observable toString(). */
QString KeyCombination::display() const
{
    return m_keySequence.toString(QKeySequence::NativeText);
}

QKeySequence KeyCombination::keySequence() const
{
    return m_keySequence;
}

void KeyCombination::setKeySequence(const QKeySequence &sequence)
{
    if (m_keySequence != sequence) {
        m_keySequence = sequence;
        Q_EMIT keySequenceChanged();
    }
}

bool KeyCombination::shift() const
{
    return m_keySequence[0].keyboardModifiers() & Qt::ShiftModifier;
}

void KeyCombination::setShift(bool on)
{
    setKeyboardModifier(Qt::ShiftModifier, on);
}

bool KeyCombination::control() const
{
    return m_keySequence[0].keyboardModifiers() & Qt::ControlModifier;
}

void KeyCombination::setControl(bool on)
{
    setKeyboardModifier(Qt::ControlModifier, on);
}

bool KeyCombination::alt() const
{
    return m_keySequence[0].keyboardModifiers() & Qt::AltModifier;
}

void KeyCombination::setAlt(bool on)
{
    setKeyboardModifier(Qt::AltModifier, on);
}

bool KeyCombination::meta() const
{
    return m_keySequence[0].keyboardModifiers() & Qt::MetaModifier;
}

void KeyCombination::setMeta(bool on)
{
    setKeyboardModifier(Qt::MetaModifier, on);
}

Qt::Key KeyCombination::key() const
{
    return m_keySequence[0].key();
}

void KeyCombination::setKey(Qt::Key value)
{
    if (key() != value) {
        const auto modifiers = m_keySequence[0].keyboardModifiers();
        m_keySequence = QKeySequence(QKeyCombination(modifiers, value));
        Q_EMIT keySequenceChanged();
        Q_EMIT keySequenceModified();
    }
}

void KeyCombination::setKeyboardModifier(Qt::KeyboardModifier keyboardModifier, bool newValue)
{
    const auto oldValue = m_keySequence[0].keyboardModifiers() & keyboardModifier;
    if (oldValue != newValue) {
        auto keyboardModifiers = m_keySequence[0].keyboardModifiers();
        keyboardModifiers.setFlag(keyboardModifier, newValue);
        const auto key = m_keySequence[0].key();
        m_keySequence = QKeySequence(QKeyCombination(keyboardModifiers, key));
        Q_EMIT keySequenceChanged();
        Q_EMIT keySequenceModified();
    }
}

#include "moc_keycombination.cpp"
