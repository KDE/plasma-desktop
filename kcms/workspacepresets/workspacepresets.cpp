/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "workspacepresets.h"
#include "presetsmodel.h"
#include "presetstorage.h"
#include "shortcutpresets.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QScreen>
#include <QUuid>

K_PLUGIN_CLASS_WITH_JSON(WorkspacePresets, "kcm_workspacepresets.json")

WorkspacePresets::WorkspacePresets(QObject *parent, const KPluginMetaData &data)
    : KQuickConfigModule(parent, data)
    , m_presets(new PresetsModel(this))
{
    m_applyShortcuts = PresetStorage::index()->group(QStringLiteral("State")).readEntry("ApplyShortcuts", false);
    // Remember the layout in effect when the module opens, so it can be restored, then refresh the
    // model so the transient "Previous Layout" entry (backed by that snapshot) appears. The fast
    // file snapshot runs now; the slow DBus capture is deferred so it does not stall module open.
    snapshotLaunchState();
    m_presets->reload();
    QMetaObject::invokeMethod(this, &WorkspacePresets::captureLaunchState, Qt::QueuedConnection);
}

PresetsModel *WorkspacePresets::presets() const
{
    return m_presets;
}

QString WorkspacePresets::currentPresetForScreen(const QString &screenName) const
{
    return PresetStorage::index()->group(QStringLiteral("CurrentPresets")).readEntry(screenName, QString());
}

void WorkspacePresets::setCurrentPreset(const QString &id, const QString &screenName)
{
    KConfigGroup group = PresetStorage::index()->group(QStringLiteral("CurrentPresets"));
    if (screenName.isEmpty()) {
        // Whole-config change (Defaults / all screens): record it for every connected screen.
        const auto screens = QGuiApplication::screens();
        for (const QScreen *screen : screens) {
            group.writeEntry(screen->name(), id);
        }
    } else {
        group.writeEntry(screenName, id);
    }
    group.sync();
}

bool WorkspacePresets::applyShortcuts() const
{
    return m_applyShortcuts;
}

void WorkspacePresets::setApplyShortcuts(bool apply)
{
    if (m_applyShortcuts == apply) {
        return;
    }
    m_applyShortcuts = apply;
    KConfigGroup state = PresetStorage::index()->group(QStringLiteral("State"));
    state.writeEntry("ApplyShortcuts", apply);
    state.sync();
    Q_EMIT applyShortcutsChanged();
}

void WorkspacePresets::setSelection(const QString &id, const QString &screenName)
{
    m_selectedId = id;
    m_selectedBuiltIn = m_presets->builtInAt(m_presets->indexOfId(id));
    m_selectedScreen = screenName;
    // Modified when the selection differs from what is currently applied on this screen, or a
    // preset has already been applied this session (so Reset stays available to return to the
    // on-load layout no matter which screen is shown).
    setNeedsSave(id != currentPresetForScreen(screenName) || m_appliedDiffersFromLaunch);
}

void WorkspacePresets::setCurrentLayoutScreen(const QString &screenName)
{
    m_currentLayoutScreen = screenName;
    m_presets->setCurrentLayoutScreen(screenName);
    // The "Previous Layout" preview is the as-opened state, captured per screen when the module
    // opened, so it stays available to return to no matter which preset is applied afterwards.
    const QVariantList panels = m_launchPanels.contains(screenName) ? m_launchPanels.value(screenName) : queryLivePanels(screenName);
    m_presets->setCurrentLayoutPanels(panels);
}

