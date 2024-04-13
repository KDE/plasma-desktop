/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QKeySequence>
#include <QObject>

class QAction;
class KeyboardLayoutActionCollection;

class ShortcutHelper final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QKeySequence alternativeShortcut READ alternativeShortcut WRITE setAlternativeShortcut NOTIFY alternativeShortcutChanged)
    Q_PROPERTY(QKeySequence lastUsedShortcut READ lastUsedShortcut WRITE setLastUsedShortcut NOTIFY lastUsedShortcutChanged)

public:
    ShortcutHelper(QObject *parent);

    QKeySequence alternativeShortcut() const;
    void setAlternativeShortcut(const QKeySequence &sequence);
    Q_INVOKABLE QKeySequence defaultAlternativeShortcut();

    QKeySequence lastUsedShortcut() const;
    void setLastUsedShortcut(const QKeySequence &sequence);
    Q_INVOKABLE QKeySequence defaultLastUsedShortcut();

    bool isSaveNeeded();
    bool isDefaults();

    KeyboardLayoutActionCollection *actionColletion();

public Q_SLOTS:
    void defaults();
    void load();
    void save();

Q_SIGNALS:
    void alternativeShortcutChanged();
    void lastUsedShortcutChanged();

private:
    QKeySequence getSequence(QAction *action);

private:
    KeyboardLayoutActionCollection *const m_actionCollection;

    QKeySequence m_alternativeShortcut;
    QKeySequence m_lastUsedShortcut;
};
