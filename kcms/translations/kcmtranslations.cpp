/*
 *  Copyright 2010, 2014 John Layt <john@layt.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "kcmtranslations.h"

#include "../formats/writeexports.h"

#include <KAboutData>
#include <KActionSelector>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KMessageWidget>

#include <QListWidget>
#include <QListWidgetItem>
#include <QStandardPaths>

#include "ui_kcmtranslationswidget.h"

K_PLUGIN_FACTORY(KCMTranslationsFactory, registerPlugin<KCMTranslations>();)

KCMTranslations::KCMTranslations(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args),
      m_ui(new Ui::KCMTranslationsWidget),
      m_messageWidget(0)
{
    KAboutData *about = new KAboutData(QStringLiteral("KCMTranslations"),
                                       i18n("Translations"),
                                       QString(),
                                       i18n("Configure Plasma translations"),
                                       KAboutLicense::GPL);
    about->addAuthor(i18n("John Layt"), i18n("Maintainer"), QStringLiteral("jlayt@kde.net"));
    setAboutData(about);

    m_ui->setupUi(this);

    // Get the current config
    m_config = KConfigGroup(KSharedConfig::openConfig(configFile), "Translations");

    // Set up UI to respond to user changing the translation selection
    connect(m_ui->m_selectTranslations, &KActionSelector::added, this,                       &KCMTranslations::changedTranslationsSelected);
    connect(m_ui->m_selectTranslations, &KActionSelector::removed, this,                       &KCMTranslations::changedTranslationsAvailable);
    connect(m_ui->m_selectTranslations, &KActionSelector::movedUp, this,                       &KCMTranslations::changedTranslationsSelected);
    connect(m_ui->m_selectTranslations, &KActionSelector::movedDown, this,                       &KCMTranslations::changedTranslationsSelected);

    // If user clicks the Install button, trigger distro specific install routine
    connect(m_ui->m_buttonTranslationsInstall,  &QPushButton::clicked, this,                               &KCMTranslations::installTranslations);

    // Hide the Install button, this will be activated by those distros that support this feature.
    m_ui->m_buttonTranslationsInstall->setHidden(true);
}

KCMTranslations::~KCMTranslations()
{
    delete m_ui;
}

// Reimp
// Load == User has clicked on Reset to restore saved settings
// Also gets called automatically called after constructor
void KCMTranslations::load()
{
    // Get the currently installed translations for Plasma
    // TODO May want to later add all installed .po files on system?
    m_installedTranslations.clear();

    m_installedTranslations = KLocalizedString::availableDomainTranslations("systemsettings").toList();

    // Load the current user translations
    loadTranslations();

    // Check if any of the current user translations are no longer installed
    // If any missing remove them and save the settings, then tell the user
    QStringList missingLanguages;
    QStringList availableLanguages;
    if (!m_config.isEntryImmutable(lcLanguage)) {
        foreach (const QString &languageCode, m_kcmTranslations) {
            if (m_installedTranslations.contains(languageCode)) {
                availableLanguages.append(languageCode);
            } else {
                missingLanguages.append(languageCode);
            }
        }
        m_config.writeEntry(lcLanguage, availableLanguages.join(":"));
        m_config.sync();
        m_config.config()->reparseConfiguration();
        loadTranslations();
    }

    // Then update all the widgets to use the new settings
    initWidgets();

    if (missingLanguages.count()) {
        const QString txt = i18ncp("%1 is the language code",
                                                "The translation files for the language with the code '%2' "
                                                "could not be found. The "
                                                "language has been removed from your configuration. "
                                                "If you want to add it back, please install the "
                                                "localization files for it and add the language again.",

                                                "The translation files for the languages with the codes "
                                                "'%2' could not be found. These "
                                                "languages have been removed from your configuration. "
                                                "If you want to add them back, please install the "
                                                "localization files for it and the languages again.",
                            missingLanguages.count(),
                            missingLanguages.join("', '"));

        m_messageWidget = new KMessageWidget(this);
        m_messageWidget->setMessageType(KMessageWidget::Information);
        m_messageWidget->setWordWrap(true);
        m_messageWidget->setText(txt);

        m_ui->verticalLayout->insertWidget(0, m_messageWidget);
    }
}

// Reimp
// Save the new LANGUAGE setting
void KCMTranslations::save()
{
    m_config.writeEntry(lcLanguage, m_kcmTranslations.join(":"), KConfig::Persistent | KConfig::Global);
    m_config.sync();
    KMessageBox::information(this,
                             i18n("Your changes will take effect the next time you log in."),
                             i18n("Applying Language Settings"),
                             QLatin1String("LanguageChangesApplyOnlyToNewlyStartedPrograms"));
    load();
    writeExports();
}

// Reimp
// Defaults == User has clicked on Defaults to load default settings
// We interpret this to mean the default LANG setting
void KCMTranslations::defaults()
{
    // Get the users LANG setting, or if empty the system LANG
    KConfigGroup formatsConfig = KConfigGroup(KSharedConfig::openConfig(configFile), "Formats");

    QString lang = formatsConfig.readEntry("LANG", QString());
    if (lang.isEmpty() || !m_installedTranslations.contains(lang)) {
        lang = QLocale::system().name();
    }
    if (!m_installedTranslations.contains(lang)) {
        lang = QStringLiteral("en_US");
    }

    // Use the available lang as the default
    m_kcmTranslations.clear();
    m_kcmTranslations.append(lang);

    // The update all the widgets to use the new language
    initWidgets();
}

void KCMTranslations::loadTranslations()
{
    // Load the user translations
    m_configTranslations = m_config.readEntry(lcLanguage, QString());
    m_kcmTranslations.clear();
    m_kcmTranslations = m_configTranslations.split(':', QString::SkipEmptyParts);
}

QString KCMTranslations::quickHelp() const
{
    return i18n("<h1>Translations</h1>\n"
                "<p>Here you can set your preferred language for translating the "
                "user interface of your applications. You can choose a single "
                "language, or a list of languages to be applied in sequence. Only "
                "language translations that are installed on your system will "
                "be listed as available. If your language is not listed then "
                "you will need to install it first.</p>");
}

void KCMTranslations::initWidgets()
{
    initTranslations();
    initTranslationsInstall();
    // Init the KCM "Apply" button
    emit changed(m_kcmTranslations.join(":") != m_configTranslations);
}

void KCMTranslations::initTranslations()
{
    m_ui->m_selectTranslations->blockSignals(true);

    m_ui->m_selectTranslations->setAvailableLabel(i18n("Available Languages:"));
    QString availableHelp = i18n("This is the list of installed KDE Plasma language "
                                 "translations not currently being used. <br />To use a language "
                                 "translation move it to the 'Preferred Languages' list in "
                                 "the order of preference.  <br />If no suitable languages are "
                                 "listed, then you may need to install more language packages "
                                 "using your usual installation method.");
    m_ui->m_selectTranslations->availableListWidget()->setToolTip(availableHelp);
    m_ui->m_selectTranslations->availableListWidget()->setWhatsThis(availableHelp);

    m_ui->m_selectTranslations->setSelectedLabel(i18n("Preferred Languages:"));
    QString selectedHelp = i18n("This is the list of installed KDE Plasma language "
                                "translations currently being used, listed in order of "
                                "preference.  <br />If a translation is not available for the "
                                "first language in the list, the next language will be used. <br /> "
                                "If no other translations are available then US English will "
                                "be used.");
    m_ui->m_selectTranslations->selectedListWidget()->setToolTip(selectedHelp);
    m_ui->m_selectTranslations->selectedListWidget()->setWhatsThis(selectedHelp);

    // Clear the selector before reloading
    m_ui->m_selectTranslations->availableListWidget()->clear();
    m_ui->m_selectTranslations->selectedListWidget()->clear();

    // Load each user selected language into the selected list
    foreach (const QString &languageCode, m_kcmTranslations) {
        QListWidgetItem *listItem = new QListWidgetItem(m_ui->m_selectTranslations->selectedListWidget());
        // TODO This gives the name in the language itself, not in current language, need new QLocale api for that
        QString label = QLocale(languageCode).nativeLanguageName();
        if (label.isEmpty()) {
            label = languageCode;
        }
        listItem->setText(label);
        listItem->setData(Qt::UserRole, languageCode);
    }

    // Load all the available languages the user hasn't selected into the available list
    foreach (const QString &languageCode, m_installedTranslations) {
        if (!m_kcmTranslations.contains(languageCode)) {
            QListWidgetItem *listItem = new QListWidgetItem(m_ui->m_selectTranslations->availableListWidget());
            // TODO This gives the name in the language itself, not in current language, need new QLocale api for that
            QString label = QLocale(languageCode).nativeLanguageName();
            if (label.isEmpty()) {
                label = languageCode;
            }
            listItem->setText(label);
            listItem->setData(Qt::UserRole, languageCode);
        }
    }
    m_ui->m_selectTranslations->availableListWidget()->sortItems();

    // Default to selecting the first Selected language,
    // otherwise the first Available language,
    // otherwise no languages so disable all buttons
    if (m_ui->m_selectTranslations->selectedListWidget()->count() > 0) {
        m_ui->m_selectTranslations->selectedListWidget()->setCurrentRow(0);
    } else if (m_ui->m_selectTranslations->availableListWidget()->count() > 0) {
        m_ui->m_selectTranslations->availableListWidget()->setCurrentRow(0);
    }

    // If the setting is locked down by Kiosk, then don't let the user make any changes
    if (m_config.isEntryImmutable(lcLanguage)) {
        m_ui->m_selectTranslations->setEnabled(false);
    }

    m_ui->m_selectTranslations->blockSignals(false);
}

void KCMTranslations::changedTranslationsAvailable(QListWidgetItem *item)
{
    Q_UNUSED(item);
    m_ui->m_selectTranslations->availableListWidget()->sortItems();
    int row = m_ui->m_selectTranslations->availableListWidget()->currentRow();
    changedTranslations();
    m_ui->m_selectTranslations->availableListWidget()->setCurrentRow(row);
}

void KCMTranslations::changedTranslationsSelected(QListWidgetItem *item)
{
    Q_UNUSED(item);
    int row = m_ui->m_selectTranslations->selectedListWidget()->currentRow();
    changedTranslations();
    m_ui->m_selectTranslations->selectedListWidget()->setCurrentRow(row);
}

void KCMTranslations::changedTranslations()
{
    // Read the list of all Selected translations from the selector widget
    m_kcmTranslations.clear();
    for (int i = 0; i < m_ui->m_selectTranslations->selectedListWidget()->count(); ++i) {
        m_kcmTranslations.append(m_ui->m_selectTranslations->selectedListWidget()->item(i)->data(Qt::UserRole).toString());
    }
    initWidgets();
}

void KCMTranslations::initTranslationsInstall()
{
    m_ui->m_buttonTranslationsInstall->blockSignals(true);
    m_ui->m_buttonTranslationsInstall->setText(i18n("Install more languages"));
    QString helpText = i18n("<p>Click here to install more languages</p>");
    m_ui->m_buttonTranslationsInstall->setToolTip(helpText);
    m_ui->m_buttonTranslationsInstall->setWhatsThis(helpText);
    m_ui->m_buttonTranslationsInstall->blockSignals(false);
}

void KCMTranslations::installTranslations()
{
    // User has clicked Install Languages button, trigger distro specific install routine
}

#include "kcmtranslations.moc"
