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
#include "workspaceoptions.h"

#include "ui_mainpage.h"

#include <KAboutData>
#include <KPluginFactory>
#include <KConfigGroup>

using namespace KAuth;

K_PLUGIN_FACTORY(WorkspaceOptionsModuleFactory, registerPlugin<WorkspaceOptionsModule>();)
K_EXPORT_PLUGIN(WorkspaceOptionsModuleFactory("kcmworkspaceoptions"))


WorkspaceOptionsModule::WorkspaceOptionsModule(QWidget *parent, const QVariantList &args)
  : KCModule(parent, args),
    m_kwinConfig( KSharedConfig::openConfig("kwinrc")),
    m_ownConfig( KSharedConfig::openConfig("workspaceoptionsrc")),
    m_plasmaShellAutostart("plasmashell"),
    m_krunnerAutostart("krunner"),
    m_ui(new Ui::MainPage)
{
    KAboutData *about =
    new KAboutData(QStringLiteral("kcmworkspaceoptions"), i18n("Global options for the Plasma Workspace"),
                   QString("1.0"), QString(), KAboutLicense::GPL,
                   i18n("(c) 2009 Marco Martin"));

    about->addAuthor(i18n("Marco Martin"), i18n("Maintainer"), "notmart@gmail.com");

    setAboutData(about);

    setButtons(Help|Apply);

    m_ui->setupUi(this);

    connect(m_ui->showToolTips, SIGNAL(toggled(bool)), this, SLOT(changed()));
}

WorkspaceOptionsModule::~WorkspaceOptionsModule()
{
    delete m_ui;
}


void WorkspaceOptionsModule::save()
{
    {
        KConfig config("plasmarc");
        KConfigGroup cg(&config, "PlasmaToolTips");
        cg.writeEntry("Delay", m_ui->showToolTips->isChecked() ? 0.7 : -1);
    }

}

void WorkspaceOptionsModule::load()
{
    {
        KConfig config("plasmarc");
        KConfigGroup cg(&config, "PlasmaToolTips");
        m_ui->showToolTips->setChecked(cg.readEntry("Delay", 0.7) > 0);
    }
}

void WorkspaceOptionsModule::defaults()
{
    m_ui->showToolTips->setChecked(true);
}

#include "workspaceoptions.moc"
