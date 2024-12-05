/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_componentchooser.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>

#include <KBuildSycocaProgressDialog>
#include <KLocalizedString>
#include <KPluginFactory>

#include "components/componentchooserarchivemanager.h"
#include "components/componentchooserbrowser.h"
#include "components/componentchooseremail.h"
#include "components/componentchooserfilemanager.h"
#include "components/componentchoosergeo.h"
#include "components/componentchooserimageviewer.h"
#include "components/componentchoosermusicplayer.h"
#include "components/componentchooserpdfviewer.h"
#include "components/componentchoosertel.h"
#include "components/componentchooserterminal.h"
#include "components/componentchoosertexteditor.h"
#include "components/componentchooservideoplayer.h"

extern KSERVICE_EXPORT int ksycoca_ms_between_checks;

K_PLUGIN_FACTORY_WITH_JSON(KcmComponentChooserFactory, "kcm_componentchooser.json", registerPlugin<KcmComponentChooser>();)

KcmComponentChooser::KcmComponentChooser(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
    , m_browsers(new ComponentChooserBrowser(this))
    , m_fileManagers(new ComponentChooserFileManager(this))
    , m_terminalEmulators(new ComponentChooserTerminal(this))
    , m_emailClients(new ComponentChooserEmail(this))
    , m_geoUriHandlers(new ComponentChooserGeo(this))
    , m_telUriHandlers(new ComponentChooserTel(this))
    , m_textEditors(new ComponentChooserTextEditor(this))
    , m_imageViewers(new ComponentChooserImageViewer(this))
    , m_musicPlayers(new ComponentChooserMusicPlayer(this))
    , m_videoPlayers(new ComponentChooserVideoPlayer(this))
    , m_pdfViewers(new ComponentChooserPdfViewer(this))
    , m_archiveManagers(new ComponentChooserArchiveManager(this))
{
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
    return m_browsers;
}

ComponentChooser *KcmComponentChooser::emailClients() const
{
    return m_emailClients;
}

ComponentChooser *KcmComponentChooser::terminalEmulators() const
{
    return m_terminalEmulators;
}

ComponentChooser *KcmComponentChooser::fileManagers() const
{
    return m_fileManagers;
}

ComponentChooser *KcmComponentChooser::geoUriHandlers() const
{
    return m_geoUriHandlers;
}

ComponentChooser *KcmComponentChooser::telUriHandlers() const
{
    return m_telUriHandlers;
}

ComponentChooser *KcmComponentChooser::textEditors() const
{
    return m_textEditors;
}

ComponentChooser *KcmComponentChooser::imageViewers() const
{
    return m_imageViewers;
}

ComponentChooser *KcmComponentChooser::musicPlayers() const
{
    return m_musicPlayers;
}

ComponentChooser *KcmComponentChooser::videoPlayers() const
{
    return m_videoPlayers;
}

ComponentChooser *KcmComponentChooser::pdfViewers() const
{
    return m_pdfViewers;
}

ComponentChooser *KcmComponentChooser::archiveManagers() const
{
    return m_archiveManagers;
}

void KcmComponentChooser::defaults()
{
    m_browsers->defaults();
    m_fileManagers->defaults();
    m_terminalEmulators->defaults();
    m_emailClients->defaults();
    m_geoUriHandlers->defaults();
    m_telUriHandlers->defaults();
    m_textEditors->defaults();
    m_imageViewers->defaults();
    m_musicPlayers->defaults();
    m_videoPlayers->defaults();
    m_pdfViewers->defaults();
    m_archiveManagers->defaults();
}

void KcmComponentChooser::load()
{
    m_browsers->load();
    m_fileManagers->load();
    m_terminalEmulators->load();
    m_emailClients->load();
    m_geoUriHandlers->load();
    m_telUriHandlers->load();
    m_textEditors->load();
    m_imageViewers->load();
    m_musicPlayers->load();
    m_videoPlayers->load();
    m_pdfViewers->load();
    m_archiveManagers->load();
}

void KcmComponentChooser::save()
{
    QList<ComponentChooser *> componentsSaved;

    auto handleSave = [&componentsSaved](ComponentChooser *chooser) {
        if (chooser->isSaveNeeded()) {
            chooser->save();
            componentsSaved.append(chooser);
        }
    };
    handleSave(m_browsers);
    handleSave(m_fileManagers);
    handleSave(m_terminalEmulators);
    handleSave(m_emailClients);
    handleSave(m_geoUriHandlers);
    handleSave(m_telUriHandlers);
    handleSave(m_textEditors);
    handleSave(m_imageViewers);
    handleSave(m_musicPlayers);
    handleSave(m_videoPlayers);
    handleSave(m_pdfViewers);
    handleSave(m_archiveManagers);

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.klauncher5"),
                                                          QStringLiteral("/KLauncher"),
                                                          QStringLiteral("org.kde.KLauncher"),
                                                          QStringLiteral("reparseConfiguration"));
    QDBusConnection::sessionBus().send(message);

    // HACK to ensure mime cache is updated right away
    ComponentChooser::forceReloadServiceCache();

    for (auto *componentSaved : componentsSaved) {
        componentSaved->onSaved();
    }
}

bool KcmComponentChooser::isDefaults() const
{
    return m_browsers->isDefaults() && m_fileManagers->isDefaults() && m_terminalEmulators->isDefaults() && m_emailClients->isDefaults()
        && m_geoUriHandlers->isDefaults() && m_telUriHandlers->isDefaults() && m_textEditors->isDefaults() && m_imageViewers->isDefaults()
        && m_musicPlayers->isDefaults() && m_videoPlayers->isDefaults() && m_pdfViewers->isDefaults() && m_archiveManagers->isDefaults();
}

bool KcmComponentChooser::isSaveNeeded() const
{
    return m_browsers->isSaveNeeded() || m_fileManagers->isSaveNeeded() || m_terminalEmulators->isSaveNeeded() || m_emailClients->isSaveNeeded()
        || m_geoUriHandlers->isSaveNeeded() || m_telUriHandlers->isSaveNeeded() || m_textEditors->isSaveNeeded() || m_imageViewers->isSaveNeeded()
        || m_musicPlayers->isSaveNeeded() || m_videoPlayers->isSaveNeeded() || m_pdfViewers->isSaveNeeded() || m_archiveManagers->isSaveNeeded();
}

#include "kcm_componentchooser.moc"

#include "moc_kcm_componentchooser.cpp"
