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
    , m_data(new SpellCheckingData(this))
{
    connect(settings(), &Sonnet::Settings::modifiedChanged, this, [this] {
        setNeedsSave(settings()->modified());
    });

    const auto updateRepresentsDefaults = [this] {
        setRepresentsDefaults(m_data->settingsAreDefaults());
    };
    connect(settings(), &Sonnet::Settings::skipUppercaseChanged, this, updateRepresentsDefaults);
    connect(settings(), &Sonnet::Settings::autodetectLanguageChanged, this, updateRepresentsDefaults);
    connect(settings(), &Sonnet::Settings::backgroundCheckerEnabledChanged, this, updateRepresentsDefaults);
    connect(settings(), &Sonnet::Settings::checkerEnabledByDefaultChanged, this, updateRepresentsDefaults);
    connect(settings(), &Sonnet::Settings::skipRunTogetherChanged, this, updateRepresentsDefaults);
    connect(settings(), &Sonnet::Settings::defaultLanguageChanged, this, updateRepresentsDefaults);
    connect(settings(), &Sonnet::Settings::preferredLanguagesChanged, this, updateRepresentsDefaults);
    connect(settings(), &Sonnet::Settings::currentIgnoreListChanged, this, updateRepresentsDefaults);
}

void SonnetSpellCheckingModule::load()
{
    settings()->restore();
}

void SonnetSpellCheckingModule::save()
{
    settings()->save();
}

void SonnetSpellCheckingModule::defaults()
{
    settings()->setCurrentIgnoreList(Sonnet::Settings::defaultIgnoreList());
    settings()->setSkipUppercase(Sonnet::Settings::defaultSkipUppercase());
    settings()->setAutodetectLanguage(Sonnet::Settings::defaultAutodetectLanguage());
    settings()->setBackgroundCheckerEnabled(Sonnet::Settings::defaultBackgroundCheckerEnabled());
    settings()->setCheckerEnabledByDefault(Sonnet::Settings::defaultCheckerEnabledByDefault());
    settings()->setSkipRunTogether(Sonnet::Settings::defaultSkipRunTogether());
    settings()->setDefaultLanguage(Sonnet::Settings::defaultDefaultLanguage());
    settings()->setPreferredLanguages(Sonnet::Settings::defaultPreferredLanguages());
}

Sonnet::Settings *SonnetSpellCheckingModule::settings() const
{
    return m_data->settings();
}

#include "spellchecking.moc"

#include "moc_spellchecking.cpp"
