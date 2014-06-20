/*
 *  Copyright (C) 2009 Marco Martin <notmart@gmail.com>
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
 *
 */
#ifndef WORKSPACEOPTIONS_H
#define WORKSPACEOPTIONS_H

#include <kcmodule.h>
#include <ksharedconfig.h>
#include <kautostart.h>

namespace Ui {
  class MainPage;
}

class WorkspaceOptionsModule : public KCModule
{
    Q_OBJECT

public:
    WorkspaceOptionsModule(QWidget *parent, const QVariantList &);
    ~WorkspaceOptionsModule();
    void save();
    void load();
    void defaults();

private:
    KSharedConfigPtr m_kwinConfig;
    KSharedConfigPtr m_ownConfig;
    KAutostart m_plasmaShellAutostart;
    KAutostart m_krunnerAutostart;
    bool m_currentlyIsDesktop;
    bool m_currentlyFixedDashboard;

    Ui::MainPage *m_ui;
};

#endif // WORKSPACEOPTIONS_H