QVariantList WorkspacePresets::queryLivePanels(const QString &screenName) const
{
    // Ask the running shell which panels it actually shows on this screen. Unlike the on-disk
    // config, panels() only returns instantiated panels, so orphaned containments left behind by
    // earlier layout changes do not leak into the preview.
    QString connector = screenName;
    connector.replace(QLatin1Char('\\'), QStringLiteral("\\\\")).replace(QLatin1Char('"'), QStringLiteral("\\\""));
    const QString script = QStringLiteral(
                               "var targetConnector = \"%1\";\n"
                               "var target = targetConnector.length ? screenForConnector(targetConnector) : -1;\n"
                               "var out = [];\n"
                               "panels().forEach(function(panel) {\n"
                               "    if (target >= 0 && panel.screen !== target) {\n"
                               "        return;\n"
                               "    }\n"
                               "    var vertical = (panel.location === \"left\" || panel.location === \"right\");\n"
                               "    var floating = panel.floating ? 1 : 0;\n"
                               "    var fill = (panel.lengthMode === \"fill\") ? 1 : 0;\n"
                               "    var thin = (!vertical && panel.height <= gridUnit * 2) ? 1 : 0;\n"
                               "    out.push(panel.location + \"|\" + floating + \"|\" + fill + \"|\" + thin);\n"
                               "});\n"
                               "print(out.join(\";\"));\n")
                               .arg(connector);

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                          QStringLiteral("/PlasmaShell"),
                                                          QStringLiteral("org.kde.PlasmaShell"),
                                                          QStringLiteral("evaluateScript"));
    message.setArguments({script});

    const QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        // The shell may not be reachable (e.g. running standalone in kcmshell); fall back to the
        // launch snapshot file (the layout as it was when the module opened).
        return PresetsModel::parsePanels(PresetStorage::previousLayoutPath(), screenName);
    }

    static const QStringList edges = {QStringLiteral("top"), QStringLiteral("bottom"), QStringLiteral("left"), QStringLiteral("right")};
    QVariantList panels;
    const QString output = reply.arguments().value(0).toString().trimmed();
    const QStringList items = output.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    for (const QString &item : items) {
        const QStringList parts = item.split(QLatin1Char('|'));
        if (parts.size() < 4 || !edges.contains(parts.at(0))) {
            continue;
        }
        panels.append(
            PresetsModel::panel(parts.at(0), parts.at(2) == QLatin1String("1"), parts.at(3) == QLatin1String("1"), parts.at(1) == QLatin1String("1")));
    }
    return panels;
}

void WorkspacePresets::applyPreset(const QString &id, bool builtIn, bool withShortcuts, const QString &screenName)
{
    if (builtIn) {
        applyBuiltIn(id, screenName);
        if (withShortcuts) {
            applyBuiltInShortcuts(id);
        }
    } else {
        if (withShortcuts) {
            restoreUserShortcuts(id);
        }
        applyUserPreset(id, screenName);
    }

    setCurrentPreset(id, screenName);
}

void WorkspacePresets::applyBuiltIn(const QString &id, const QString &screenName)
{
    QFile script(QStringLiteral(":/layouts/%1.js").arg(id));
    if (!script.open(QIODevice::ReadOnly)) {
        Q_EMIT errorOccurred(i18n("Unknown layout preset \"%1\".", id));
        return;
    }

    const QString scriptText = QString::fromUtf8(script.readAll());

    // Scope the layout to the screen the module is shown on: the layout scripts only use
    // clearPanels()/newPanel() from this preamble, which target that screen (and leave panels on
    // other screens untouched). An empty screen name falls back to all screens.
    QString connector = screenName;
    connector.replace(QLatin1Char('\\'), QStringLiteral("\\\\")).replace(QLatin1Char('"'), QStringLiteral("\\\""));
    const QString preamble = QStringLiteral(
                                 "var targetConnector = \"%1\";\n"
                                 "var target = targetConnector.length ? screenForConnector(targetConnector) : -1;\n"
                                 "function clearPanels() {\n"
                                 "    panels().forEach(function(panel) {\n"
                                 "        if (target < 0 || panel.screen === target) {\n"
                                 "            panel.remove();\n"
                                 "        }\n"
                                 "    });\n"
                                 "}\n"
                                 "function newPanel() {\n"
                                 "    var panel = new Panel;\n"
                                 "    if (target >= 0) {\n"
                                 "        panel.screen = target;\n"
                                 "    }\n"
                                 "    return panel;\n"
                                 "}\n")
                                 .arg(connector);

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                          QStringLiteral("/PlasmaShell"),
                                                          QStringLiteral("org.kde.PlasmaShell"),
                                                          QStringLiteral("evaluateScript"));
    const QString fullScript = preamble + scriptText;
    message.setArguments({fullScript});

    const QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        Q_EMIT errorOccurred(i18n("Could not apply the layout: %1", reply.errorMessage()));
        return;
    }

    // When the layout uses the Global Menu applet, ensure the kded appmenu module (the menu
    // registrar) is loaded so applications launched afterwards export their menus to it.
    // Already-running applications still need to be restarted to populate the global menu.
    if (scriptText.contains(QLatin1String("org.kde.plasma.appmenu"))) {
        QDBusMessage loadModule = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded6"),
                                                                 QStringLiteral("/kded"),
                                                                 QStringLiteral("org.kde.kded6"),
                                                                 QStringLiteral("loadModule"));
        loadModule.setArguments({QStringLiteral("appmenu")});
        QDBusConnection::sessionBus().call(loadModule, QDBus::NoBlock);
    }
}

