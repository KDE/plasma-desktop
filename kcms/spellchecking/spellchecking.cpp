/*
    SPDX-FileCopyrightText: 2008 Albert Astals Cid <aacid@kde.org>
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "spellchecking.h"

#include <QBoxLayout>
#include <QSet>

#include "spellcheckingdata.h"
#include <KConfigDialogManager>
#include <KPluginFactory>
#include <Sonnet/ConfigView>
#include <Sonnet/Settings>

K_PLUGIN_FACTORY(SpellFactory, registerPlugin<SonnetSpellCheckingModule>(); registerPlugin<SpellCheckingData>();)

SonnetSpellCheckingModule::SonnetSpellCheckingModule(QWidget *parent, const QVariantList &)
    : KCModule(parent)
    , m_data(new SpellCheckingData(this))
{
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_configWidget = new Sonnet::ConfigView(this);
    m_configWidget->setNoBackendFoundVisible(skeleton()->clients().isEmpty());
    layout->addWidget(m_configWidget);
    m_managedConfig = addConfig(skeleton(), m_configWidget);
    connect(m_configWidget, &Sonnet::ConfigView::configChanged, this, &SonnetSpellCheckingModule::stateChanged);
}

void SonnetSpellCheckingModule::stateChanged()
{
    bool unmanagedChangeState = false;
    bool unmanagedDefaultState = true;

    QStringList refIgnoreList(skeleton()->ignoreList());
    refIgnoreList.removeDuplicates();
    refIgnoreList.sort();

    QStringList currentIgnoreList(m_configWidget->ignoreList());
    currentIgnoreList.removeDuplicates();
    currentIgnoreList.sort();

    QStringList defaultIgnoreList(Sonnet::Settings::defaultIgnoreList());
    defaultIgnoreList.removeDuplicates();
    defaultIgnoreList.sort();

    unmanagedChangeState |= currentIgnoreList != refIgnoreList;
    unmanagedDefaultState &= currentIgnoreList == defaultIgnoreList;

    QStringList refPreferredLanguagesList(skeleton()->preferredLanguages());
    refPreferredLanguagesList.removeDuplicates();
    refPreferredLanguagesList.sort();

    QStringList currentPreferredLanguagesList(m_configWidget->preferredLanguages());
    currentPreferredLanguagesList.removeDuplicates();
    currentPreferredLanguagesList.sort();

    QStringList defaultPreferredLanguagesList(Sonnet::Settings::defaultPreferredLanguages());
    defaultPreferredLanguagesList.removeDuplicates();
    defaultPreferredLanguagesList.sort();

    unmanagedChangeState |= currentPreferredLanguagesList != refPreferredLanguagesList;
    unmanagedDefaultState &= currentPreferredLanguagesList == defaultPreferredLanguagesList;

    unmanagedChangeState |= skeleton()->defaultLanguage() != m_configWidget->language();
    unmanagedDefaultState &= m_configWidget->language() == Sonnet::Settings::defaultDefaultLanguage();

    unmanagedWidgetDefaultState(unmanagedDefaultState);
    unmanagedWidgetChangeState(unmanagedChangeState);
}

SonnetSpellCheckingModule::~SonnetSpellCheckingModule()
{
}

void SonnetSpellCheckingModule::load()
{
    KCModule::load();
    // Set unmanaged widget value
    m_configWidget->setIgnoreList(skeleton()->ignoreList());
    m_configWidget->setPreferredLanguages(skeleton()->preferredLanguages());
    m_configWidget->setLanguage(skeleton()->defaultLanguage());
}

void SonnetSpellCheckingModule::save()
{
    skeleton()->setIgnoreList(m_configWidget->ignoreList());
    skeleton()->setPreferredLanguages(m_configWidget->preferredLanguages());
    skeleton()->setDefaultLanguage(m_configWidget->language());

    // with addConfig, save on skeleton will be trigger only if one managed widget changed
    if (!m_managedConfig->hasChanged()) {
        skeleton()->save();
    }
    KCModule::save();
}

void SonnetSpellCheckingModule::defaults()
{
    KCModule::defaults();
    // set default value for unmanaged widgets
    m_configWidget->setIgnoreList(Sonnet::Settings::defaultIgnoreList());
    m_configWidget->setPreferredLanguages(Sonnet::Settings::defaultPreferredLanguages());
    m_configWidget->setLanguage(Sonnet::Settings::defaultDefaultLanguage());
}

SpellCheckingSkeleton *SonnetSpellCheckingModule::skeleton() const
{
    return m_data->settings();
}

#include "spellchecking.moc"
