/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QVariantList>

/**
 * Lists the workspace layout presets shown in the KCM grid: the built-in
 * presets shipped with the module followed by the user-saved ones. Each row
 * carries enough geometry (PanelsRole) for the QML delegate to draw a small
 * schematic preview of the layout.
 */
class PresetsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
        BuiltInRole,
        PanelsRole, // QVariantList of QVariantMap {edge, fill, thin, floating}
    };

    explicit PresetsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    /// Re-reads the user-saved presets from disk (built-ins are constant).
    void reload();

    /// Helpers for mapping between a grid row and a preset, used to drive the selection.
    Q_INVOKABLE int indexOfId(const QString &id) const;
    Q_INVOKABLE QString idAt(int row) const;
    Q_INVOKABLE bool builtInAt(int row) const;

    /// Record the screen the "Current Layout" preview is scoped to (used by the file-based fallback).
    Q_INVOKABLE void setCurrentLayoutScreen(const QString &screenName);

    /// Re-read the live layout file into the "Current Layout" preview. This is a best-effort
    /// fallback: the file can list orphaned panel containments the running shell no longer shows,
    /// so prefer setCurrentLayoutPanels() with live data queried from plasmashell.
    void refreshCurrentLayout();

    /// Replace the "Current Layout" preview with panels queried live from the running shell.
    void setCurrentLayoutPanels(const QVariantList &panels);

    /// Build a panel descriptor (the same shape stored in PanelsRole), for callers assembling a
    /// preview from live data.
    static QVariantMap panel(const QString &edge, bool fill, bool thin = false, bool floating = false);

    /// Parse the panels from a layout file. When screenName is given and the file carries a
    /// screen-connector mapping, only that screen's panels are returned, otherwise all of them.
    /// Best-effort: the file can list orphaned containments, so prefer live data when available.
    static QVariantList parsePanels(const QString &appletsrcPath, const QString &screenName = QString());

private:
    struct Preset {
        QString id;
        QString name;
        QString description;
        bool builtIn = false;
        QVariantList panels;
    };

    static QList<Preset> builtInPresets();
    void loadUserPresets();

    QList<Preset> m_presets;
    QString m_currentLayoutScreen; // screen for the transient "Current Layout" preview
};