void WorkspacePresets::blendDesktopRestart()
{
    // Reuse KWin's BlendChanges effect (the smooth cross-fade behind dark/light mode switches): it
    // snapshots the screen now and cross-fades to the new content after the delay, hiding the
    // abrupt plasmashell restart. The call is blocking so KWin captures the snapshot before the
    // shell is replaced. A shell restart takes longer to repaint than a colour-scheme change (which
    // uses 300 ms), so hold the snapshot a little longer.
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                          QStringLiteral("/org/kde/KWin/BlendChanges"),
                                                          QStringLiteral("org.kde.KWin.BlendChanges"),
                                                          QStringLiteral("start"));
    message.setArguments({1000});
    QDBusConnection::sessionBus().call(message);
}

void WorkspacePresets::applyUserPreset(const QString &id, const QString &screenName)
{
    // Preferred path: replay the saved panels live through the scripting interface (no restart),
    // exactly like the built-in presets.
    if (applyUserPresetLive(id, screenName)) {
        return;
    }

    // Fallback for presets saved before live replay existed (no serialized layout on disk): replace
    // the live configuration with the snapshot and restart plasmashell so it re-reads the layout.
    const QString snapshot = PresetStorage::snapshotPath(id);
    if (!QFile::exists(snapshot)) {
        Q_EMIT errorOccurred(i18n("The saved layout no longer exists."));
        return;
    }

    const QString live = PresetStorage::liveConfigPath();
    QFile::remove(live);
    if (!QFile::copy(snapshot, live)) {
        Q_EMIT errorOccurred(i18n("Could not restore the saved layout."));
        return;
    }

    blendDesktopRestart();
    if (!QProcess::startDetached(QStringLiteral("plasmashell"), {QStringLiteral("--replace")})) {
        Q_EMIT errorOccurred(i18n("The layout was restored but plasmashell could not be restarted. Log out and back in to apply it."));
        return;
    }
}

bool WorkspacePresets::applyUserPresetLive(const QString &id, const QString &screenName)
{
    QFile file(PresetStorage::layoutScriptPath(id));
    if (!file.open(QIODevice::ReadOnly)) {
        return false; // no serialized layout: caller falls back to a restart
    }
    const QByteArray layoutJson = file.readAll();
    return replaySerializedLayout(layoutJson, screenName);
}

bool WorkspacePresets::replaySerializedLayout(const QByteArray &layoutJson, const QString &screenName)
{
    if (layoutJson.isEmpty()) {
        return false;
    }

    QString connector = screenName;
    connector.replace(QLatin1Char('\\'), QStringLiteral("\\\\")).replace(QLatin1Char('"'), QStringLiteral("\\\""));

    // Clear the target screen's panels (all screens if no connector is given), then recreate the
    // saved panels (with their applets and configuration). The config-group handling mirrors
    // plasmashell's own loadSerializedLayout(); the serialized form is panels-only, so desktops are
    // left untouched.
    QString script = QStringLiteral(R"JS(
var targetConnector = "__TARGET_CONNECTOR__";
var target = targetConnector.length ? screenForConnector(targetConnector) : -1;
panels().forEach(function(p) {
    if (target < 0 || p.screen === target) {
        p.remove();
    }
});
var layout = __LAYOUT_JSON__;
function loadConfigs(obj, config) {
    if (!config) {
        return;
    }
    for (var escapedGroup in config) {
        var groups = escapedGroup.split("/").filter(function(s) { return s.length > 0; }).map(decodeURIComponent);
        obj.currentConfigGroup = groups;
        var entries = config[escapedGroup];
        for (var key in entries) {
            obj.writeConfig(key, entries[key]);
        }
    }
}
// The on-load screen each panel lived on, kept in its root config group by dumpCurrentLayoutJS.
function savedScreen(pd) {
    if (pd.config && pd.config["/"] && pd.config["/"]["lastScreen"] !== undefined) {
        return parseInt(pd.config["/"]["lastScreen"], 10);
    }
    return -1;
}
(layout.panels || []).forEach(function(pd) {
    var panel = new Panel;
    panel.location = pd.location;
    if (pd.height !== undefined) {
        panel.height = pd.height * gridUnit;
    }
    if (pd.lengthMode !== undefined) {
        panel.lengthMode = pd.lengthMode;
    }
    if (pd.maximumLength !== undefined) {
        panel.maximumLength = pd.maximumLength * gridUnit;
    }
    if (pd.minimumLength !== undefined) {
        panel.minimumLength = pd.minimumLength * gridUnit;
    }
    if (pd.offset !== undefined) {
        panel.offset = pd.offset * gridUnit;
    }
    if (pd.alignment !== undefined) {
        panel.alignment = pd.alignment;
    }
    if (pd.hiding !== undefined) {
        panel.hiding = pd.hiding;
    }
    if (pd.opacity !== undefined) {
        panel.opacity = pd.opacity;
    }
    loadConfigs(panel, pd.config);
    (pd.applets || []).forEach(function(ad) {
        var widget = panel.addWidget(ad.plugin);
        loadConfigs(widget, ad.config);
    });
    // Place the panel: a specific target screen when applying to one screen, otherwise each panel
    // returns to the screen it was on. Done last so it wins over any lastScreen in the config.
    var scr = (target >= 0) ? target : savedScreen(pd);
    if (scr >= 0) {
        panel.screen = scr;
    }
});
)JS");
    script.replace(QStringLiteral("__TARGET_CONNECTOR__"), connector);
    script.replace(QStringLiteral("__LAYOUT_JSON__"), QString::fromUtf8(layoutJson));

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                          QStringLiteral("/PlasmaShell"),
                                                          QStringLiteral("org.kde.PlasmaShell"),
                                                          QStringLiteral("evaluateScript"));
    message.setArguments({script});

    const QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    // On failure return false so the caller can fall back to the file-swap + restart path.
    return reply.type() != QDBusMessage::ErrorMessage;
}

