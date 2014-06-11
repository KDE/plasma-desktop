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

#include <KAboutData>
#include <KActionSelector>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <kglobalsettings.h>

#include <QListWidget>
#include <QListWidgetItem>
#include <QStandardPaths>

#include "ui_kcmtranslationswidget.h"

K_PLUGIN_FACTORY(KCMTranslationsFactory, registerPlugin<KCMTranslations>();)

KCMTranslations::KCMTranslations(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args),
      m_ui(new Ui::KCMTranslationsWidget)
{
    KAboutData *about = new KAboutData(I18N_NOOP("KCMTranslations"),
                                       i18n("Translations"),
                                       QString(),
                                       i18n("Configure Plasma translations"),
                                       KAboutLicense::GPL);
    about->addAuthor(ki18n("John Layt").toString(), ki18n("Maintainer").toString(), QStringLiteral("jlayt@kde.net"));
    setAboutData(about);

    m_ui->setupUi(this);

    // Set the translation domain to Plasma, i.e.
    KLocalizedString::setApplicationDomain("plasma");

    // Get the current config
    m_config = KConfigGroup(KSharedConfig::openConfig("plasma-localerc"), "Translations");

    // Set up UI to respond to user changing the translation selection
    connect(m_ui->m_selectTranslations,         SIGNAL(added( QListWidgetItem*)),
            this,                               SLOT(changedTranslationsSelected(QListWidgetItem*)));
    connect(m_ui->m_selectTranslations,         SIGNAL(removed( QListWidgetItem*)),
            this,                               SLOT(changedTranslationsAvailable(QListWidgetItem*)));
    connect(m_ui->m_selectTranslations,         SIGNAL(movedUp( QListWidgetItem*)),
            this,                               SLOT(changedTranslationsSelected(QListWidgetItem*)));
    connect(m_ui->m_selectTranslations,         SIGNAL(movedDown( QListWidgetItem*)),
            this,                               SLOT(changedTranslationsSelected(QListWidgetItem*)));

    // If user clicks the Install button, trigger distro specific install routine
    connect(m_ui->m_buttonTranslationsInstall,  SIGNAL(clicked()),
            this,                               SLOT(installTranslations()));

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
    m_installedTranslations = KLocalizedString::availableApplicationTranslations("systemsettings").toList();
    if (!m_installedTranslations.contains("en_US")) {
        m_installedTranslations.append("en_US");
    }

    // Load the current user translations
    loadTranslations();

    // Check if any of the current user translations are no longer installed
    // If any missing remove them and save the settings, then tell the user
    QStringList missingLanguages;
    QStringList availableLanguages;
    if (!m_config.isEntryImmutable("LANGUAGE")) {
        foreach (const QString &languageCode, m_kcmTranslations) {
            if (m_installedTranslations.contains(languageCode)) {
                availableLanguages.append(languageCode);
            } else {
                missingLanguages.append(languageCode);
            }
        }
        if (!missingLanguages.isEmpty()) {
            m_config.writeEntry("LANGUAGE", availableLanguages.join(":"), KConfig::Persistent | KConfig::Global);
            m_config.sync();
            m_config.config()->reparseConfiguration();
            loadTranslations();
        }
    }

    // Then update all the widgets to use the new settings
    initWidgets();

    // Now we have a ui built tell the user about the missing languages
    foreach (const QString &languageCode, missingLanguages) {
        KMessageBox::information(this, ki18n("You have the language with code '%1' in your list "
                                             "of languages to use for translation but the "
                                             "localization files for it could not be found. The "
                                             "language has been removed from your configuration. "
                                             "If you want to add it again please install the "
                                             "localization files for it and add the language again.")
                                             .subs(languageCode).toString());
    }
}

// Reimp
// Save the new LANGUAGE setting
void KCMTranslations::save()
{
    m_config.writeEntry("LANGUAGE", m_kcmTranslations.join(":"), KConfig::Persistent | KConfig::Global);
    m_config.sync();
    KMessageBox::information(this,
                             i18n("Your changes will take effect the next time you log in."),
                             i18n("Applying Language Settings"),
                             QLatin1String("LanguageChangesApplyOnlyToNewlyStartedPrograms"));
    load();
    KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_LOCALE);
}

