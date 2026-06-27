/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KSharedConfig>

#include <QStandardPaths>
#include <QString>

/**
 * Filesystem layout for user-saved presets:
 *   - index:     ~/.config/kcm_workspacepresetsrc, one group per preset UUID (Name, Created)
 *   - snapshots: <data>/kcm_workspacepresets/<uuid>.appletsrc, a copy of the live Plasma config
 */
namespace PresetStorage
{

// Reserved id for the transient "Current Layout" entry (the state captured at launch). It is shown
// in the grid while the module is open and discarded on close - it is never saved as a preset.
inline QString currentPresetId()
{
    return QStringLiteral("__current__");
}

inline QString baseDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/plasma/kcm_workspacepresets");
}

inline QString snapshotPath(const QString &uuid)
{
    return baseDir() + QLatin1Char('/') + uuid + QStringLiteral(".appletsrc");
}

// Panels-only serialized layout (Plasma's dumpCurrentLayoutJS JSON) captured when the preset was
// saved, so it can be replayed live through the scripting API instead of restarting plasmashell.
inline QString layoutScriptPath(const QString &uuid)
{
    return baseDir() + QLatin1Char('/') + uuid + QStringLiteral(".layout.json");
}

inline KSharedConfig::Ptr index()
{
    return KSharedConfig::openConfig(QStringLiteral("kcm_workspacepresetsrc"), KConfig::SimpleConfig);
}

inline QString liveConfigPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/plasma-org.kde.plasma.desktop-appletsrc");
}

inline QString liveShortcutsPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/kglobalshortcutsrc");
}

inline QString shortcutsSnapshotPath(const QString &uuid)
{
    return baseDir() + QLatin1Char('/') + uuid + QStringLiteral(".kglobalshortcutsrc");
}

// The layout/shortcuts in effect just before the last preset was applied, so it can be reverted.
inline QString previousLayoutPath()
{
    return baseDir() + QStringLiteral("/previous.appletsrc");
}

// Serialized (panels-only) form of the on-load layout, so Reset can replay it live without
// restarting plasmashell. Refreshed each time the module opens.
inline QString previousLayoutScriptPath()
{
    return baseDir() + QStringLiteral("/previous.layout.json");
}

inline QString previousShortcutsPath()
{
    return baseDir() + QStringLiteral("/previous.kglobalshortcutsrc");
}

}
