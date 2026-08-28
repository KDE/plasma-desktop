/*
    SPDX-FileCopyrightText: 2008 Albert Astals Cid <aacid@kde.org>
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "spellchecking.h"

#include "spellcheckingdata.h"

#include <KPluginFactory>
#include <Sonnet/Settings>

K_PLUGIN_FACTORY_WITH_JSON(SpellFactory, "kcm_spellchecking.json", registerPlugin<SonnetSpellCheckingModule>(); registerPlugin<SpellCheckingData>();)

SonnetSpellCheckingModule::SonnetSpellCheckingModule(QObject *parent, const KPluginMetaData &data)
    : KQuickConfigModule(parent, data)
    , m_settings(new Sonnet::Settings(this))
{
    connect(m_settings, &Sonnet::Settings::modifiedChanged, this, [this] {
        setNeedsSave(true);
    });
}

void SonnetSpellCheckingModule::save()
{
    m_settings->save();
}

void SonnetSpellCheckingModule::defaults()
{
    m_settings->setCurrentIgnoreList(Sonnet::Settings::defaultIgnoreList());
    m_settings->setSkipUppercase(Sonnet::Settings::defaultSkipUppercase());
    m_settings->setAutodetectLanguage(Sonnet::Settings::defaultAutodetectLanguage());
    m_settings->setBackgroundCheckerEnabled(Sonnet::Settings::defaultBackgroundCheckerEnabled());
    m_settings->setCheckerEnabledByDefault(Sonnet::Settings::defaultCheckerEnabledByDefault());
    m_settings->setSkipRunTogether(Sonnet::Settings::defaultSkipRunTogether());
    m_settings->setDefaultLanguage(Sonnet::Settings::defaultDefaultLanguage());
    m_settings->setPreferredLanguages(Sonnet::Settings::defaultPreferredLanguages());
}

#include "spellchecking.moc"

#include "moc_spellchecking.cpp"
