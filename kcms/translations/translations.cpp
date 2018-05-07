/*
 *  Copyright (C) 2014 John Layt <john@layt.net>
 *  Copyright (C) 2018 Eike Hein <hein@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "translations.h"
#include "translationsmodel.h"

#include "../formats/writeexports.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>

K_PLUGIN_FACTORY_WITH_JSON(TranslationsFactory, "kcm_translations.json", registerPlugin<Translations>();)

Translations::Translations(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_translationsModel(new TranslationsModel(this))
    , m_everSaved(false)
{
    KAboutData *about = new KAboutData("kcm_translations",
        i18n("Configure Plasma translations"),
        "2.0", QString(), KAboutLicense::LGPL);
    setAboutData(about);

    setButtons(Apply | Default);

    m_config = KConfigGroup(KSharedConfig::openConfig(configFile), "Translations");

    connect(m_translationsModel, &TranslationsModel::selectedLanguagesChanged,
        this, &Translations::selectedLanguagesChanged);
    connect(m_translationsModel, &TranslationsModel::missingLanguagesChanged,
        this, &Translations::missingLanguagesChanged);
}

Translations::~Translations()
{
}

QAbstractItemModel* Translations::translationsModel() const
{
    return m_translationsModel;
}

bool Translations::everSaved() const
{
    return m_everSaved;
}

void Translations::load()
{
    m_configuredLanguages = m_config.readEntry(lcLanguage,
        QString()).split(':', QString::SkipEmptyParts);

    m_translationsModel->setSelectedLanguages(m_configuredLanguages);
}

void Translations::save()
{
    m_everSaved = true;
    emit everSavedChanged();

    m_configuredLanguages = m_translationsModel->selectedLanguages();

    for (const QString& lang : m_translationsModel->missingLanguages()) {
        m_configuredLanguages.removeOne(lang);
    }

    m_config.writeEntry(lcLanguage, m_configuredLanguages.join(QStringLiteral(":")), KConfig::Persistent);
    m_config.sync();

    writeExports();

    m_translationsModel->setSelectedLanguages(m_configuredLanguages);
}

void Translations::defaults()
{
    KConfigGroup formatsConfig = KConfigGroup(KSharedConfig::openConfig(configFile), "Formats");

    QString lang = formatsConfig.readEntry("LANG", QString());

    if (lang.isEmpty()
        || !KLocalizedString::availableDomainTranslations("systemsettings").contains(lang)) {
        lang = QLocale::system().name();
    }

    if (!KLocalizedString::availableDomainTranslations("systemsettings").contains(lang)) {
        lang = QStringLiteral("en_US");
    }

    QStringList languages;
    languages << lang;

    m_translationsModel->setSelectedLanguages(languages);
}

void Translations::selectedLanguagesChanged()
{
    setNeedsSave(m_configuredLanguages != m_translationsModel->selectedLanguages());
}

void Translations::missingLanguagesChanged()
{
    if (m_translationsModel->missingLanguages().count()) {
        setNeedsSave(true);
    }
}

#include "translations.moc"
