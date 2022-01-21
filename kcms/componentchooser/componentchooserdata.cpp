/*
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchooserdata.h"

#include "componentchooserbrowser.h"
#include "componentchooseremail.h"
#include "componentchooserfilemanager.h"
#include "componentchoosergeo.h"
#include "componentchoosertel.h"
#include "componentchooserimageviewer.h"
#include "componentchoosermusicplayer.h"
#include "componentchooserpdfviewer.h"
#include "componentchooserterminal.h"
#include "componentchoosertexteditor.h"
#include "componentchooservideoplayer.h"
#include "componentchooserarchivemanager.h"

ComponentChooserData::ComponentChooserData(QObject *parent, const QVariantList &args)
    : KCModuleData(parent, args)
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
    load();
}

void ComponentChooserData::load()
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

void ComponentChooserData::save()
{
    m_browsers->save();
    m_fileManagers->save();
    m_terminalEmulators->save();
    m_emailClients->save();
    m_geoUriHandlers->save();
    m_telUriHandlers->save();
    m_textEditors->save();
    m_imageViewers->save();
    m_musicPlayers->save();
    m_videoPlayers->save();
    m_pdfViewers->save();
    m_archiveManagers->save();
}

void ComponentChooserData::defaults()
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

bool ComponentChooserData::isDefaults() const
{
    return m_browsers->isDefaults() && m_fileManagers->isDefaults() && m_terminalEmulators->isDefaults() && m_emailClients->isDefaults()
        && m_geoUriHandlers->isDefaults() && m_telUriHandlers->isDefaults()
        && m_textEditors->isDefaults() && m_imageViewers->isDefaults() && m_musicPlayers->isDefaults() && m_videoPlayers->isDefaults()
        && m_pdfViewers->isDefaults() && m_archiveManagers->isDefaults();
}

bool ComponentChooserData::isSaveNeeded() const
{
    return m_browsers->isSaveNeeded() || m_fileManagers->isSaveNeeded() || m_terminalEmulators->isSaveNeeded() || m_emailClients->isSaveNeeded()
        || m_geoUriHandlers->isSaveNeeded() || m_telUriHandlers->isSaveNeeded()
        || m_textEditors->isSaveNeeded() || m_imageViewers->isSaveNeeded() || m_musicPlayers->isSaveNeeded() || m_videoPlayers->isSaveNeeded()
        || m_pdfViewers->isSaveNeeded()
        || m_archiveManagers->isSaveNeeded();
}

ComponentChooser *ComponentChooserData::browsers() const
{
    return m_browsers;
}

ComponentChooser *ComponentChooserData::fileManagers() const
{
    return m_fileManagers;
}

ComponentChooser *ComponentChooserData::terminalEmulators() const
{
    return m_terminalEmulators;
}

ComponentChooser *ComponentChooserData::emailClients() const
{
    return m_emailClients;
}

ComponentChooser *ComponentChooserData::geoUriHandlers() const
{
    return m_geoUriHandlers;
}

ComponentChooser *ComponentChooserData::textEditors() const
{
    return m_textEditors;
}

ComponentChooser *ComponentChooserData::telUriHandlers() const
{
    return m_telUriHandlers;
}

ComponentChooser *ComponentChooserData::imageViewers() const
{
    return m_imageViewers;
}

ComponentChooser *ComponentChooserData::musicPlayers() const
{
    return m_musicPlayers;
}

ComponentChooser *ComponentChooserData::videoPlayers() const
{
    return m_videoPlayers;
}
ComponentChooser *ComponentChooserData::pdfViewers() const
{
    return m_pdfViewers;
}
ComponentChooser *ComponentChooserData::archiveManagers() const
{
    return m_archiveManagers;
}
