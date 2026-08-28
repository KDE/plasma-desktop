/*
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "spellcheckingdata.h"

#include <QVariantList>

#include <Sonnet/Settings>

SpellCheckingData::SpellCheckingData(QObject *parent)
    : KCModuleData(parent)
    , m_settings(new Sonnet::Settings(this))
{
}

bool SpellCheckingData::isDefaults() const
{
    if (m_settings->skipUppercase() != Sonnet::Settings::defaultSkipUppercase()) {
        return false;
    }

    if (m_settings->autodetectLanguage() != Sonnet::Settings::defaultAutodetectLanguage()) {
        return false;
    }

    if (m_settings->backgroundCheckerEnabled() != Sonnet::Settings::defaultBackgroundCheckerEnabled()) {
        return false;
    }

    if (m_settings->checkerEnabledByDefault() != Sonnet::Settings::defaultCheckerEnabledByDefault()) {
        return false;
    }

    if (m_settings->skipRunTogether() != Sonnet::Settings::defaultSkipRunTogether()) {
        return false;
    }

    if (m_settings->defaultLanguage() != Sonnet::Settings::defaultDefaultLanguage()) {
        return false;
    }

    const auto &currentIgnoreList = m_settings->currentIgnoreList();
    const auto &defaultIgnoreList = Sonnet::Settings::defaultIgnoreList();
    if (QSet(currentIgnoreList.begin(), currentIgnoreList.end()) != QSet(defaultIgnoreList.begin(), defaultIgnoreList.end())) {
        return false;
    }

    const auto &currentPreferredLanguages = m_settings->preferredLanguages();
    const auto &defaultPreferredLanguages = Sonnet::Settings::defaultPreferredLanguages();
    if (QSet(currentPreferredLanguages.begin(), currentPreferredLanguages.end()) != QSet(defaultPreferredLanguages.begin(), defaultPreferredLanguages.end())) {
        return false;
    }

    return KCModuleData::isDefaults();
}

#include "spellcheckingdata.moc"

#include "moc_spellcheckingdata.cpp"
