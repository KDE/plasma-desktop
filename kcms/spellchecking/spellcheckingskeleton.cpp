/*
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "spellcheckingskeleton.h"
#include <Sonnet/ConfigView>
#include <Sonnet/Settings>

SpellCheckingSkeleton::SpellCheckingSkeleton(QObject *parent)
    : KCoreConfigSkeleton(QString(), parent)
    , m_store(new Sonnet::Settings(this))
{
    // Associated with managed widget
    addItem(new KPropertySkeletonItem(m_store, "skipUppercase", Sonnet::Settings::defaultSkipUppercase()), "skipUppercase");
    addItem(new KPropertySkeletonItem(m_store, "autodetectLanguage", Sonnet::Settings::defaultAutodetectLanguage()), "autodetectLanguage");
    addItem(new KPropertySkeletonItem(m_store, "backgroundCheckerEnabled", Sonnet::Settings::defaultBackgroundCheckerEnabled()), "backgroundCheckerEnabled");
    addItem(new KPropertySkeletonItem(m_store, "checkerEnabledByDefault", Sonnet::Settings::defaultCheckerEnabledByDefault()), "checkerEnabledByDefault");
    addItem(new KPropertySkeletonItem(m_store, "skipRunTogether", Sonnet::Settings::defauktSkipRunTogether()), "skipRunTogether");
    usrRead();
}

void SpellCheckingSkeleton::usrRead()
{
    m_ignoreList = m_store->currentIgnoreList();
    m_preferredLanguages = m_store->preferredLanguages();
    m_defaultLanguage = m_store->defaultLanguage();
    KCoreConfigSkeleton::usrRead();
}

bool SpellCheckingSkeleton::usrSave()
{
    m_store->setCurrentIgnoreList(m_ignoreList);
    m_store->setPreferredLanguages(m_preferredLanguages);
    m_store->setDefaultLanguage(m_defaultLanguage);
    m_store->save();
    return KCoreConfigSkeleton::usrSave();
}

void SpellCheckingSkeleton::setPreferredLanguages(const QStringList &preferredLanguages)
{
    m_preferredLanguages = preferredLanguages;
}

QStringList SpellCheckingSkeleton::preferredLanguages() const
{
    return m_preferredLanguages;
}

void SpellCheckingSkeleton::setIgnoreList(const QStringList &ignoreList)
{
    m_ignoreList = ignoreList;
}

QStringList SpellCheckingSkeleton::ignoreList() const
{
    return m_ignoreList;
}

void SpellCheckingSkeleton::setDefaultLanguage(const QString &defaultLanguage)
{
    m_defaultLanguage = defaultLanguage;
}

QString SpellCheckingSkeleton::defaultLanguage() const
{
    return m_defaultLanguage;
}

QStringList SpellCheckingSkeleton::clients() const
{
    return m_store->clients();
}

#include "spellcheckingskeleton.moc"
