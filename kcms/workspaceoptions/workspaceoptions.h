/*
    SPDX-FileCopyrightText: 2018 <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KQuickManagedConfigModule>

class WorkspaceOptionsData;
class WorkspaceOptionsGlobalsSettings;
class WorkspaceOptionsPlasmaSettings;
class WorkspaceOptionsKwinSettings;

class KCMWorkspaceOptions : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(WorkspaceOptionsGlobalsSettings *globalsSettings READ globalsSettings CONSTANT)
    Q_PROPERTY(WorkspaceOptionsPlasmaSettings *plasmaSettings READ plasmaSettings CONSTANT)
    Q_PROPERTY(WorkspaceOptionsKwinSettings *kwinSettings READ kwinSettings CONSTANT)

public:
    KCMWorkspaceOptions(QObject *parent, const KPluginMetaData &metaData);
    ~KCMWorkspaceOptions() override
    {
    }

    WorkspaceOptionsGlobalsSettings *globalsSettings() const;
    WorkspaceOptionsPlasmaSettings *plasmaSettings() const;
    WorkspaceOptionsKwinSettings *kwinSettings() const;

public Q_SLOTS:
    void save() override;
    void requestReboot();

Q_SIGNALS:
    void primarySelectionOptionSaved();

private:
    WorkspaceOptionsData *m_data;
};
