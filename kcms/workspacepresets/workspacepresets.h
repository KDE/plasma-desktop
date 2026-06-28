/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KQuickConfigModule>

#include <QHash>
#include <QVariantList>

class PresetsModel;

/**
 * KCM to switch the workspace between preset panel layouts. Presets are selected in a grid and
 * committed with the Apply button (which targets the screen the module is shown on). Built-in
 * presets are applied live through plasmashell's scripting interface; user presets are snapshots
 * of the live configuration restored by replacing the config and restarting plasmashell.
 */
class WorkspacePresets : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(PresetsModel *presets READ presets CONSTANT)
    Q_PROPERTY(bool applyShortcuts READ applyShortcuts WRITE setApplyShortcuts NOTIFY applyShortcutsChanged)

public:
    WorkspacePresets(QObject *parent, const KPluginMetaData &data);

    PresetsModel *presets() const;

    /// Whether Apply should also set the preset's matching global shortcuts.
    bool applyShortcuts() const;
    void setApplyShortcuts(bool apply);

    /// Id of the preset applied most recently to the given screen (empty if unknown). Per screen so
    /// the selection can follow the screen the module is shown on.
    Q_INVOKABLE QString currentPresetForScreen(const QString &screenName) const;

    /// Record the pending selection (the grid's current item) and update the modified state so the
    /// Apply button is enabled only when the selection differs from what is applied on that screen.
    Q_INVOKABLE void setSelection(const QString &id, const QString &screenName);

    /// Set the screen the module is shown on and refresh the "Current Layout" preview to the panels
    /// actually shown there by the running shell. Called on show and whenever the window changes screen.
    Q_INVOKABLE void setCurrentLayoutScreen(const QString &screenName);

    /// Save the live layout as a new, named user preset. Returns its id.
    Q_INVOKABLE QString saveCurrentLayout(const QString &name);

    /// Rename a user preset.
    Q_INVOKABLE void renamePreset(const QString &id, const QString &newName);

    /// Delete a user preset.
    Q_INVOKABLE void deletePreset(const QString &id);

    void defaults() override; // select the Plasma preset
    void load() override; // reinitialise: restore the layout that was in effect at launch
    void save() override; // apply the selected preset to the selected screen

Q_SIGNALS:
    void layoutSaved(const QString &id);
    void errorOccurred(const QString &message);
    void applyShortcutsChanged();
    /// Ask the view to reselect the preset applied on the current screen (Reset/initial load).
    void reinitialise();
    /// Ask the view to select a specific preset (Defaults).
    void selectPreset(const QString &id);

private:
    void applyPreset(const QString &id, bool builtIn, bool withShortcuts, const QString &screenName);
    void applyBuiltIn(const QString &id, const QString &screenName);
    void applyUserPreset(const QString &id, const QString &screenName);
    // Replay a preset's saved panels live on the given screen via the scripting API (no restart).
    // Returns false if the preset has no serialized layout, so the caller can fall back to a restart.
    bool applyUserPresetLive(const QString &id, const QString &screenName);
    // Recreate the panels from a serialized (dumpCurrentLayoutJS) layout live: clears the panels on
    // the given screen (or all screens if empty) and recreates the saved ones. Returns false if the
    // layout is empty or the running shell could not be reached.
    bool replaySerializedLayout(const QByteArray &layoutJson, const QString &screenName);
    // Capture the live panels as a serialized layout (Plasma's dumpCurrentLayoutJS) to destPath for
    // later live replay; best-effort, returns false if the running shell could not be queried.
    static bool captureLayoutScript(const QString &destPath);
    void applyBuiltInShortcuts(const QString &id);
    void restoreUserShortcuts(const QString &id);
    void snapshotLaunchState();
    // Slow, DBus-heavy part of capturing the on-load state (serialized layout + per-screen panel
    // previews); deferred off the constructor so it does not block the settings window appearing.
    void captureLaunchState();
    void restoreLaunchState();
    void setCurrentPreset(const QString &id, const QString &screenName);
    static void reloadShortcuts();
    // Cross-fade the upcoming plasmashell restart using KWin's BlendChanges effect (the same one
    // that smooths dark/light mode switches), so the shell reload fades instead of flickering.
    static void blendDesktopRestart();
    // Ask the running shell which panels it shows on the given screen (accurate, unlike the config
    // file which may list orphaned containments); falls back to the launch snapshot file on failure.
    QVariantList queryLivePanels(const QString &screenName) const;

    PresetsModel *const m_presets;
    QString m_selectedId;
    QString m_selectedScreen;
    QString m_currentLayoutScreen; // screen the module is shown on (for the Previous Layout preview)
    QHash<QString, QVariantList> m_launchPanels; // per-screen panels captured when the module opened
    bool m_selectedBuiltIn = true;
    bool m_applyShortcuts = false;
    bool m_initialLoadDone = false; // first load() is the framework's; later ones are Reset
    bool m_appliedDiffersFromLaunch = false; // a preset was applied since the module opened
    QHash<QString, QString> m_launchPresets; // per-screen preset applied when the module opened
};