// Reimp
// Defaults == User has clicked on Defaults to load default settings
// We interpret this to mean the default LANG setting
void KCMTranslations::defaults()
{
    // Get the users LANG setting, or if empty the system LANG
    QString lang = m_config.readEntry("LANG", QString());
    if (lang.isEmpty() || !m_installedTranslations.contains(lang))
        lang = QLocale::system().name();
    if (!m_installedTranslations.contains(lang))
        lang = QStringLiteral("en_US");

    // Use the available lang as the default
    m_kcmTranslations.clear();
    m_kcmTranslations.append(lang);

    // The update all the widgets to use the new language
    initWidgets();
}

void KCMTranslations::loadTranslations()
{
    // Load the user translations
    m_configTranslations = m_config.readEntry("LANGUAGE", QString());
    m_kcmTranslations.clear();
    m_kcmTranslations = m_configTranslations.split(':', QString::SkipEmptyParts);
}

QString KCMTranslations::quickHelp() const
{
    return ki18n("<h1>Translations</h1>\n"
                 "<p>Here you can set your preferred language for translating the "
                 "user interface of your applications. You can choose a single "
                 "language, or a list of languages to be applied in sequence. Only "
                 "language translations that are installed on your system will "
                 "be listed as available. If your language is not listed then "
                 "you will need to install it first.</p>").toString();
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

    m_ui->m_selectTranslations->setAvailableLabel(ki18n( "Available Languages:" ).toString());
    QString availableHelp = ki18n("<p>This is the list of installed KDE Plasma language "
                                  "translations not currently being used.  To use a language "
                                  "translation move it to the 'Preferred Languages' list in "
                                  "the order of preference.  If no suitable languages are "
                                  "listed, then you may need to install more language packages "
                                  "using your usual installation method.</p>" ).toString();
    m_ui->m_selectTranslations->availableListWidget()->setToolTip(availableHelp);
    m_ui->m_selectTranslations->availableListWidget()->setWhatsThis(availableHelp);

    m_ui->m_selectTranslations->setSelectedLabel(ki18n("Preferred Languages:").toString());
    QString selectedHelp = ki18n("<p>This is the list of installed KDE Plasma language "
                                 "translations currently being used, listed in order of "
                                 "preference.  If a translation is not available for the "
                                 "first language in the list, the next language will be used.  "
                                 "If no other translations are available then US English will "
                                 "be used.</p>").toString();
    m_ui->m_selectTranslations->selectedListWidget()->setToolTip(selectedHelp);
    m_ui->m_selectTranslations->selectedListWidget()->setWhatsThis(selectedHelp);

    QString enUS;
    QString defaultLang = ki18nc("%1 = default language name", "%1 (Default)").subs(enUS).toString();

    // Clear the selector before reloading
    m_ui->m_selectTranslations->availableListWidget()->clear();
    m_ui->m_selectTranslations->selectedListWidget()->clear();

    // Load each user selected language into the selected list
    foreach (const QString &languageCode, m_kcmTranslations) {
        QListWidgetItem *listItem = new QListWidgetItem(m_ui->m_selectTranslations->selectedListWidget());
        // TODO This gives the name in the language itself, not in current language, need new QLocale api for that
        listItem->setText(QLocale(languageCode).nativeLanguageName());
        listItem->setData(Qt::UserRole, languageCode);
    }

    // Load all the available languages the user hasn't selected into the available list
    foreach ( const QString &languageCode, m_installedTranslations ) {
        if (!m_kcmTranslations.contains(languageCode)) {
            QListWidgetItem *listItem = new QListWidgetItem(m_ui->m_selectTranslations->availableListWidget());
            // TODO This gives the name in the language itself, not in current language, need new QLocale api for that
            listItem->setText(QLocale(languageCode).nativeLanguageName());
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
    if (m_config.isEntryImmutable("LANGUAGE")) {
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
    m_ui->m_buttonTranslationsInstall->setText( ki18n("Install more languages").toString());
    QString helpText = ki18n("<p>Click here to install more languages</p>").toString();
    m_ui->m_buttonTranslationsInstall->setToolTip(helpText);
    m_ui->m_buttonTranslationsInstall->setWhatsThis(helpText);
    m_ui->m_buttonTranslationsInstall->blockSignals(false);
}

void KCMTranslations::installTranslations()
{
    // User has clicked Install Languages button, trigger distro specific install routine
}

#include "kcmtranslations.moc"
