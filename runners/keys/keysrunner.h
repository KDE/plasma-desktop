/*
    SPDX-FileCopyrightText: 2025 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KRunner/AbstractRunner>

class GlobalAccelModel;
class FilteredShortcutsModel;

/**
 * This class looks for matches in the set of .desktop files installed by
 * applications. This way the user can type exactly what they see in the
 * applications menu and have it start the appropriate app. Essentially anything
 * that KService knows about, this runner can launch
 */

class KeysRunner : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    KeysRunner(QObject *parent, const KPluginMetaData &metaData);

    void init() override;
    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;

private Q_SLOTS:
    void onShortcutsChanged(const QStringList &actionId, const QList<int> &newKeys);

private:
    void onPrepare();

    GlobalAccelModel *m_globalAccelModel = nullptr;
    FilteredShortcutsModel *m_filteredModel = nullptr;
    bool m_needsUpdate = true;
};