bool WorkspacePresets::captureLayoutScript(const QString &destPath)
{
    // Ask plasmashell to serialize the running layout, then keep only the panels so re-applying the
    // preset cannot duplicate desktop widgets.
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                          QStringLiteral("/PlasmaShell"),
                                                          QStringLiteral("org.kde.PlasmaShell"),
                                                          QStringLiteral("dumpCurrentLayoutJS"));
    const QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        return false;
    }

    // The reply is a JS script: 'var plasma = ...; var layout = {...}; plasma.loadSerializedLayout(layout);'.
    // Extract the JSON object assigned to 'layout'.
    const QByteArray dump = reply.arguments().value(0).toByteArray();
    const QByteArray startMarker = "var layout = ";
    const int start = dump.indexOf(startMarker);
    const int end = dump.indexOf(";\n\nplasma.loadSerializedLayout");
    if (start < 0 || end < 0 || end <= start) {
        return false;
    }
    const QByteArray jsonBytes = dump.mid(start + startMarker.size(), end - (start + startMarker.size()));

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }
    QJsonObject layout = doc.object();
    layout.remove(QStringLiteral("desktops"));

    QFile file(destPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(layout).toJson(QJsonDocument::Compact));
    return true;
}

void WorkspacePresets::applyBuiltInShortcuts(const QString &id)
{
    const ShortcutPreset preset = shortcutsFor(id);

    KConfig config(PresetStorage::liveShortcutsPath(), KConfig::SimpleConfig);
    KConfigGroup kwin = config.group(QStringLiteral("kwin"));
    for (auto it = preset.kwin.constBegin(); it != preset.kwin.constEnd(); ++it) {
        // The entry format is "active,default,FriendlyName"; replace only the active field so the
        // upstream default binding and the display name are preserved. When there is no existing
        // entry the upstream default is unknown, so leave that field empty rather than recording the
        // preset binding as if it were the default (which would mislead "Reset to defaults").
        const QStringList parts = kwin.readEntry(it.key(), QString()).split(QLatin1Char(','));
        const QString defaultKey = parts.size() > 1 ? parts.at(1) : QString();
        const QString friendly = parts.size() > 2 ? parts.mid(2).join(QLatin1Char(',')) : it.key();
        kwin.writeEntry(it.key(), QStringList{it.value(), defaultKey, friendly}.join(QLatin1Char(',')));
    }
    config.sync();

    if (preset.metaKey != ShortcutPreset::Unchanged) {
        KConfig kwinrc(QStringLiteral("kwinrc"));
        KConfigGroup modifiers = kwinrc.group(QStringLiteral("ModifierOnlyShortcuts"));
        if (preset.metaKey == ShortcutPreset::Launcher) {
            modifiers.writeEntry(QStringLiteral("Meta"), QStringLiteral("org.kde.plasmashell,/PlasmaShell,org.kde.PlasmaShell,activateLauncherMenu"));
        } else {
            modifiers.writeEntry(QStringLiteral("Meta"),
                                 QStringLiteral("org.kde.kglobalaccel,/component/kwin,org.kde.kglobalaccel.Component,invokeShortcut,Overview"));
        }
        kwinrc.sync();
    }

    reloadShortcuts();
}

