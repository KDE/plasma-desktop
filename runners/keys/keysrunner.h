/*
    SPDX-FileCopyrightText: 2025 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KRunner/AbstractRunner>

class GlobalAccelModel;
class FilteredShortcutsModel;

/**
 * This runner looks for shortcuts defined in the set of .desktop files installed by
 * applications. It's useful for users to invoke shortcuts without having to know the
 * key bindings.
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
    void onShortcutsChanged(const QStringList &actionId, const QList<QKeySequence> &newKeys);

private:
    void onPrepare();

    GlobalAccelModel *m_globalAccelModel = nullptr;
    FilteredShortcutsModel *m_filteredModel = nullptr;
    bool m_needsUpdate = true;

    bool m_isKRunner = false;
    bool m_isPlasmashell = false;
};
