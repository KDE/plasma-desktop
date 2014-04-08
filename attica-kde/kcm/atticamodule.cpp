/*
    This file is part of KDE.

    Copyright (c) 2009 Eckhart Wörner <ewoerner@kde.org>
    Copyright (c) 2010 Frederik Gladhorn <gladhorn@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "atticamodule.h"


#include <KPluginFactory>
#include <KAboutData>
#include <KDebug>
#include <kicon.h>
#include <kurlrequesterdialog.h>

#include <attica/providermanager.h>

#include "providerconfigwidget.h"


K_PLUGIN_FACTORY(AtticaModuleFactory, registerPlugin<AtticaModule>();)
K_EXPORT_PLUGIN(AtticaModuleFactory("kcm_attica"))

AtticaModule::AtticaModule(QWidget* parent, const QVariantList&)
    : KCModule(AtticaModuleFactory::componentData(), parent)
{
    KAboutData *about = new KAboutData(
            "kcm_attica", 0, ki18n("Social Desktop"),
            KDE_VERSION_STRING, KLocalizedString(), KAboutData::License_GPL,
            ki18n("Copyright 2009 Eckhart Wörner"));
    about->addAuthor(ki18n("Eckhart Wörner"), KLocalizedString(), "ewoerner@kde.org");
    about->addAuthor(ki18n("Dmitry Suzdalev"), KLocalizedString(), "dimsuz@gmail.com");
    about->addAuthor(ki18n("Frederik Gladhorn"), KLocalizedString(), "gladhorn@kde.org");
    setAboutData(about);

    m_ui.setupUi(this);
    
    m_ui.addProviderButton->setIcon(KIcon("list-add"));
    m_ui.removeProviderButton->setIcon(KIcon("list-remove"));
    
    // FIXME
    m_ui.removeProviderButton->setEnabled(false);
    
    connect(m_ui.addProviderButton, SIGNAL(clicked()), this, SLOT(addProvider()));
    connect(m_ui.removeProviderButton, SIGNAL(clicked()), this, SLOT(removeProvider()));
    
    connect(m_ui.providerComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(providerSelected(int)));
    
    connect(m_ui.providerConfigWidget, SIGNAL(changed(bool)),
            this, SIGNAL(changed(bool)));

    m_manager.setAuthenticationSuppressed(true);

    connect(&m_manager, SIGNAL(providerAdded(const Attica::Provider&)), SLOT(providerAdded(const Attica::Provider&)));
    connect(&m_manager, SIGNAL(defaultProvidersLoaded()), SLOT(onDefaultProvidersLoaded()));

    startLoadingDefaultProviders();
}

AtticaModule::~AtticaModule()
{
}

void AtticaModule::defaults()
{
}

void AtticaModule::load()
{
    startLoadingDefaultProviders();
}


void AtticaModule::save()
{
    m_ui.providerConfigWidget->saveData();
}

void AtticaModule::startLoadingDefaultProviders()
{
    emit changed(true);
    m_manager.clear();
    m_manager.loadDefaultProviders();
    m_ui.lblProviderList->setText(i18n("Loading provider list..."));
    m_ui.providerComboBox->hide();
    m_ui.providerConfigWidget->setEnabled(false);
}

void AtticaModule::providerAdded(const Attica::Provider& provider)
{
    // Add new provider
    QString baseUrl = provider.baseUrl().toString();
    int idx = m_ui.providerComboBox->findData(baseUrl);

    if ( idx == -1)
    {
        kDebug() << "Adding provider" << baseUrl;
        QString name = provider.name();
        if (name.isEmpty())
            name = baseUrl;
        m_ui.providerComboBox->addItem(KIcon("system-users"), name, provider.baseUrl());
    }

    // set only if this is a first provider, otherwise it will be
    // set on explicit selection
    if (m_ui.providerComboBox->count() == 1) {
        m_ui.providerConfigWidget->setProvider(provider);
    }
}

void AtticaModule::providerSelected(int providerNumber)
{
    QUrl providerUrl = m_ui.providerComboBox->itemData(providerNumber).toUrl();
    m_ui.providerConfigWidget->setProvider(m_manager.providerByUrl(providerUrl));
}

void AtticaModule::onDefaultProvidersLoaded()
{
    m_ui.lblProviderList->setText(i18n("Choose a provider to manage:"));
    m_ui.providerComboBox->show();
    m_ui.providerConfigWidget->setEnabled(true);

    // at least now set it to not changed
    emit changed(false);
}

void AtticaModule::addProvider()
{
    KUrlRequesterDialog dialog("http://", i18nc("addition of an attica/knewstuff provider by entering its url", "URL of the provider file (provider.xml)"), this);
    if (dialog.exec() == KDialog::Accepted) {
        kDebug() << "Add provider: " << dialog.selectedUrl();
        m_manager.addProviderFileToDefaultProviders(dialog.selectedUrl());
    }
}

void AtticaModule::removeProvider()
{
    //m_manager.removeProviderFileToDefaultProviders(url);
}


#include "atticamodule.moc"
