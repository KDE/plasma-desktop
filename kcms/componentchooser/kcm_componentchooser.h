/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KQuickAddons/ManagedConfigModule>

#include "componentchooser.h"

class ComponentChooserData;

class KcmComponentChooser : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(ComponentChooser *browsers READ browsers CONSTANT)
    Q_PROPERTY(ComponentChooser *emailClients READ emailClients CONSTANT)
    Q_PROPERTY(ComponentChooser *terminalEmulators READ terminalEmulators CONSTANT)
    Q_PROPERTY(ComponentChooser *fileManagers READ fileManagers CONSTANT)
    Q_PROPERTY(ComponentChooser *geoUriHandlers READ geoUriHandlers CONSTANT)
    Q_PROPERTY(ComponentChooser *telUriHandlers READ telUriHandlers CONSTANT)

public:
    KcmComponentChooser(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);

    ComponentChooser *browsers() const;
    ComponentChooser *emailClients() const;
    ComponentChooser *terminalEmulators() const;
    ComponentChooser *fileManagers() const;
    ComponentChooser *geoUriHandlers() const;
    ComponentChooser *telUriHandlers() const;

    void defaults() override;
    void load() override;
    void save() override;
    bool isDefaults() const override;
    bool isSaveNeeded() const override;

private:
    ComponentChooserData *const m_data;
};
