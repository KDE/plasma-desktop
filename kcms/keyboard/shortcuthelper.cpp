/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "shortcuthelper.h"

#include <KGlobalAccel>

#include "bindings.h"
#include "debug.h"

namespace
{
static const QKeySequence DefaultAlternativeShortcut = QKeySequence(Qt::META | Qt::ALT | Qt::Key_K);
static const QKeySequence DefaultLastUsedShortcut = QKeySequence(Qt::META | Qt::ALT | Qt::Key_L);
}

ShortcutHelper::ShortcutHelper(QObject *parent)
    : QObject(parent)
    , m_actionCollection(new KeyboardLayoutActionCollection(this, true))
{
}

QKeySequence ShortcutHelper::alternativeShortcut() const
{
    return m_alternativeShortcut;
}

void ShortcutHelper::setAlternativeShortcut(const QKeySequence &sequence)
{
    if (sequence != m_alternativeShortcut) {
        m_alternativeShortcut = sequence;
        Q_EMIT alternativeShortcutChanged();
    }
}

QKeySequence ShortcutHelper::defaultAlternativeShortcut()
{
    return DefaultAlternativeShortcut;
}

QKeySequence ShortcutHelper::lastUsedShortcut() const
{
    return m_lastUsedShortcut;
}

void ShortcutHelper::setLastUsedShortcut(const QKeySequence &sequence)
{
    if (sequence != m_lastUsedShortcut) {
        m_lastUsedShortcut = sequence;
        Q_EMIT lastUsedShortcutChanged();
    }
}

QKeySequence ShortcutHelper::defaultLastUsedShortcut()
{
    return DefaultLastUsedShortcut;
}

bool ShortcutHelper::isSaveNeeded()
{
    return getSequence(m_actionCollection->getToggleAction()) != m_alternativeShortcut
        || getSequence(m_actionCollection->getLastUsedLayoutAction()) != m_lastUsedShortcut;
}

bool ShortcutHelper::isDefaults()
{
    return m_alternativeShortcut == DefaultAlternativeShortcut || m_lastUsedShortcut == DefaultLastUsedShortcut;
}

KeyboardLayoutActionCollection *ShortcutHelper::actionColletion()
{
    return m_actionCollection;
}

void ShortcutHelper::defaults()
{
    m_actionCollection->setToggleShortcut(DefaultAlternativeShortcut);
    m_actionCollection->setLastUsedLayoutShortcut(DefaultLastUsedShortcut);

    setAlternativeShortcut(DefaultAlternativeShortcut);
    setLastUsedShortcut(DefaultLastUsedShortcut);
}

void ShortcutHelper::load()
{
    setAlternativeShortcut(getSequence(m_actionCollection->getToggleAction()));
    setLastUsedShortcut(getSequence(m_actionCollection->getLastUsedLayoutAction()));
}

void ShortcutHelper::save()
{
    m_actionCollection->setToggleShortcut(m_alternativeShortcut);
    m_actionCollection->setLastUsedLayoutShortcut(m_lastUsedShortcut);
}

QKeySequence ShortcutHelper::getSequence(QAction *action)
{
    const auto &shortcuts = KGlobalAccel::self()->shortcut(action);
    return shortcuts.isEmpty() ? QKeySequence() : shortcuts.first();
}

#include "moc_shortcuthelper.cpp"
#include "shortcuthelper.moc"
