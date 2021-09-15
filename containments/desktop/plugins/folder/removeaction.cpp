/*
    SPDX-FileCopyrightText: 2021 Derek Christ <christ.derek@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "removeaction.h"

#include <QGuiApplication>
#include <QKeyEvent>

RemoveAction::RemoveAction(KActionCollection *collection, QObject *parent)
    : QAction(parent)
    , m_collection(collection)
{
    connect(this, &RemoveAction::triggered, this, [=]() {
        if (m_action) {
            m_action->trigger();
        }
    });
}

void RemoveAction::update(ShiftState shiftState)
{
    if (!m_collection) {
        m_action = nullptr;
        return;
    }

    if (shiftState == ShiftState::Unknown) {
        shiftState = QGuiApplication::keyboardModifiers() & Qt::ShiftModifier ? ShiftState::Pressed : ShiftState::Released;
    }

    switch (shiftState) {
    case ShiftState::Pressed: {
        m_action = m_collection->action(QStringLiteral("del"));

        if (m_action) {
            // Make sure we show Shift+Del in the context menu.
            auto deleteShortcuts = m_action->shortcuts();
            deleteShortcuts.removeAll(Qt::SHIFT | Qt::Key_Delete);
            deleteShortcuts.prepend(Qt::SHIFT | Qt::Key_Delete);
            m_collection->setDefaultShortcuts(this, deleteShortcuts);
        }
        break;
    }
    case ShiftState::Released: {
        m_action = m_collection->action(QStringLiteral("trash"));

        if (m_action) {
            // Make sure we show Del in the context menu.
            auto trashShortcuts = m_action->shortcuts();
            trashShortcuts.removeAll(QKeySequence::Delete);
            trashShortcuts.prepend(QKeySequence::Delete);
            m_collection->setDefaultShortcuts(this, trashShortcuts);
        }
        break;
    }
    case ShiftState::Unknown:
        Q_UNREACHABLE();
        break;
    }

    if (m_action) {
        setText(m_action->text());
        setIcon(m_action->icon());
        setEnabled(m_action->isEnabled());
    }
}

const QAction *RemoveAction::proxyAction() const
{
    return m_action;
}

bool RemoveAction::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    // Catching Shift modifier usage on open context menus to swap the
    // Trash/Delete actions.
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Shift) {
            update(RemoveAction::ShiftState::Pressed);
        }
    } else if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Shift) {
            update(RemoveAction::ShiftState::Released);
        }
    }

    return false;
}
