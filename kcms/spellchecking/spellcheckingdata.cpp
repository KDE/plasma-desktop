/*
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "spellcheckingdata.h"

#include <QVariantList>

#include <Sonnet/Settings>

#include "spellcheckingskeleton.h"

SpellCheckingData::SpellCheckingData(QObject *parent, const QVariantList &args)
    : KCModuleData(parent, args)
    , m_settings(new SpellCheckingSkeleton(this))
{
    autoRegisterSkeletons();
}

SpellCheckingSkeleton *SpellCheckingData::settings() const
{
    return m_settings;
}

bool SpellCheckingData::isDefaults() const
{
    bool isDefaults = KCModuleData::isDefaults();

    QStringList refIgnoreList(m_settings->ignoreList());
    refIgnoreList.removeDuplicates();
    refIgnoreList.sort();

    QStringList defaultIgnoreList(Sonnet::Settings::defaultIgnoreList());
    defaultIgnoreList.removeDuplicates();
    defaultIgnoreList.sort();

    QStringList refPreferredLanguagesList(m_settings->preferredLanguages());
    refPreferredLanguagesList.removeDuplicates();
    refPreferredLanguagesList.sort();

    QStringList defaultPreferredLanguagesList(Sonnet::Settings::defaultPreferredLanguages());
    defaultPreferredLanguagesList.removeDuplicates();
    defaultPreferredLanguagesList.sort();

    isDefaults &= refIgnoreList == defaultIgnoreList;
    isDefaults &= refPreferredLanguagesList == defaultPreferredLanguagesList;
    isDefaults &= m_settings->defaultLanguage() == Sonnet::Settings::defaultDefaultLanguage();

    return isDefaults;
}

#include "spellcheckingdata.moc"
