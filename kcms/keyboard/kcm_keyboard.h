/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>
    SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KQuickManagedConfigModule>

class KeyboardSettingsData;
class WorkspaceOptions;
class KeyboardMiscSettings;
class KeyboardSettings;
class KeyboardConfig;
class ShortcutHelper;
class LayoutModel;
class UserLayoutModel;
class XkbOptionsModel;

namespace NumLockState
{
Q_NAMESPACE

enum NumLockState {
    NumLockOn,
    NumLockOff,
    NumLockUnchanged,
};
Q_ENUM_NS(NumLockState)
};

class KCMKeyboard final : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(WorkspaceOptions *workspaceOptions READ workspaceOptions CONSTANT)
    Q_PROPERTY(KeyboardMiscSettings *miscSettings READ miscSettings CONSTANT)
    Q_PROPERTY(KeyboardSettings *keyboardSettings READ keyboardSettings CONSTANT)
    Q_PROPERTY(LayoutModel *layouts READ layouts CONSTANT)
    Q_PROPERTY(UserLayoutModel *userLayoutModel READ userLayoutModel CONSTANT)
    Q_PROPERTY(ShortcutHelper *shortcutHelper READ shortcutHelper CONSTANT)
    Q_PROPERTY(XkbOptionsModel *xkbOptionsModel READ xkbOptionsModel CONSTANT)

    Q_PROPERTY(int maxGroupCount READ maxGroupCount CONSTANT)

public:
    KCMKeyboard(QObject *parent, const KPluginMetaData &data);
    ~KCMKeyboard() override;

    WorkspaceOptions *workspaceOptions() const;
    KeyboardMiscSettings *miscSettings() const;
    KeyboardSettings *keyboardSettings() const;
    LayoutModel *layouts() const;
    UserLayoutModel *userLayoutModel() const;
    ShortcutHelper *shortcutHelper() const;
    XkbOptionsModel *xkbOptionsModel() const;

    int maxGroupCount() const;

    Q_INVOKABLE bool hasAccentSupport();
    Q_INVOKABLE void requestPreview(const QString &model, const QString &layout, const QString &variant, const QString &title = QString());

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;

private:
    bool isSaveNeeded() const override;
    bool isDefaults() const override;

private Q_SLOTS:
    void resetShortcuts();

private:
    KeyboardSettingsData *const m_data;
    KeyboardConfig *const m_config;

    LayoutModel *const m_layoutModel;
    UserLayoutModel *const m_userLayoutModel;
    ShortcutHelper *const m_shortcutHelper;
    XkbOptionsModel *const m_xkbOptionsModel;
};
