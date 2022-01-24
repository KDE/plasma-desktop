/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_componentchooser.h"

#include <KAboutData>
#include <KBuildSycocaProgressDialog>
#include <KLocalizedString>
#include <KPluginFactory>

#include "componentchooserarchivemanager.h"
#include "componentchooserbrowser.h"
#include "componentchooserdata.h"
#include "componentchooseremail.h"
#include "componentchooserfilemanager.h"
#include "componentchooserimageviewer.h"
#include "componentchoosermatrix.h"
#include "componentchoosermusicplayer.h"
#include "componentchooserpdfviewer.h"
#include "componentchooserterminal.h"
#include "componentchoosertexteditor.h"
#include "componentchooservideoplayer.h"

K_PLUGIN_FACTORY_WITH_JSON(KcmComponentChooserFactory, "componentchooser.json", registerPlugin<KcmComponentChooser>(); registerPlugin<ComponentChooserData>();)

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
    connect(geoUriHandlers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(telUriHandlers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(textEditors(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(imageViewers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(musicPlayers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(videoPlayers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(pdfViewers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(archiveManagers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
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

ComponentChooser *KcmComponentChooser::geoUriHandlers() const
{
    return m_data->geoUriHandlers();
}

ComponentChooser *KcmComponentChooser::telUriHandlers() const
{
    return m_data->telUriHandlers();
}

ComponentChooser *KcmComponentChooser::textEditors() const
{
    return m_data->textEditors();
}

ComponentChooser *KcmComponentChooser::imageViewers() const
{
    return m_data->imageViewers();
}

ComponentChooser *KcmComponentChooser::musicPlayers() const
{
    return m_data->musicPlayers();
}
ComponentChooser *KcmComponentChooser::videoPlayers() const
{
    return m_data->videoPlayers();
}
ComponentChooser *KcmComponentChooser::pdfViewers() const
{
    return m_data->pdfViewers();
}
ComponentChooser *KcmComponentChooser::archiveManagers() const
{
    return m_data->archiveManagers();
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