void WorkspacePresets::restoreUserShortcuts(const QString &id)
{
    const QString snapshot = PresetStorage::shortcutsSnapshotPath(id);
    if (!QFile::exists(snapshot)) {
        return; // the preset was saved without a shortcuts snapshot
    }
    const QString live = PresetStorage::liveShortcutsPath();
    QFile::remove(live);
    if (!QFile::copy(snapshot, live)) {
        Q_EMIT errorOccurred(i18n("Could not restore the saved keyboard shortcuts."));
        return;
    }
    reloadShortcuts();
}

void WorkspacePresets::snapshotLaunchState()
{
    if (!QDir().mkpath(PresetStorage::baseDir())) {
        return;
    }
    QFile::remove(PresetStorage::previousLayoutPath());
    QFile::copy(PresetStorage::liveConfigPath(), PresetStorage::previousLayoutPath());
    QFile::remove(PresetStorage::previousShortcutsPath());
    QFile::copy(PresetStorage::liveShortcutsPath(), PresetStorage::previousShortcutsPath());

    // The launch layout is represented by the transient "Previous Layout" entry on every screen.
    m_launchPresets.clear();
    m_launchPanels.clear();
    const auto screens = QGuiApplication::screens();
    for (const QScreen *screen : screens) {
        m_launchPresets.insert(screen->name(), PresetStorage::currentPresetId());
        setCurrentPreset(PresetStorage::currentPresetId(), screen->name());
    }
}

void WorkspacePresets::captureLaunchState()
{
    // The DBus round-trips below (one layout dump plus one panel query per screen) are too slow to
    // run from the constructor without stalling the settings window, so they are deferred until
    // after the module is shown. They only need to complete before the first Apply/Reset.
    captureLayoutScript(PresetStorage::previousLayoutScriptPath());

    // Capture the panels actually shown on each screen now, so the "Previous Layout" preview stays
    // fixed on the as-opened state even after presets are applied.
    const auto screens = QGuiApplication::screens();
    for (const QScreen *screen : screens) {
        m_launchPanels.insert(screen->name(), queryLivePanels(screen->name()));
    }
}

void WorkspacePresets::restoreLaunchState()
{
    // Restore the launch shortcuts first (no restart needed) for both code paths below.
    const QString shortcuts = PresetStorage::previousShortcutsPath();
    if (QFile::exists(shortcuts)) {
        const QString liveShortcuts = PresetStorage::liveShortcutsPath();
        QFile::remove(liveShortcuts);
        if (QFile::copy(shortcuts, liveShortcuts)) {
            reloadShortcuts();
        } else {
            Q_EMIT errorOccurred(i18n("Could not restore the keyboard shortcuts."));
        }
    }

    // Preferred path: replay the on-load panels live across all screens (no plasmashell restart).
    QFile scriptFile(PresetStorage::previousLayoutScriptPath());
    if (scriptFile.open(QIODevice::ReadOnly) && replaySerializedLayout(scriptFile.readAll(), QString())) {
        for (auto it = m_launchPresets.constBegin(); it != m_launchPresets.constEnd(); ++it) {
            setCurrentPreset(it.value(), it.key());
        }
        m_appliedDiffersFromLaunch = false;
        return;
    }

    // Fallback: rewrite the config from the on-load snapshot and restart plasmashell.
    const QString layout = PresetStorage::previousLayoutPath();
    if (!QFile::exists(layout)) {
        return;
    }
    const QString live = PresetStorage::liveConfigPath();
    QFile::remove(live);
    if (!QFile::copy(layout, live)) {
        Q_EMIT errorOccurred(i18n("Could not restore the layout."));
        return;
    }
    for (auto it = m_launchPresets.constBegin(); it != m_launchPresets.constEnd(); ++it) {
        setCurrentPreset(it.value(), it.key());
    }
    blendDesktopRestart();
    QProcess::startDetached(QStringLiteral("plasmashell"), {QStringLiteral("--replace")});
    m_appliedDiffersFromLaunch = false;
}

void WorkspacePresets::defaults()
{
    // Select the Plasma preset; the user commits it with Apply.
    Q_EMIT selectPreset(QStringLiteral("plasma"));
}

