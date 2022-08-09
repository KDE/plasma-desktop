/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_componentchooser.h"

#include <QApplication>

#include <KBuildSycocaProgressDialog>
#include <KLocalizedString>
#include <KPluginFactory>

#include "componentchooserbrowser.h"
#include "componentchooserdata.h"
#include "componentchooseremail.h"
#include "componentchooserfilemanager.h"
#include "componentchooserterminal.h"

K_PLUGIN_FACTORY_WITH_JSON(KcmComponentChooserFactory, "kcm_componentchooser.json", registerPlugin<KcmComponentChooser>();
                           registerPlugin<ComponentChooserData>();)

KcmComponentChooser::KcmComponentChooser(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, metaData, args)
    , m_data(new ComponentChooserData(this))
{
    setButtons(Help | Default | Apply);

    connect(browsers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(fileManagers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(terminalEmulators(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(emailClients(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(geoUriHandlers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
    connect(telUriHandlers(), &ComponentChooser::indexChanged, this, &KcmComponentChooser::settingsChanged);
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
    KBuildSycocaProgressDialog::rebuildKSycoca(QApplication::activeWindow());
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
