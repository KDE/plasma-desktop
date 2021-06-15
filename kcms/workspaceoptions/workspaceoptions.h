/*
    SPDX-FileCopyrightText: 2018 <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _KCM_WORKSPACEOPTIONS_H
#define _KCM_WORKSPACEOPTIONS_H

#include <KQuickAddons/ManagedConfigModule>

class WorkspaceOptionsData;
class WorkspaceOptionsGlobalsSettings;
class WorkspaceOptionsPlasmaSettings;

class KCMWorkspaceOptions : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(WorkspaceOptionsGlobalsSettings *globalsSettings READ globalsSettings CONSTANT)
    Q_PROPERTY(WorkspaceOptionsPlasmaSettings *plasmaSettings READ plasmaSettings CONSTANT)

public:
    KCMWorkspaceOptions(QObject *parent, const QVariantList &args);
    ~KCMWorkspaceOptions() override
    {
    }

    WorkspaceOptionsGlobalsSettings *globalsSettings() const;
    WorkspaceOptionsPlasmaSettings *plasmaSettings() const;

public Q_SLOTS:
    void save() override;

private:
    WorkspaceOptionsData *m_data;
};

#endif // _KCM_WORKSPACEOPTIONS_H