void WorkspacePresets::load()
{
    // Reinitialise. The framework's initial call only re-selects. Later calls come from the Reset
    // button: restore the launch layout only if a preset was actually applied since the module
    // opened; otherwise nothing changed on the desktop and resetting the selection is enough (no
    // need to rewrite the config or restart plasmashell).
    if (m_initialLoadDone && m_appliedDiffersFromLaunch) {
        restoreLaunchState();
    }
    m_initialLoadDone = true;
    setNeedsSave(false);
    Q_EMIT reinitialise();
}

void WorkspacePresets::save()
{
    if (m_selectedId.isEmpty()) {
        return;
    }
    if (m_selectedId == PresetStorage::currentPresetId()) {
        restoreLaunchState(); // the "Previous Layout" entry restores the launch snapshot (clears the flag)
    } else {
        applyPreset(m_selectedId, m_selectedBuiltIn, m_applyShortcuts, m_selectedScreen);
        m_appliedDiffersFromLaunch = true;
    }
    // The "Previous Layout" preview deliberately stays on the as-opened state so it remains a way
    // back; it is not refreshed to the just-applied layout.
    // Stay "modified" while the live layout differs from the on-load state, so Reset stays active
    // and restores it. The host clears needsSave right after a successful Apply, so re-assert it on
    // a queued call (after that reset) to keep Reset usable while the desktop differs from on-load.
    setNeedsSave(m_appliedDiffersFromLaunch);
    if (m_appliedDiffersFromLaunch) {
        QMetaObject::invokeMethod(
            this,
            [this]() {
                setNeedsSave(true);
            },
            Qt::QueuedConnection);
    }
}

void WorkspacePresets::reloadShortcuts()
{
    // kglobalacceld re-reads kglobalshortcutsrc when restarted; on a systemd-managed session this
    // is the supported way. If it is not systemd-managed the call is a no-op and the new shortcuts
    // take effect on the next login.
    QProcess::startDetached(QStringLiteral("systemctl"),
                            {QStringLiteral("--user"), QStringLiteral("try-restart"), QStringLiteral("plasma-kglobalacceld.service")});
    // Make KWin re-read kwinrc for the Meta-only shortcut.
    QDBusConnection::sessionBus().call(
        QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"), QStringLiteral("/KWin"), QStringLiteral("org.kde.KWin"), QStringLiteral("reconfigure")),
        QDBus::NoBlock);
}

QString WorkspacePresets::saveCurrentLayout(const QString &name)
{
    if (!QDir().mkpath(PresetStorage::baseDir())) {
        Q_EMIT errorOccurred(i18n("Could not create the presets directory."));
        return {};
    }

    const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    if (!QFile::copy(PresetStorage::liveConfigPath(), PresetStorage::snapshotPath(uuid))) {
        Q_EMIT errorOccurred(i18n("Could not read the current layout."));
        return {};
    }
    // Snapshot the current global shortcuts too, so applying this preset can optionally restore
    // them. Best-effort: the file may not exist if no shortcuts have ever been customized.
    QFile::copy(PresetStorage::liveShortcutsPath(), PresetStorage::shortcutsSnapshotPath(uuid));

    // Capture the layout in serialized form so it can be re-applied live (no plasmashell restart).
    // Best-effort: if the running shell cannot be queried, applying falls back to the snapshot.
    captureLayoutScript(PresetStorage::layoutScriptPath(uuid));

    KSharedConfig::Ptr index = PresetStorage::index();
    KConfigGroup group = index->group(uuid);
    group.writeEntry("Name", name);
    group.writeEntry("Created", QDateTime::currentDateTime().toString(Qt::ISODate));
    index->sync();

    m_presets->reload();
    Q_EMIT layoutSaved(uuid);
    return uuid;
}

void WorkspacePresets::renamePreset(const QString &id, const QString &newName)
{
    KSharedConfig::Ptr index = PresetStorage::index();
    if (!index->hasGroup(id)) {
        return;
    }
    KConfigGroup group = index->group(id);
    group.writeEntry("Name", newName);
    index->sync();
    m_presets->reload();
}

void WorkspacePresets::deletePreset(const QString &id)
{
    KSharedConfig::Ptr index = PresetStorage::index();
    if (index->hasGroup(id)) {
        index->deleteGroup(id);
        index->sync();
    }
    QFile::remove(PresetStorage::snapshotPath(id));
    QFile::remove(PresetStorage::shortcutsSnapshotPath(id));
    QFile::remove(PresetStorage::layoutScriptPath(id));
    m_presets->reload();
}

#include "workspacepresets.moc"

#include "moc_workspacepresets.cpp"
