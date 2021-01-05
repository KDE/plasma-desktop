/***************************************************************************
 *   Copyright (C) 2020 Tobias Fella <fella@posteo.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "kcm_componentchooser.h"

#include <KAboutData>
#include <KBuildSycocaProgressDialog>
#include <KLocalizedString>
#include <KPluginFactory>

#include "componentchooserbrowser.h"
#include "componentchooserfilemanager.h"
#include "componentchooserterminal.h"
#include "componentchooseremail.h"

K_PLUGIN_CLASS_WITH_JSON(KcmComponentChooser, "metadata.json")

KcmComponentChooser::KcmComponentChooser(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
{
    KAboutData *aboutData = new KAboutData("kcm_componentchooser", i18nc("@title", "Default Applications"), "1.0", QString(), KAboutLicense::LicenseKey::GPL_V2);

    aboutData->addAuthor(i18n("Joseph Wenninger"), QString(), QStringLiteral("jowenn@kde.org"));
    aboutData->addAuthor(i18n("Méven Car"), QString(), QStringLiteral("meven.car@kdemail.net"));
    aboutData->addAuthor(i18n("Tobias Fella"), QString(), QStringLiteral("fella@posteo.de"));

    setAboutData(aboutData);
    setButtons(Help | Default | Apply);

    m_browsers = new ComponentChooserBrowser(this);
    m_fileManagers = new ComponentChooserFileManager(this);
    m_terminalEmulators = new ComponentChooserTerminal(this);
    m_emailClients = new ComponentChooserEmail(this);

    connect(m_browsers, &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(m_fileManagers, &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(m_terminalEmulators, &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(m_emailClients, &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
}

void KcmComponentChooser::defaults()
{
    m_browsers->defaults();
    m_fileManagers->defaults();
    m_terminalEmulators->defaults();
    m_emailClients->defaults();
}

void KcmComponentChooser::load()
{
    m_browsers->load();
    m_fileManagers->load();
    m_terminalEmulators->load();
    m_emailClients->load();
}

void KcmComponentChooser::save()
{
    m_browsers->save();
    m_fileManagers->save();
    m_terminalEmulators->save();
    m_emailClients->save();

    KBuildSycocaProgressDialog::rebuildKSycoca(nullptr);
}

bool KcmComponentChooser::isDefaults() const
{
    return m_browsers->isDefaults() && m_fileManagers->isDefaults() && m_terminalEmulators->isDefaults() && m_emailClients->isDefaults();
}
bool KcmComponentChooser::isSaveNeeded() const
{
    return m_browsers->isSaveNeeded() || m_fileManagers->isSaveNeeded() || m_terminalEmulators->isSaveNeeded() || m_emailClients->isSaveNeeded();
}


#include "kcm_componentchooser.moc"
