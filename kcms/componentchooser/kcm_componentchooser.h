/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KQuickManagedConfigModule>

#include "componentchooser.h"

class ComponentChooserData;

class KcmComponentChooser : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(ComponentChooser *browsers READ browsers CONSTANT)
    Q_PROPERTY(ComponentChooser *emailClients READ emailClients CONSTANT)
    Q_PROPERTY(ComponentChooser *terminalEmulators READ terminalEmulators CONSTANT)
    Q_PROPERTY(ComponentChooser *fileManagers READ fileManagers CONSTANT)
    Q_PROPERTY(ComponentChooser *geoUriHandlers READ geoUriHandlers CONSTANT)
    Q_PROPERTY(ComponentChooser *telUriHandlers READ telUriHandlers CONSTANT)
    Q_PROPERTY(ComponentChooser *textEditors READ textEditors CONSTANT)
    Q_PROPERTY(ComponentChooser *imageViewers READ imageViewers CONSTANT)
    Q_PROPERTY(ComponentChooser *musicPlayers READ musicPlayers CONSTANT)
    Q_PROPERTY(ComponentChooser *videoPlayers READ videoPlayers CONSTANT)
    Q_PROPERTY(ComponentChooser *pdfViewers READ pdfViewers CONSTANT)
    Q_PROPERTY(ComponentChooser *archiveManagers READ archiveManagers CONSTANT)

public:
    KcmComponentChooser(QObject *parent, const KPluginMetaData &metaData);

    ComponentChooser *browsers() const;
    ComponentChooser *emailClients() const;
    ComponentChooser *terminalEmulators() const;
    ComponentChooser *fileManagers() const;
    ComponentChooser *geoUriHandlers() const;
    ComponentChooser *telUriHandlers() const;
    ComponentChooser *textEditors() const;
    ComponentChooser *imageViewers() const;
    ComponentChooser *musicPlayers() const;
    ComponentChooser *videoPlayers() const;
    ComponentChooser *pdfViewers() const;
    ComponentChooser *archiveManagers() const;

    void defaults() override;
    void load() override;
    void save() override;
    bool isDefaults() const override;
    bool isSaveNeeded() const override;

private:
    ComponentChooser *m_browsers;
    ComponentChooser *m_fileManagers;
    ComponentChooser *m_terminalEmulators;
    ComponentChooser *m_emailClients;
    ComponentChooser *m_geoUriHandlers;
    ComponentChooser *m_telUriHandlers;
    ComponentChooser *m_textEditors;
    ComponentChooser *m_imageViewers;
    ComponentChooser *m_musicPlayers;
    ComponentChooser *m_videoPlayers;
    ComponentChooser *m_pdfViewers;
    ComponentChooser *m_archiveManagers;
};
