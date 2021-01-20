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
#include "componentchooserdata.h"
#include "componentchooseremail.h"
#include "componentchooserfilemanager.h"
#include "componentchooserterminal.h"

K_PLUGIN_FACTORY_WITH_JSON(KcmComponentChooserFactory, "metadata.json", registerPlugin<KcmComponentChooser>(); registerPlugin<ComponentChooserData>();)

KcmComponentChooser::KcmComponentChooser(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_data(new ComponentChooserData(this))
{
    KAboutData *aboutData = new KAboutData("kcm_componentchooser", //
                                           i18nc("@title", "Default Applications"),
                                           "1.0",
                                           QString(),
                                           KAboutLicense::LicenseKey::GPL_V2);

    aboutData->addAuthor(i18n("Joseph Wenninger"), QString(), QStringLiteral("jowenn@kde.org"));
    aboutData->addAuthor(i18n("Méven Car"), QString(), QStringLiteral("meven.car@kdemail.net"));
    aboutData->addAuthor(i18n("Tobias Fella"), QString(), QStringLiteral("fella@posteo.de"));

    setAboutData(aboutData);
    setButtons(Help | Default | Apply);

    connect(browsers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(fileManagers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(terminalEmulators(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(emailClients(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
}

ComponentChooser *KcmComponentChooser::browsers() const
{
    return m_data->browsers();
}

ComponentChooser *KcmComponentChooser::emailClients() const
{
    return m_data->emailClients();
}

ComponentChooser *KcmComponentChooser::terminalEmulators() const
{
    return m_data->terminalEmulators();
}

ComponentChooser *KcmComponentChooser::fileManagers() const
{
    return m_data->fileManagers();
}

void KcmComponentChooser::defaults()
{
    m_data->defaults();
}

void KcmComponentChooser::load()
{
    m_data->load();
}

void KcmComponentChooser::save()
{
    m_data->save();
    KBuildSycocaProgressDialog::rebuildKSycoca(nullptr);
}

bool KcmComponentChooser::isDefaults() const
{
    return m_data->isDefaults();
}

bool KcmComponentChooser::isSaveNeeded() const
{
    return m_data->isSaveNeeded();
}

#include "kcm_componentchooser.moc"
