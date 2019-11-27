/*
 *  Copyright (C) 2018 <furkantokac34@gmail.com>
 *  Copyright (c) 2019 Cyril Rossi <cyril.rossi@enioka.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _KCM_WORKSPACEOPTIONS_H
#define _KCM_WORKSPACEOPTIONS_H

#include <KQuickAddons/ManagedConfigModule>

class WorkspaceOptionsGlobalsSettings;
class WorkspaceOptionsPlasmaSettings;

class KCMWorkspaceOptions : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(WorkspaceOptionsGlobalsSettings *globalsSettings READ globalsSettings CONSTANT)
    Q_PROPERTY(WorkspaceOptionsPlasmaSettings *plasmaSettings READ plasmaSettings CONSTANT)

public:
    KCMWorkspaceOptions(QObject *parent, const QVariantList &args);
    ~KCMWorkspaceOptions() override {}

    WorkspaceOptionsGlobalsSettings *globalsSettings() const;
    WorkspaceOptionsPlasmaSettings *plasmaSettings() const;

public Q_SLOTS:
    void save() override;

private:
    WorkspaceOptionsGlobalsSettings *m_globalsSettings;
    WorkspaceOptionsPlasmaSettings *m_plasmaSettings;
};

#endif  // _KCM_WORKSPACEOPTIONS_H
