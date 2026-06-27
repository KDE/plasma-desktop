/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "presetsmodel.h"
#include "presetstorage.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QFile>

PresetsModel::PresetsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    reload();
}

QVariantMap PresetsModel::panel(const QString &edge, bool fill, bool thin, bool floating)
{
    return QVariantMap{
        {QStringLiteral("edge"), edge},
        {QStringLiteral("fill"), fill},
        {QStringLiteral("thin"), thin},
        {QStringLiteral("floating"), floating},
    };
}

QList<PresetsModel::Preset> PresetsModel::builtInPresets()
{
    return {
        {QStringLiteral("plasma"),
         i18n("KDE Plasma"),
         i18n("The default Plasma layout: a single Task Manager panel along the bottom edge."),
         true,
         {panel(QStringLiteral("bottom"), true)}},
        {QStringLiteral("plasma-left"),
         i18n("Plasma (Left)"),
         i18n("The default Plasma panel, placed on the left edge of the screen."),
         true,
         {panel(QStringLiteral("left"), true)}},
        {QStringLiteral("macos"),
         i18n("macOS"),
         i18n("A thin global menu bar at the top and a floating, centered dock at the bottom."),
         true,
         {panel(QStringLiteral("top"), true, true), panel(QStringLiteral("bottom"), false, false, true)}},
        {QStringLiteral("gnome"),
         i18n("GNOME 3"),
         i18n("A single top bar with an Activities-style launcher, a centered clock and a status area."),
         true,
         {panel(QStringLiteral("top"), true, true)}},
        {QStringLiteral("xfce"),
         i18n("Xfce"),
         i18n("A top panel with menu, window list and status area, plus a small launcher dock at the bottom."),
         true,
         {panel(QStringLiteral("top"), true, true), panel(QStringLiteral("bottom"), false, false, true)}},
        {QStringLiteral("unity"),
         i18n("Unity"),
         i18n("A vertical launcher dock on the left edge and a top bar with the global menu and status area."),
         true,
         {panel(QStringLiteral("left"), true), panel(QStringLiteral("top"), true, true)}},
    };
}

QVariantList PresetsModel::parsePanels(const QString &appletsrcPath, const QString &screenName)
{
    // Plasma::Types::Location: TopEdge=3, BottomEdge=4, LeftEdge=5, RightEdge=6.
    static const QHash<int, QString> edges = {{3, QStringLiteral("top")},
                                              {4, QStringLiteral("bottom")},
                                              {5, QStringLiteral("left")},
                                              {6, QStringLiteral("right")}};

    QVariantList result;
    KConfig config(appletsrcPath, KConfig::SimpleConfig);

    // If a screen is requested and the file records a connector->id mapping, keep only that
    // screen's panels; otherwise (no mapping, or no screen given) keep all of them.
    int targetScreen = -2; // -2 means "no screen filter"
    if (!screenName.isEmpty()) {
        const KConfigGroup connectors = config.group(QStringLiteral("ScreenConnectors"));
        const QStringList connectorIds = connectors.keyList();
        for (const QString &id : connectorIds) {
            if (connectors.readEntry(id, QString()) == screenName) {
                targetScreen = id.toInt();
                break;
            }
        }
    }

    KConfigGroup containments = config.group(QStringLiteral("Containments"));
    const QStringList ids = containments.groupList();
    for (const QString &id : ids) {
        const KConfigGroup containment = containments.group(id);
        if (containment.readEntry(QStringLiteral("plugin")) != QLatin1String("org.kde.panel")) {
            continue;
        }
        if (targetScreen != -2 && containment.readEntry(QStringLiteral("lastScreen"), -1) != targetScreen) {
            continue;
        }
        const auto edge = edges.constFind(containment.readEntry(QStringLiteral("location"), 0));
        if (edge != edges.constEnd()) {
            const bool vertical = (*edge == QLatin1String("left") || *edge == QLatin1String("right"));
            result.append(panel(*edge, /*fill*/ true, /*thin*/ !vertical));
        }
    }
    return result;
}

void PresetsModel::setCurrentLayoutScreen(const QString &screenName)
{
    m_currentLayoutScreen = screenName;
}

void PresetsModel::refreshCurrentLayout()
{
    const int row = indexOfId(PresetStorage::currentPresetId());
    if (row >= 0) {
        m_presets[row].panels = parsePanels(PresetStorage::liveConfigPath(), m_currentLayoutScreen);
        Q_EMIT dataChanged(index(row), index(row), {PanelsRole});
    }
}

void PresetsModel::setCurrentLayoutPanels(const QVariantList &panels)
{
    const int row = indexOfId(PresetStorage::currentPresetId());
    if (row >= 0) {
        m_presets[row].panels = panels;
        Q_EMIT dataChanged(index(row), index(row), {PanelsRole});
    }
}

void PresetsModel::loadUserPresets()
{
    const KSharedConfig::Ptr index = PresetStorage::index();
    const QStringList uuids = index->groupList();
    for (const QString &uuid : uuids) {
        const KConfigGroup group = index->group(uuid);
        // Skip the reserved groups ([CurrentPresets], [State], ...); only real presets have a Name.
        if (!group.hasKey(QStringLiteral("Name"))) {
            continue;
        }
        Preset preset;
        preset.id = uuid;
        preset.name = group.readEntry(QStringLiteral("Name"), uuid);
        preset.description = i18n("Custom layout saved on %1", group.readEntry(QStringLiteral("Created")));
        preset.builtIn = false;
        preset.panels = parsePanels(PresetStorage::snapshotPath(uuid));
        m_presets.append(preset);
    }
}

void PresetsModel::reload()
{
    beginResetModel();
    m_presets.clear();

    // Transient "Current Layout" entry: shows the layout currently loaded (per the current screen).
    // It lets the user return to their starting point without saving it first, and is discarded when
    // the module closes.
    const QString liveLayout = PresetStorage::liveConfigPath();
    if (QFile::exists(liveLayout)) {
        m_presets.append(Preset{
            PresetStorage::currentPresetId(),
            i18n("Previous Layout"),
            i18n("The layout that was in use when this page was opened. Apply it (or use Reset) to return to it."),
            true, // not user-editable (no rename/delete)
            parsePanels(liveLayout, m_currentLayoutScreen),
        });
    }

    m_presets.append(builtInPresets());
    loadUserPresets();
    endResetModel();
}

int PresetsModel::indexOfId(const QString &id) const
{
    for (int i = 0; i < m_presets.count(); ++i) {
        if (m_presets.at(i).id == id) {
            return i;
        }
    }
    return -1;
}

QString PresetsModel::idAt(int row) const
{
    return (row >= 0 && row < m_presets.count()) ? m_presets.at(row).id : QString();
}

bool PresetsModel::builtInAt(int row) const
{
    return (row >= 0 && row < m_presets.count()) && m_presets.at(row).builtIn;
}

int PresetsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_presets.count();
}

QVariant PresetsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_presets.count()) {
        return {};
    }
    const Preset &preset = m_presets.at(index.row());
    switch (role) {
    case IdRole:
        return preset.id;
    case NameRole:
        return preset.name;
    case DescriptionRole:
        return preset.description;
    case BuiltInRole:
        return preset.builtIn;
    case PanelsRole:
        return preset.panels;
    }
    return {};
}

QHash<int, QByteArray> PresetsModel::roleNames() const
{
    return {
        {IdRole, "presetId"},
        {NameRole, "name"},
        {DescriptionRole, "description"},
        {BuiltInRole, "builtIn"},
        {PanelsRole, "panels"},
    };
}

#include "moc_presetsmodel.cpp"
