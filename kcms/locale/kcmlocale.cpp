/*
 *  Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
 *  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
 *  Copyright (c) 1999-2003 Hans Petter Bieker <bieker@kde.org>
 *  Copyright 2010 John Layt <john@layt.net>
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

#include "kcmlocale.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QPrinter>
#include <QTimer>

#include <KAboutData>
#include <KActionSelector>
#include <KBuildSycocaProgressDialog>
#include <KCalendarSystem>
#include <KDateTime>
#include <KDebug>
#include <KGlobal>
#include <KGlobalSettings>
#include <KIcon>
#include <KLocale>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KStandardDirs>
#include <KCurrencyCode>
#include <QStandardPaths>

#include "ui_kcmlocalewidget.h"

K_PLUGIN_FACTORY( KCMLocaleFactory, registerPlugin<KCMLocale>(); )

KCMLocale::KCMLocale( QWidget *parent, const QVariantList &args )
         : KCModule(parent, args ),
           m_userConfig( 0 ),
           m_kcmConfig( 0 ),
           m_currentConfig( 0 ),
           m_defaultConfig( 0 ),
           m_groupConfig( 0 ),
           m_countryConfig( 0 ),
           m_cConfig( 0 ),
           m_kcmLocale( 0 ),
           m_defaultLocale( 0 ),
           m_ui( new Ui::KCMLocaleWidget )
{
    KAboutData *about = new KAboutData( I18N_NOOP("kcmlocale"), QString(),
                                        i18n( "Localization options for KDE applications" ),
                                        QString(), QString(), KAboutData::License_GPL,
                                        i18n( "Copyright 2010 John Layt" ) );

//     KAboutData *about =
//         new KAboutData( I18N_NOOP("kcmstyle"), QString(),
//                         i18n("KDE Style Module"),
//                         QString(), QString(), KAboutData::License_GPL,
//                         i18n("(c) 2002 Karol Szwed, Daniel Molkentin"));
//

    about->addAuthor( ki18n( "John Layt" ).toString(), ki18n( "Maintainer" ).toString(), QStringLiteral("john@layt.net") );

    setAboutData( about );

    m_ui->setupUi( this );

    initSettings();

    // Country tab
    connect( m_ui->m_comboCountry,       SIGNAL( activated( int ) ),
             this,                       SLOT( changedCountryIndex( int ) ) );
    connect( m_ui->m_buttonDefaultCountry, SIGNAL( clicked() ),
             this,                       SLOT( defaultCountry() ) );

    connect( m_ui->m_comboCountryDivision,       SIGNAL( activated( int ) ),
             this,                               SLOT( changedCountryDivisionIndex( int ) ) );
    connect( m_ui->m_buttonDefaultCountryDivision, SIGNAL( clicked() ),
             this,                               SLOT( defaultCountryDivision() ) );

    // Translations tab

    // User has changed the translations selection in some way
    connect( m_ui->m_selectTranslations,         SIGNAL( added( QListWidgetItem* ) ),
             this,                               SLOT( changedTranslationsSelected( QListWidgetItem* ) ) );
    connect( m_ui->m_selectTranslations,         SIGNAL( removed( QListWidgetItem* ) ),
             this,                               SLOT( changedTranslationsAvailable( QListWidgetItem* ) ) );
    connect( m_ui->m_selectTranslations,         SIGNAL( movedUp( QListWidgetItem* ) ),
             this,                               SLOT( changedTranslationsSelected( QListWidgetItem* ) ) );
    connect( m_ui->m_selectTranslations,         SIGNAL( movedDown( QListWidgetItem* ) ),
             this,                               SLOT( changedTranslationsSelected( QListWidgetItem* ) ) );
    // User has clicked Install button, trigger distro specific install routine
    connect( m_ui->m_buttonTranslationsInstall,  SIGNAL( clicked() ),
             this,                               SLOT( installTranslations() ) );
    // Hide the Install button, this will be activated by those distros that support this feature.
    m_ui->m_buttonTranslationsInstall->setHidden( true );
    // User has clicked Default button, resest lists to Defaults
    connect( m_ui->m_buttonDefaultTranslations,    SIGNAL( clicked() ),
             this,                               SLOT( defaultTranslations() ) );

    // Numbers tab

    connect( m_ui->m_comboNumericDigitGrouping,         SIGNAL( currentIndexChanged( int ) ),
             this,                                      SLOT(   changedNumericDigitGroupingIndex( int ) ) );
    connect( m_ui->m_buttonDefaultNumericDigitGrouping, SIGNAL( clicked() ),
             this,                                      SLOT(   defaultNumericDigitGrouping() ) );

    connect( m_ui->m_comboThousandsSeparator,       SIGNAL( editTextChanged( const QString & ) ),
             this,                                  SLOT( changedNumericThousandsSeparator( const QString & ) ) );
    connect( m_ui->m_buttonDefaultThousandsSeparator, SIGNAL( clicked() ),
             this,                                  SLOT( defaultNumericThousandsSeparator() ) );

    connect( m_ui->m_comboDecimalSymbol,       SIGNAL( editTextChanged( const QString & ) ),
             this,                             SLOT( changedNumericDecimalSymbol( const QString & ) ) );
    connect( m_ui->m_buttonDefaultDecimalSymbol, SIGNAL( clicked() ),
             this,                             SLOT( defaultNumericDecimalSymbol() ) );

    connect( m_ui->m_intDecimalPlaces,         SIGNAL( valueChanged( int ) ),
             this,                             SLOT( changedNumericDecimalPlaces( int ) ) );
    connect( m_ui->m_buttonDefaultDecimalPlaces, SIGNAL( clicked() ),
             this,                             SLOT( defaultNumericDecimalPlaces() ) );

    connect( m_ui->m_comboPositiveSign,       SIGNAL( editTextChanged( const QString & ) ),
             this,                            SLOT( changedNumericPositiveSign( const QString & ) ) );
    connect( m_ui->m_buttonDefaultPositiveSign, SIGNAL( clicked() ),
             this,                            SLOT( defaultNumericPositiveSign() ) );

    connect( m_ui->m_comboNegativeSign,       SIGNAL( editTextChanged( const QString & ) ),
             this,                            SLOT( changedNumericNegativeSign( const QString & ) ) );
    connect( m_ui->m_buttonDefaultNegativeSign, SIGNAL( clicked() ),
             this,                            SLOT( defaultNumericNegativeSign() ) );

    connect( m_ui->m_comboDigitSet,       SIGNAL( currentIndexChanged( int ) ),
             this,                        SLOT( changedNumericDigitSetIndex( int ) ) );
    connect( m_ui->m_buttonDefaultDigitSet, SIGNAL( clicked() ),
             this,                        SLOT( defaultNumericDigitSet() ) );

    // Money tab

    connect( m_ui->m_comboCurrencyCode,       SIGNAL( currentIndexChanged( int ) ),
             this,                              SLOT( changedCurrencyCodeIndex( int ) ) );
    connect( m_ui->m_buttonDefaultCurrencyCode, SIGNAL( clicked() ),
             this,                              SLOT( defaultCurrencyCode() ) );

    connect( m_ui->m_comboCurrencySymbol,       SIGNAL( currentIndexChanged( int ) ),
             this,                              SLOT( changedCurrencySymbolIndex( int ) ) );
    connect( m_ui->m_buttonDefaultCurrencySymbol, SIGNAL( clicked() ),
             this,                              SLOT( defaultCurrencySymbol() ) );

    connect( m_ui->m_comboMonetaryDigitGrouping,         SIGNAL( currentIndexChanged( int ) ),
             this,                                       SLOT(   changedMonetaryDigitGroupingIndex( int ) ) );
    connect( m_ui->m_buttonDefaultMonetaryDigitGrouping, SIGNAL( clicked() ),
             this,                                       SLOT(   defaultMonetaryDigitGrouping() ) );

    connect( m_ui->m_comboMonetaryThousandsSeparator,       SIGNAL( editTextChanged( const QString & ) ),
             this,                                          SLOT( changedMonetaryThousandsSeparator( const QString & ) ) );
    connect( m_ui->m_buttonDefaultMonetaryThousandsSeparator, SIGNAL( clicked() ),
             this,                                          SLOT( defaultMonetaryThousandsSeparator() ) );

    connect( m_ui->m_comboMonetaryDecimalSymbol,       SIGNAL( editTextChanged( const QString & ) ),
             this,                                     SLOT( changedMonetaryDecimalSymbol( const QString & ) ) );
    connect( m_ui->m_buttonDefaultMonetaryDecimalSymbol, SIGNAL( clicked() ),
             this,                                     SLOT( defaultMonetaryDecimalSymbol() ) );

    connect( m_ui->m_intMonetaryDecimalPlaces,         SIGNAL( valueChanged( int ) ),
             this,                                     SLOT( changedMonetaryDecimalPlaces( int ) ) );
    connect( m_ui->m_buttonDefaultMonetaryDecimalPlaces, SIGNAL( clicked() ),
             this,                                     SLOT( defaultMonetaryDecimalPlaces() ) );

    connect( m_ui->m_comboMonetaryPositiveFormat,       SIGNAL( currentIndexChanged( int ) ),
             this,                                      SLOT( changedMonetaryPositiveFormatIndex( int ) ) );
    connect( m_ui->m_buttonDefaultMonetaryPositiveFormat, SIGNAL( clicked() ),
             this,                                      SLOT( defaultMonetaryPositiveFormat() ) );

    connect( m_ui->m_comboMonetaryNegativeFormat,       SIGNAL( currentIndexChanged( int ) ),
             this,                                      SLOT( changedMonetaryNegativeFormatIndex( int ) ) );
    connect( m_ui->m_buttonDefaultMonetaryNegativeFormat, SIGNAL( clicked() ),
             this,                                      SLOT( defaultMonetaryNegativeFormat() ) );

    connect( m_ui->m_comboMonetaryDigitSet,       SIGNAL( currentIndexChanged( int ) ),
             this,                                SLOT( changedMonetaryDigitSetIndex( int ) ) );
    connect( m_ui->m_buttonDefaultMonetaryDigitSet, SIGNAL( clicked() ),
             this,                                SLOT( defaultMonetaryDigitSet() ) );

    // Calendar tab

    connect( m_ui->m_comboCalendarSystem,       SIGNAL( currentIndexChanged( int ) ),
             this,                              SLOT( changedCalendarSystemIndex( int ) ) );
    connect( m_ui->m_buttonDefaultCalendarSystem, SIGNAL( clicked() ),
             this,                              SLOT( defaultCalendarSystem() ) );

    connect( m_ui->m_checkCalendarGregorianUseCommonEra,       SIGNAL( clicked( bool ) ),
             this,                                             SLOT( changedUseCommonEra( bool ) ) );
    connect( m_ui->m_buttonDefaultCalendarGregorianUseCommonEra, SIGNAL( clicked() ),
             this,                                             SLOT( defaultUseCommonEra() ) );

    connect( m_ui->m_intShortYearWindowStartYear, SIGNAL( valueChanged( int ) ),
             this,                                SLOT( changedShortYearWindow( int ) ) );
    connect( m_ui->m_buttonDefaultShortYearWindow,  SIGNAL( clicked() ),
             this,                                SLOT( defaultShortYearWindow() ) );

    connect( m_ui->m_comboWeekNumberSystem,         SIGNAL( currentIndexChanged( int ) ),
             this,                                  SLOT(   changedWeekNumberSystemIndex( int ) ) );
    connect( m_ui->m_buttonDefaultWeekNumberSystem, SIGNAL( clicked() ),
             this,                                  SLOT(   defaultWeekNumberSystem() ) );

    connect( m_ui->m_comboWeekStartDay,       SIGNAL( currentIndexChanged( int ) ),
             this,                            SLOT( changedWeekStartDayIndex( int ) ) );
    connect( m_ui->m_buttonDefaultWeekStartDay, SIGNAL( clicked() ),
             this,                            SLOT( defaultWeekStartDay() ) );

    connect( m_ui->m_comboWorkingWeekStartDay,       SIGNAL( currentIndexChanged( int ) ),
             this,                                   SLOT( changedWorkingWeekStartDayIndex( int ) ) );
    connect( m_ui->m_buttonDefaultWorkingWeekStartDay, SIGNAL( clicked() ),
             this,                                   SLOT( defaultWorkingWeekStartDay() ) );

    connect( m_ui->m_comboWorkingWeekEndDay,       SIGNAL( currentIndexChanged( int ) ),
             this,                                 SLOT( changedWorkingWeekEndDayIndex( int ) ) );
    connect( m_ui->m_buttonDefaultWorkingWeekEndDay, SIGNAL( clicked() ),
             this,                                 SLOT( defaultWorkingWeekEndDay() ) );

    connect( m_ui->m_comboWeekDayOfPray,       SIGNAL( currentIndexChanged( int ) ),
             this,                             SLOT( changedWeekDayOfPrayIndex( int ) ) );
    connect( m_ui->m_buttonDefaultWeekDayOfPray, SIGNAL( clicked() ),
             this,                             SLOT( defaultWeekDayOfPray() ) );

    // Date / Time tab

    connect( m_ui->m_comboTimeFormat,       SIGNAL( editTextChanged( const QString & ) ),
             this,                          SLOT( changedTimeFormat( const QString & ) ) );
    connect( m_ui->m_buttonDefaultTimeFormat, SIGNAL( clicked() ),
             this,                          SLOT( defaultTimeFormat() ) );

    connect( m_ui->m_comboAmSymbol,       SIGNAL( editTextChanged( const QString & ) ),
             this,                        SLOT( changedAmSymbol( const QString & ) ) );
    connect( m_ui->m_buttonDefaultAmSymbol, SIGNAL( clicked() ),
             this,                        SLOT( defaultAmSymbol() ) );

    connect( m_ui->m_comboPmSymbol,       SIGNAL( editTextChanged( const QString & ) ),
             this,                        SLOT( changedPmSymbol( const QString & ) ) );
    connect( m_ui->m_buttonDefaultPmSymbol, SIGNAL( clicked() ),
             this,                        SLOT( defaultPmSymbol() ) );

    connect( m_ui->m_comboDateFormat,       SIGNAL( editTextChanged( const QString & ) ),
             this,                          SLOT( changedDateFormat( const QString & ) ) );
    connect( m_ui->m_buttonDefaultDateFormat, SIGNAL( clicked() ),
             this,                          SLOT( defaultDateFormat() ) );

    connect( m_ui->m_comboShortDateFormat,       SIGNAL( editTextChanged( const QString & ) ),
             this,                               SLOT( changedShortDateFormat( const QString & ) ) );
    connect( m_ui->m_buttonDefaultShortDateFormat, SIGNAL( clicked() ),
             this,                               SLOT( defaultShortDateFormat() ) );

    connect( m_ui->m_checkMonthNamePossessive,       SIGNAL( clicked( bool ) ),
             this,                                   SLOT( changedMonthNamePossessive( bool ) ) );
    connect( m_ui->m_buttonDefaultMonthNamePossessive, SIGNAL( clicked() ),
             this,                                   SLOT( defaultMonthNamePossessive() ) );

    connect( m_ui->m_comboDateTimeDigitSet,       SIGNAL( currentIndexChanged( int ) ),
             this,                                SLOT( changedDateTimeDigitSetIndex( int ) ) );
    connect( m_ui->m_buttonDefaultDateTimeDigitSet, SIGNAL( clicked() ),
             this,                                SLOT( defaultDateTimeDigitSet() ) );

    // Other tab

    connect( m_ui->m_comboPageSize,       SIGNAL( currentIndexChanged( int ) ),
             this,                        SLOT( changedPageSizeIndex( int ) ) );
    connect( m_ui->m_buttonDefaultPageSize, SIGNAL( clicked() ),
             this,                        SLOT( defaultPageSize() ) );

    connect( m_ui->m_comboMeasureSystem,       SIGNAL( currentIndexChanged( int ) ),
             this,                             SLOT( changedMeasureSystemIndex( int ) ) );
    connect( m_ui->m_buttonDefaultMeasureSystem, SIGNAL( clicked() ),
             this,                             SLOT( defaultMeasureSystem() ) );

    connect( m_ui->m_comboBinaryUnitDialect,       SIGNAL( currentIndexChanged( int ) ),
             this,                                 SLOT( changedBinaryUnitDialectIndex( int ) ) );
    connect( m_ui->m_buttonDefaultBinaryUnitDialect, SIGNAL( clicked() ),
             this,                                 SLOT( defaultBinaryUnitDialect() ) );
}

KCMLocale::~KCMLocale()
{
    // Throw away any unsaved changes as delete calls an unwanted sync()
    m_kcmConfig->markAsClean();
    m_userConfig->markAsClean();
    m_defaultConfig->markAsClean();
    m_cConfig->markAsClean();
    m_countryConfig->markAsClean();
    m_groupConfig->markAsClean();
    delete m_kcmLocale;
    delete m_defaultLocale;
    delete m_ui;
}

// Init all the config/settings objects, only called at the very start but inits everything so
// load() and mergeSettings() can assume everything exists every time they are called
void KCMLocale::initSettings()
{
    // Setup the KCM Config/Settings
    // These are the effective settings merging KCM Changes, User, Group, Country, and C settings
    // This will be used to display current state of settings in the KCM
    // These settings should never be saved anywhere
    m_kcmConfig = KSharedConfig::openConfig( "kcmlocale-kcm", KConfig::SimpleConfig );
    m_kcmSettings = KConfigGroup( m_kcmConfig, "Locale" );
    m_kcmSettings.deleteGroup();
    m_kcmSettings.markAsClean();

    // Setup the Default Config/Settings
    // These will be a merge of the C, Country and Group settings
    // If the user clicks on the Defaults button, these are the settings that will be used
    // These settings should never be saved anywhere
    m_defaultConfig = KSharedConfig::openConfig( "kcmlocale-default", KConfig::SimpleConfig );
    m_defaultSettings = KConfigGroup( m_defaultConfig, "Locale" );

    // Setup the User Config/Settings
    // These are the User overrides, they exclude any Group, Country, or C settings
    // This will be used to store the User changes
    // These are the only settings that should ever be saved
    m_userConfig = KSharedConfig::openConfig( "kcmlocale-user", KConfig::IncludeGlobals );
    m_userSettings = KConfigGroup( m_userConfig, "Locale" );

    // Setup the Current Config/Settings
    // These are the currently saved User settings
    // This will be used to check if the kcm settings have been changed
    // These settings should never be saved anywhere
    m_currentConfig = KSharedConfig::openConfig( "kcmlocale-current", KConfig::IncludeGlobals );
    m_currentSettings = KConfigGroup( m_currentConfig, "Locale" );

    // Setup the Group Config/Settings
    // These are the Group overrides, they exclude any User, Country, or C settings
    // This will be used in the merge to obtain the KCM Defaults
    // These settings should never be saved anywhere
    m_groupConfig = KSharedConfig::openConfig( "kcmlocale-group", KConfig::NoGlobals );
    m_groupSettings = KConfigGroup( m_groupConfig, "Locale" );

    // Setup the C Config Settings
    // These are the C/Posix defaults and KDE defaults where a setting doesn't exist in Posix
    // This will be used as the lowest level in the merge to obtain the KCM Defaults
    // These settings should never be saved anywhere
    m_cConfig = KSharedConfig::openConfig( KStandardDirs::locate( "locale",
                                           QString::fromLatin1("l10n/C/kf5_entry.desktop") ) );
    m_cSettings= KConfigGroup( m_cConfig, "KCM Locale" );

    initCountrySettings( KGlobal::locale()->country() );

    initCalendarSettings();

    m_kcmLocale = new KLocale( m_kcmConfig );
    m_defaultLocale = new KLocale( m_defaultConfig );

    // Find out the system country using a null config
    m_systemCountry = m_kcmLocale->country();

    // Set up the initial languages to use
    m_currentTranslations = m_userSettings.readEntry( "Language", QString() );
    m_kcmTranslations = m_currentTranslations.split( ':', QString::SkipEmptyParts );
}

void KCMLocale::initCountrySettings( const QString &countryCode )
{
    // Setup a the Country Config/Settings
    // These are the Country overrides, they exclude any User, Group, or C settings
    // This will be used in the merge to obtain the KCM Defaults
    // These settings should never be saved anywhere
    m_countryConfig = KSharedConfig::openConfig( KStandardDirs::locate( "locale",
                                                 QString::fromLatin1("l10n/%1/kf5_entry.desktop")
                                                 .arg( countryCode ) ) );
    m_countrySettings = KConfigGroup( m_countryConfig, "KCM Locale" );
    QString calendarType = m_countrySettings.readEntry( "CalendarSystem", "gregorian" );
    QString calendarGroup = QString::fromLatin1( "KCalendarSystem %1" ).arg( calendarType );
    m_countryCalendarSettings = m_countrySettings.group( calendarGroup );
}

// Init all the calendar settings sub-group objects, called both at the start and whenever the
// calendar system is changed
void KCMLocale::initCalendarSettings()
{
    // Setup the User Config/Settings
    // These are the User overrides, they exclude any Group, Country, or C settings
    // This will be used to store the User changes
    QString calendarType = m_kcmSettings.readEntry( "CalendarSystem", QString() );
    QString calendarGroup = QString::fromLatin1( "KCalendarSystem %1" ).arg( calendarType );
    m_userCalendarSettings = m_userSettings.group( calendarGroup );

    // Setup the Current Config/Settings
    // These are the currently saved User settings
    // This will be used to check if the kcm settings have been changed
    calendarType = m_currentSettings.readEntry( "CalendarSystem", KGlobal::locale()->calendar()->calendarType() );
    calendarGroup = QString::fromLatin1( "KCalendarSystem %1" ).arg( calendarType );
    m_currentCalendarSettings = m_currentSettings.group( calendarGroup );

    // Setup the Group Config/Settings
    // These are the Group overrides, they exclude any User, Country, or C settings
    // This will be used in the merge to obtain the KCM Defaults
    // These settings should never be saved anywhere
    calendarType = m_groupSettings.readEntry( "CalendarSystem", KGlobal::locale()->calendar()->calendarType() );
    calendarGroup = QString::fromLatin1( "KCalendarSystem %1" ).arg( calendarType );
    m_groupCalendarSettings = m_groupSettings.group( calendarGroup );

    // Setup the C Config Settings
    // These are the C/Posix defaults and KDE defaults where a setting doesn't exist in Posix
    // This will be used as the lowest level in the merge to obtain the KCM Defaults
    // These settings should never be saved anywhere
    calendarType = m_cSettings.readEntry( "CalendarSystem", QString() );
    calendarGroup = QString::fromLatin1( "KCalendarSystem %1" ).arg( calendarType );
    m_cCalendarSettings = m_cSettings.group( calendarGroup );

    // Setup a the Country Config/Settings
    // These are the Country overrides, they exclude any User, Group, or C settings
    // This will be used in the merge to obtain the KCM Defaults
    // These settings should never be saved anywhere
    calendarType = m_countrySettings.readEntry( "CalendarSystem", "gregorian" );
    calendarGroup = QString::fromLatin1( "KCalendarSystem %1" ).arg( calendarType );
    m_countryCalendarSettings = m_countrySettings.group( calendarGroup );
}

// Load == User has clicked on Reset to restore load saved settings
// Also gets called automatically called after constructor
void KCMLocale::load()
{
    // Throw away any unsaved changes then reload from file
    m_userConfig->markAsClean();
    m_userConfig->reparseConfiguration();

    // Get the currently installed translations
    m_installedTranslations.clear();
    m_installedTranslations = m_kcmLocale->installedLanguages();

    // Check if any of the user requested translations are no longer installed
    // If any missing remove them and save the settings, we'll tell the user later
    m_kcmTranslations.clear();
    QStringList missingLanguages;
    QStringList userTranslations = m_userSettings.readEntry( "Language", QString() ).split( ':', QString::SkipEmptyParts );
    foreach ( const QString &languageCode, userTranslations ) {
        if ( m_installedTranslations.contains( languageCode ) ) {
            m_kcmTranslations.append( languageCode );
        } else {
            missingLanguages.append( languageCode );
        }
    }
    if (!missingLanguages.isEmpty()) {
        m_userSettings.writeEntry( "Language", m_kcmTranslations.join( ":" ), KConfig::Persistent | KConfig::Global );
        m_userConfig->sync();
    }

    // Now load the new current settings
    m_currentConfig->reparseConfiguration();
    m_currentTranslations = m_userSettings.readEntry( "Language", QString() );

    // Then create the new settings using the default, include user settings
    mergeSettings();

    // The update all the widgets to use the new settings
    initAllWidgets();

    // Now we have a ui built tell the user about the missing languages
    foreach ( const QString &languageCode, missingLanguages ) {
        KMessageBox::information(this, ki18n("You have the language with code '%1' in your list "
                                             "of languages to use for translation but the "
                                             "localization files for it could not be found. The "
                                             "language has been removed from your configuration. "
                                             "If you want to add it again please install the "
                                             "localization files for it and add the language again.")
                                             .subs( languageCode ).toString() );
    }
}

// Defaults == User has clicked on Defaults to load default settings
// We interpret this to mean the defaults for the system country and language
void KCMLocale::defaults()
{
    // Clear out the user config but don't sync or reparse as we want to ignore the user settings
    m_userCalendarSettings.deleteGroup( KConfig::Persistent | KConfig::Global );
    m_userSettings.deleteGroup( KConfig::Persistent | KConfig::Global );
    m_kcmTranslations.clear();
    m_currentTranslations = QString();

    // Reload the system country
    initCountrySettings( m_systemCountry );

    // Then create the new settings using the default, exclude user settings
    mergeSettings();

    // Save the current translations for checking later
    m_currentTranslations = m_kcmSettings.readEntry( "Language", QString() );

    // The update all the widgets to use the new settings
    initAllWidgets();
}

// Write our own copy routine as we need to be selective about what values get copied, e.g. exclude
// Name, Region, etc, and copyTo() seems to barf on some uses
void KCMLocale::copySettings( KConfigGroup *fromGroup, KConfigGroup *toGroup, KConfig::WriteConfigFlags flags )
{
    copySetting( fromGroup, toGroup, "Country", flags );
    copySetting( fromGroup, toGroup, "CountryDivision", flags );
    copySetting( fromGroup, toGroup, "Language", flags );
    copySetting( fromGroup, toGroup, "DecimalPlaces", flags );
    copySetting( fromGroup, toGroup, "DecimalSymbol", flags );
    copySetting( fromGroup, toGroup, "DigitGroupFormat", flags );
    copySetting( fromGroup, toGroup, "ThousandsSeparator", flags );
    copySetting( fromGroup, toGroup, "PositiveSign", flags );
    copySetting( fromGroup, toGroup, "NegativeSign", flags );
    copySetting( fromGroup, toGroup, "DigitSet", flags );
    copySetting( fromGroup, toGroup, "CurrencyCode", flags );
    copySetting( fromGroup, toGroup, "CurrencySymbol", flags );
    copySetting( fromGroup, toGroup, "MonetaryDecimalPlaces", flags );
    copySetting( fromGroup, toGroup, "MonetaryDecimalSymbol", flags );
    copySetting( fromGroup, toGroup, "MonetaryDigitGroupFormat", flags );
    copySetting( fromGroup, toGroup, "MonetaryThousandsSeparator", flags );
    copySetting( fromGroup, toGroup, "PositivePrefixCurrencySymbol", flags );
    copySetting( fromGroup, toGroup, "NegativePrefixCurrencySymbol", flags );
    copySetting( fromGroup, toGroup, "PositiveMonetarySignPosition", flags );
    copySetting( fromGroup, toGroup, "NegativeMonetarySignPosition", flags );
    copySetting( fromGroup, toGroup, "MonetaryDigitSet", flags );
    copySetting( fromGroup, toGroup, "CalendarSystem", flags );
    copySetting( fromGroup, toGroup, "TimeFormat", flags );
    QString eraKey = QString::fromLatin1("DayPeriod1");
    int i = 1;
    while ( fromGroup->hasKey( eraKey ) ) {
        copySetting( fromGroup, toGroup, eraKey, flags );
        ++i;
        eraKey = QString::fromLatin1("DayPeriod%1").arg( i );
    }
    copySetting( fromGroup, toGroup, "DateFormat", flags );
    copySetting( fromGroup, toGroup, "DateFormatShort", flags );
    copySetting( fromGroup, toGroup, "DateMonthNamePossessive", flags );
    copySetting( fromGroup, toGroup, "WeekNumberSystem", flags );
    copySetting( fromGroup, toGroup, "WeekStartDay", flags );
    copySetting( fromGroup, toGroup, "WorkingWeekStartDay", flags );
    copySetting( fromGroup, toGroup, "WorkingWeekEndDay", flags );
    copySetting( fromGroup, toGroup, "WeekDayOfPray", flags );
    copySetting( fromGroup, toGroup, "DateTimeDigitSet", flags );
    copySetting( fromGroup, toGroup, "BinaryUnitDialect", flags );
    copySetting( fromGroup, toGroup, "PageSize", flags );
    copySetting( fromGroup, toGroup, "MeasureSystem", flags );
}

void KCMLocale::copyCalendarSettings( KConfigGroup *fromGroup, KConfigGroup *toGroup, KConfig::WriteConfigFlags flags )
{
    copySetting( fromGroup, toGroup, "ShortYearWindowStartYear", flags );
    copySetting( fromGroup, toGroup, "UseCommonEra", flags );
    QString eraKey = QString::fromLatin1("Era1");
    int i = 1;
    while ( fromGroup->hasKey( eraKey ) ) {
        copySetting( fromGroup, toGroup, eraKey, flags );
        ++i;
        eraKey = QString::fromLatin1("Era%1").arg( i );
    }
}

void KCMLocale::copySetting( KConfigGroup *fromGroup, KConfigGroup *toGroup, const QString &key, KConfig::WriteConfigFlags flags )
{
    if ( fromGroup->hasKey( key ) ) {
        toGroup->writeEntry( key, fromGroup->readEntry( key, QString() ), flags );
    }
}

void KCMLocale::mergeSettings()
{
    // Set the Locale for the configs to use
    QString locale;
    if ( m_kcmTranslations.count() >= 1 ) {
        locale = m_kcmTranslations.first();
    } else {
        locale = "en_US";
    }

    m_cConfig->setLocale( locale );
    m_countryConfig->setLocale( locale );
    m_groupConfig->setLocale( locale );

    // Merge the default settings, i.e. C, Country, and Group
    m_defaultSettings.deleteGroup();
    m_defaultSettings.markAsClean();
    m_defaultConfig->setLocale( locale );
    copySettings( &m_cSettings, &m_defaultSettings );
    copySettings( &m_countrySettings, &m_defaultSettings );
    copySettings( &m_groupSettings, &m_defaultSettings );
    // Mark as clean to prevent any accidental sync() via side-effect once passed to the KLocale
    m_defaultConfig->markAsClean();
    // Need to set the language of the KLocale first, so when we pass the config into the setCountry
    // call the implicit setLocale() will find the languages match and not force a reparse of the
    // non-existent settings file, losing all our settings in the process.
    m_defaultLocale->setLanguage( m_kcmTranslations );
    m_defaultLocale->setCountry( m_defaultSettings.readEntry( "Country", QString() ), m_defaultConfig.data() );
    m_defaultSettings.writeEntry( "DayPeriod1",
                                  amPeriod( m_defaultLocale->dayPeriodText( QTime( 0, 0, 0 ), KLocale::LongName ),
                                            m_defaultLocale->dayPeriodText( QTime( 0, 0, 0 ), KLocale::ShortName ),
                                            m_defaultLocale->dayPeriodText( QTime( 0, 0, 0 ), KLocale::NarrowName ) ) );
    m_defaultSettings.writeEntry( "DayPeriod2",
                                  pmPeriod( m_defaultLocale->dayPeriodText( QTime( 12, 0, 0 ), KLocale::LongName ),
                                            m_defaultLocale->dayPeriodText( QTime( 12, 0, 0 ), KLocale::ShortName ),
                                            m_defaultLocale->dayPeriodText( QTime( 12, 0, 0 ), KLocale::NarrowName ) ) );
    m_defaultConfig->markAsClean();

    // Merge the KCM settings, i.e. C, Country, Group, and User
    m_kcmSettings.deleteGroup();
    m_kcmConfig->markAsClean();
    m_kcmConfig->setLocale( locale );
    copySettings( &m_defaultSettings, &m_kcmSettings );
    copySettings( &m_userSettings, &m_kcmSettings );

    // Merge the calendar settings sub-groups
    mergeCalendarSettings();

    // Reload the kcm translations list
    m_kcmTranslations.clear();
    m_kcmTranslations = m_kcmSettings.readEntry( "Language", QString() ).split( ':', QString::SkipEmptyParts );

    // Reload the kcm locale from the kcm settings
    // Mark as clean to prevent any accidental sync() via side-effect once passed to the KLocale
    m_kcmConfig->markAsClean();
    // Need to set the language of the KLocale first, so when we pass the config into the setCountry
    // call the implicit setLocale() will find the languages match and not force a reparse of the
    // non-existent settings file, losing all our settings in the process.
    m_kcmLocale->setLanguage( m_kcmTranslations );
    m_kcmLocale->setCountry( m_kcmSettings.readEntry( "Country", QString() ), m_kcmConfig.data() );
}

void KCMLocale::mergeCalendarSettings()
{
    // Merge the default settings, i.e. C, Country, and Group
    QString calendarType = m_defaultSettings.readEntry( "CalendarSystem", "gregorian" );
    QString calendarGroup = QString::fromLatin1( "KCalendarSystem %1" ).arg( calendarType );
    m_defaultCalendarSettings = m_defaultSettings.group( calendarGroup );
    m_defaultCalendarSettings.deleteGroup();
    copyCalendarSettings( &m_cCalendarSettings, &m_defaultCalendarSettings );
    copyCalendarSettings( &m_countryCalendarSettings, &m_defaultCalendarSettings );
    copyCalendarSettings( &m_groupCalendarSettings, &m_defaultCalendarSettings );

    // Merge the KCM settings, i.e. C, Country, Group, and User
    calendarType = m_kcmSettings.readEntry( "CalendarSystem", "gregorian" );
    calendarGroup = QString::fromLatin1( "KCalendarSystem %1" ).arg( calendarType );
    m_kcmCalendarSettings = m_kcmSettings.group( calendarGroup );
    m_kcmCalendarSettings.deleteGroup();
    copyCalendarSettings( &m_defaultCalendarSettings, &m_kcmCalendarSettings );
    copyCalendarSettings( &m_userCalendarSettings, &m_kcmCalendarSettings );
}

void KCMLocale::save()
{
    m_userConfig->sync();

    // rebuild the date base if language was changed
    if ( m_currentTranslations != m_kcmSettings.readEntry( "Language", QString() ) ) {
        KMessageBox::information(this, ki18n("Changed language settings apply only to "
                                            "newly started applications.\nTo change the "
                                            "language of all programs, you will have to "
                                            "logout first.").toString(),
                                ki18n("Applying Language Settings").toString(),
                                QLatin1String("LanguageChangesApplyOnlyToNewlyStartedPrograms"));
        KBuildSycocaProgressDialog::rebuildKSycoca(this);
    }

    load();
    KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_LOCALE);
}

QString KCMLocale::quickHelp() const
{
    return ki18n("<h1>Country/Region & Language</h1>\n"
                 "<p>Here you can set your localization settings such as language, "
                 "numeric formats, date and time formats, etc.  Choosing a country "
                 "will load a set of default formats which you can then change to "
                 "your personal preferences.  These personal preferences will remain "
                 "set even if you change the country.  The reset buttons allow you "
                 "to easily see where you have personal settings and to restore "
                 "those items to the country's default value.</p>"
                 ).toString();
}

void KCMLocale::initAllWidgets()
{
    //Common
    initTabs();
    initSample();
    initResetButtons();

    //Country tab
    initCountry();
    initCountryDivision();

    //Translations tab
    initTranslations();
    initTranslationsInstall();

    initSettingsWidgets();
}

void KCMLocale::initSettingsWidgets()
{
    // Initialise the settings widgets with the default values whenever the country or language changes

    //Numeric tab
    initNumericDigitGrouping();
    initNumericThousandsSeparator();
    initNumericDecimalSymbol();
    initNumericDecimalPlaces();
    initNumericPositiveSign();
    initNumericNegativeSign();
    initNumericDigitSet();

    //Monetary tab
    initCurrencyCode();  // Also inits CurrencySymbol
    initMonetaryDigitGrouping();
    initMonetaryThousandsSeparator();
    initMonetaryDecimalSymbol();
    initMonetaryDecimalPlaces();
    initMonetaryPositiveFormat();
    initMonetaryNegativeFormat();
    initMonetaryDigitSet();

    //Calendar tab
    initCalendarSystem();
    // this also inits all the Calendar System dependent settings

    //Date/Time tab
    initTimeFormat();
    initAmPmSymbols();
    initDateFormat();
    initShortDateFormat();
    initMonthNamePossessive();
    initDateTimeDigitSet();

    //Other tab
    initPageSize();
    initMeasureSystem();
    initBinaryUnitDialect();

    checkIfChanged();
}

void KCMLocale::initResetButtons()
{
    KGuiItem defaultItem( QString(), "document-revert", ki18n( "Reset item to its default value" ).toString() );

    //Country tab
    m_ui->m_buttonDefaultCountry->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultCountryDivision->setGuiItem( defaultItem );

    //Translations tab
    m_ui->m_buttonDefaultTranslations->setGuiItem( defaultItem );

    //Numeric tab
    m_ui->m_buttonDefaultNumericDigitGrouping->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultThousandsSeparator->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultDecimalSymbol->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultDecimalPlaces->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultPositiveSign->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultNegativeSign->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultDigitSet->setGuiItem( defaultItem );

    //Monetary tab
    m_ui->m_buttonDefaultCurrencyCode->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultCurrencySymbol->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultMonetaryDigitGrouping->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultMonetaryThousandsSeparator->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultMonetaryDecimalSymbol->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultMonetaryDecimalPlaces->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultMonetaryPositiveFormat->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultMonetaryNegativeFormat->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultMonetaryDigitSet->setGuiItem( defaultItem );

    //Calendar tab
    m_ui->m_buttonDefaultCalendarSystem->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultCalendarGregorianUseCommonEra->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultShortYearWindow->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultWeekNumberSystem->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultWeekStartDay->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultWorkingWeekStartDay->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultWorkingWeekEndDay->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultWeekDayOfPray->setGuiItem( defaultItem );

    //Date/Time tab
    m_ui->m_buttonDefaultTimeFormat->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultAmSymbol->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultPmSymbol->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultDateFormat->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultShortDateFormat->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultMonthNamePossessive->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultDateTimeDigitSet->setGuiItem( defaultItem );

    //Other tab
    m_ui->m_buttonDefaultPageSize->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultMeasureSystem->setGuiItem( defaultItem );
    m_ui->m_buttonDefaultBinaryUnitDialect->setGuiItem( defaultItem );
}

void KCMLocale::checkIfChanged()
{
    if ( m_userSettings.keyList() != m_currentSettings.keyList() ||
         m_userCalendarSettings.keyList() != m_currentCalendarSettings.keyList() ) {
        emit changed( true );
    } else {
        foreach( const QString & key, m_currentSettings.keyList() ) {
            if ( m_userSettings.readEntry( key, QString() ) !=
                 m_currentSettings.readEntry( key, QString() ) ) {
                emit changed( true );
                return;
            }
        }
        foreach( const QString & key, m_currentCalendarSettings.keyList() ) {
            if ( m_userCalendarSettings.readEntry( key, QString() ) !=
                 m_currentCalendarSettings.readEntry( key, QString() ) ) {
                emit changed( true );
                return;
            }
        }
        emit changed( false );
    }
}

// Determine if the item should be enabled or disabled
void KCMLocale::enableItemWidgets( const QString &itemKey,
                                   KConfigGroup *userSettings, KConfigGroup *kcmSettings, KConfigGroup *defaultSettings,
                                   QWidget *itemWidget, KPushButton *itemDefaultButton )
{
    // If the setting is locked down by Kiosk, then don't let the user make any changes
    if ( userSettings->isEntryImmutable( itemKey ) ) {
            itemWidget->setEnabled( false );
            itemDefaultButton->setEnabled( false );
    } else {
        itemWidget->setEnabled( true );
        // If the new value is not the default, then enable the default button
        if ( kcmSettings->readEntry( itemKey, QString() ) !=
             defaultSettings->readEntry( itemKey, QString() ) ) {
            itemDefaultButton->setEnabled( true );
        } else {
            itemDefaultButton->setEnabled( false );
        }
    }
}

// Set the value of a QString item in kcm and user settings
void KCMLocale::setItemValue( const QString &itemKey, const QString &itemValue,
                              KConfigGroup *userSettings, KConfigGroup *kcmSettings, KConfigGroup *defaultSettings )
{
    if ( !userSettings->isEntryImmutable( itemKey ) ) {
        // Always write to the kcm as the value will always exist there and needs overwriting
        kcmSettings->writeEntry( itemKey, itemValue );
        // If different to the default save the new value, otherwise delete and use the default
        if ( itemValue != defaultSettings->readEntry( itemKey, QString() ) ) {
            userSettings->writeEntry( itemKey, itemValue, KConfig::Persistent | KConfig::Global );
        } else {
            userSettings->deleteEntry( itemKey, KConfig::Persistent | KConfig::Global );
        }
    }
}

void KCMLocale::setItem( const QString &itemKey, const QString &itemValue,
                         QWidget *itemWidget, KPushButton *itemDefaultButton )
{
    setItemValue( itemKey, itemValue,
                  &m_userSettings, &m_kcmSettings, &m_defaultSettings);
    enableItemWidgets( itemKey,
                       &m_userSettings, &m_kcmSettings, &m_defaultSettings,
                       itemWidget, itemDefaultButton );
    checkIfChanged();
}

void KCMLocale::setItem( const QString &itemKey, int itemValue,
                         QWidget *itemWidget, KPushButton *itemDefaultButton )
{
    setItem( itemKey, QVariant( itemValue ).toString(), itemWidget, itemDefaultButton );
}

void KCMLocale::setItem( const QString &itemKey, bool itemValue,
                         QWidget *itemWidget, KPushButton *itemDefaultButton )
{
    setItem( itemKey, QVariant( itemValue ).toString(), itemWidget, itemDefaultButton );
}

void KCMLocale::setCalendarItem( const QString &itemKey, const QString &itemValue,
                                 QWidget *itemWidget, KPushButton *itemDefaultButton )
{
    setItemValue( itemKey, itemValue,
                  &m_userCalendarSettings, &m_kcmCalendarSettings, &m_defaultCalendarSettings );
    enableItemWidgets( itemKey,
                       &m_userCalendarSettings, &m_kcmCalendarSettings, &m_defaultCalendarSettings,
                       itemWidget, itemDefaultButton );
    checkIfChanged();
}

void KCMLocale::setCalendarItem( const QString &itemKey, int itemValue,
                                 QWidget *itemWidget, KPushButton *itemDefaultButton )
{
    setCalendarItem( itemKey, QVariant( itemValue ).toString(), itemWidget, itemDefaultButton );
}

void KCMLocale::setCalendarItem( const QString &itemKey, bool itemValue,
                                 QWidget *itemWidget, KPushButton *itemDefaultButton )
{
    setCalendarItem( itemKey, QVariant( itemValue ).toString(), itemWidget, itemDefaultButton );
}

// Don't use for edit combo's
void KCMLocale::setComboItem( const QString &itemKey, const QString &itemValue,
                              KComboBox *itemCombo, KPushButton *itemDefaultButton )
{
    setItem( itemKey, itemValue, itemCombo, itemDefaultButton );
    // Read the entry rather than use itemValue in case setItem didn't change the value, e.g. if immutable
    itemCombo->setCurrentIndex( itemCombo->findData( m_kcmSettings.readEntry( itemKey, QString() ) ) );
}

// Don't use for edit combo's
void KCMLocale::setComboItem( const QString &itemKey, int itemValue,
                              KComboBox *itemCombo, KPushButton *itemDefaultButton )
{
    setItem( itemKey, itemValue, itemCombo, itemDefaultButton );
    // Read the entry rather than use itemValue in case setItem didn't change the value, e.g. if immutable
    itemCombo->setCurrentIndex( itemCombo->findData( m_kcmSettings.readEntry( itemKey, 0 ) ) );
}

// Don't use for user changes as will EOL on the entry
void KCMLocale::setEditComboItem( const QString &itemKey, const QString &itemValue,
                                  KComboBox *itemCombo, KPushButton *itemDefaultButton )
{
    setItem( itemKey, itemValue, itemCombo, itemDefaultButton );
    // Read the entry rather than use itemValue in case setItem didn't change the value, e.g. if immutable
    itemCombo->setEditText( m_kcmSettings.readEntry( itemKey, QString() ) );
}

void KCMLocale::setIntItem( const QString &itemKey, int itemValue,
                            KIntNumInput *itemInput, KPushButton *itemDefaultButton )
{
    setItem( itemKey, itemValue, itemInput, itemDefaultButton );
    // Read the entry rather than use itemValue in case setItem didn't change the value, e.g. if immutable
    itemInput->setValue( m_kcmSettings.readEntry( itemKey, 0 ) );
}

void KCMLocale::setCheckItem( const QString &itemKey, bool itemValue,
                              QCheckBox *itemCheck, KPushButton *itemDefaultButton )
{
    setItem( itemKey, itemValue, itemCheck, itemDefaultButton );
    // Read the entry rather than use itemValue in case setItem didn't change the value, e.g. if immutable
    itemCheck->setChecked( m_kcmSettings.readEntry( itemKey, false ) );
}

// Add week day names to a combo
void KCMLocale::initWeekDayCombo( KComboBox *dayCombo )
{
    dayCombo->clear();
    int daysInWeek = m_kcmLocale->calendar()->daysInWeek( QDate::currentDate() );
    for ( int i = 1; i <= daysInWeek; ++i )
    {
        dayCombo->insertItem( i - 1, m_kcmLocale->calendar()->weekDayName( i ), QVariant( i ) );
    }
}

// Add standard separator symbols to a combo
void KCMLocale::initSeparatorCombo( KComboBox *seperatorCombo )
{
    seperatorCombo->clear();
    seperatorCombo->addItem( ki18nc( "No separator symbol" , "None" ).toString(), QString() );
    seperatorCombo->addItem( QString(','), QString(',') );
    seperatorCombo->addItem( QString('.'), QString('.') );
    seperatorCombo->addItem( ki18nc( "Space separator symbol", "Single Space" ).toString(), QString(' ') );
}

// Generic utility to set up a DigitSet combo, used for numbers, dates, money
void KCMLocale::initDigitSetCombo( KComboBox *digitSetCombo )
{
    digitSetCombo->clear();
    QList<KLocale::DigitSet> digitSets = m_kcmLocale->allDigitSetsList();
    foreach ( const KLocale::DigitSet &digitSet, digitSets )
    {
        digitSetCombo->addItem( m_kcmLocale->digitSetToName( digitSet, true ), QVariant( digitSet ) );
    }
}

void KCMLocale::insertDigitGroupingItem( KComboBox *digitGroupingCombo,
                                         KSharedConfigPtr groupingConfig, KConfigGroup *groupingSettings,
                                         const QString &digitGroupingKey, const QString &digitGroupingFormat)
{
    groupingSettings->writeEntry( digitGroupingKey, digitGroupingFormat );
    KLocale *customLocale = new KLocale( groupingConfig );
    if ( digitGroupingKey == "DigitGroupFormat" ) {
        digitGroupingCombo->addItem( customLocale->formatNumber( 123456789.12 ), digitGroupingFormat );
    } else {
        digitGroupingCombo->addItem( customLocale->formatMoney( 123456789.12 ), digitGroupingFormat );
    }
    groupingConfig->markAsClean();
    delete customLocale;
}

// Generic utility to set up a Digit Grouping combo, used for numbers and money
void KCMLocale::initDigitGroupingCombo( KComboBox *digitGroupingCombo, const QString &digitGroupingKey)
{
    digitGroupingCombo->clear();
    KSharedConfigPtr groupingConfig = KSharedConfig::openConfig( "kcmlocale-grouping", KConfig::SimpleConfig );
    KConfigGroup groupingSettings = KConfigGroup( groupingConfig, "Locale" );
    copySettings( &m_kcmSettings, &groupingSettings );
    insertDigitGroupingItem( digitGroupingCombo, groupingConfig, &groupingSettings, digitGroupingKey, "3" );
    insertDigitGroupingItem( digitGroupingCombo, groupingConfig, &groupingSettings, digitGroupingKey, "3;2" );
    insertDigitGroupingItem( digitGroupingCombo, groupingConfig, &groupingSettings, digitGroupingKey, "4" );
    insertDigitGroupingItem( digitGroupingCombo, groupingConfig, &groupingSettings, digitGroupingKey, "-1" );
}

void KCMLocale::initTabs()
{
    m_ui->m_tabWidgetSettings->setTabText( 0, ki18n( "Country" ).toString() );
    m_ui->m_tabWidgetSettings->setTabText( 1, ki18n( "Languages" ).toString() );
    m_ui->m_tabWidgetSettings->setTabText( 2, ki18n( "Numbers" ).toString() );
    m_ui->m_tabWidgetSettings->setTabText( 3, ki18n( "Money" ).toString() );
    m_ui->m_tabWidgetSettings->setTabText( 4, ki18n( "Calendar" ).toString() );
    m_ui->m_tabWidgetSettings->setTabText( 5, ki18n( "Date && Time" ).toString() );
    m_ui->m_tabWidgetSettings->setTabText( 6, ki18n( "Other" ).toString() );
}

void KCMLocale::initSample()
{
    m_ui->m_labelNumbersSample->setText( ki18n( "Numbers:" ).toString() );
    QString helpText = ki18n( "This is how positive numbers will be displayed.").toString();
    m_ui->m_textNumbersPositiveSample->setToolTip( helpText );
    m_ui->m_textNumbersPositiveSample->setWhatsThis( helpText );
    helpText = ki18n( "This is how negative numbers will be displayed.").toString();
    m_ui->m_textNumbersNegativeSample->setToolTip( helpText );
    m_ui->m_textNumbersNegativeSample->setWhatsThis( helpText );

    m_ui->m_labelMoneySample->setText( ki18n( "Money:" ).toString() );
    helpText = ki18n( "This is how positive monetary values will be displayed.").toString();
    m_ui->m_textMoneyPositiveSample->setToolTip( helpText );
    m_ui->m_textMoneyPositiveSample->setWhatsThis( helpText );
    helpText = ki18n( "This is how negative monetary values will be displayed.").toString();
    m_ui->m_textMoneyNegativeSample->setToolTip( helpText );
    m_ui->m_textMoneyNegativeSample->setWhatsThis( helpText );

    m_ui->m_labelDateSample->setText( ki18n( "Date:" ).toString() );
    helpText = ki18n( "This is how long dates will be displayed.").toString();
    m_ui->m_textDateSample->setToolTip( helpText );
    m_ui->m_textDateSample->setWhatsThis( helpText );

    m_ui->m_labelShortDateSample->setText( ki18n( "Short date:" ).toString() );
    helpText = ki18n( "This is how short dates will be displayed.").toString();
    m_ui->m_textShortDateSample->setToolTip( helpText );
    m_ui->m_textShortDateSample->setWhatsThis( helpText );

    m_ui->m_labelTimeSample->setText( ki18n( "Time:" ).toString() );
    helpText = ki18n( "This is how time will be displayed.").toString();
    m_ui->m_textTimeSample->setToolTip( helpText );
    m_ui->m_textTimeSample->setWhatsThis( helpText );

    QTimer *timer = new QTimer( this );
    timer->setObjectName( QLatin1String( "clock_timer" ) );
    connect( timer, SIGNAL( timeout() ), this, SLOT( updateSample() ) );
    timer->start( 1000 );
}

void KCMLocale::updateSample()
{
    m_ui->m_textNumbersPositiveSample->setText( m_kcmLocale->formatNumber( 123456789.12 ) );
    m_ui->m_textNumbersNegativeSample->setText( m_kcmLocale->formatNumber( -123456789.12 ) );

    m_ui->m_textMoneyPositiveSample->setText( m_kcmLocale->formatMoney( 123456789.12 ) );
    m_ui->m_textMoneyNegativeSample->setText( m_kcmLocale->formatMoney( -123456789.12 ) );

    KDateTime dateTime = KDateTime::currentLocalDateTime();
    m_ui->m_textDateSample->setText( m_kcmLocale->formatDate( dateTime.date(), KLocale::LongDate ) );
    m_ui->m_textShortDateSample->setText( m_kcmLocale->formatDate( dateTime.date(), KLocale::ShortDate ) );
    m_ui->m_textTimeSample->setText( m_kcmLocale->formatTime( dateTime.time(), true ) );
}

void KCMLocale::initCountry()
{
    m_ui->m_comboCountry->blockSignals( true );

    m_ui->m_labelCountry->setText( ki18n( "Country:" ).toString() );
    QString helpText = ki18n( "<p>This is the country where you live.  The KDE Workspace will use "
                              "the settings for this country or region.</p>" ).toString();
    m_ui->m_comboCountry->setToolTip( helpText );
    m_ui->m_comboCountry->setWhatsThis( helpText );

    m_ui->m_comboCountry->clear();

    QStringList countryCodes = m_kcmLocale->allCountriesList();
    countryCodes.removeDuplicates();
    QMap<QString, QString> countryNames;

    foreach ( const QString &countryCode, countryCodes ) {
        countryNames.insert( m_kcmLocale->countryCodeToName( countryCode ), countryCode );
    }

    QString systemCountryName = m_kcmLocale->countryCodeToName( m_systemCountry );
    QString systemCountry = ki18nc( "%1 is the system country name", "System Country (%1)" ).subs( systemCountryName ).toString();
    m_ui->m_comboCountry->addItem( systemCountry , QString() );

    QString defaultLocale = ki18n( "No Country (Default Settings)" ).toString();
    m_ui->m_comboCountry->addItem( defaultLocale , "C" );

    QMapIterator<QString, QString> it( countryNames );
    while ( it.hasNext() ) {
        it.next();
        KIcon flag( QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("locale/") + QString::fromLatin1( "l10n/%1/flag.png" ).arg( it.value() ) ) );
        m_ui->m_comboCountry->addItem( flag, it.key(), it.value() );
    }

    setCountry( m_kcmSettings.readEntry( "Country", QString() ) );

    m_ui->m_comboCountry->blockSignals( false );
}

void KCMLocale::defaultCountry()
{
    setCountry( m_defaultSettings.readEntry( "Country", QString() ) );
}

void KCMLocale::changedCountryIndex( int index )
{
    m_ui->m_comboCountry->blockSignals( true );
    setCountry( m_ui->m_comboCountry->itemData( index ).toString() );
    initCountrySettings( m_kcmSettings.readEntry( "Country", QString() ) );
    mergeSettings();
    m_ui->m_comboCountry->blockSignals( false );
    initSettingsWidgets();
}

void KCMLocale::setCountry( const QString &newValue )
{
    setComboItem( "Country", newValue,
                  m_ui->m_comboCountry, m_ui->m_buttonDefaultCountry );
}

void KCMLocale::initCountryDivision()
{
    m_ui->m_comboCountryDivision->blockSignals( true );

    m_ui->m_labelCountryDivision->setText( ki18n( "Subdivision:" ).toString() );
    QString helpText = ki18n( "<p>This is the country subdivision where you live, e.g. your state "
                              "or province.  The KDE Workspace will use this setting for local "
                              "information services such as holidays.</p>" ).toString();
    m_ui->m_comboCountryDivision->setToolTip( helpText );
    m_ui->m_comboCountryDivision->setWhatsThis( helpText );

    setCountryDivision( m_kcmSettings.readEntry( "CountryDivision", QString() ) );

    m_ui->m_labelCountryDivision->setHidden( true );
    m_ui->m_comboCountryDivision->setHidden( true );
    m_ui->m_buttonDefaultCountryDivision->setEnabled( false );
    m_ui->m_buttonDefaultCountryDivision->setHidden( true );

    m_ui->m_comboCountryDivision->blockSignals( false );
}

void KCMLocale::defaultCountryDivision()
{
    setCountryDivision( m_defaultSettings.readEntry( "CountryDivision", QString() ) );
}

void KCMLocale::changedCountryDivisionIndex( int index )
{
    setCountryDivision( m_ui->m_comboCountryDivision->itemData( index ).toString() );
}

void KCMLocale::setCountryDivision( const QString &newValue )
{
    setComboItem( "CountryDivision", newValue,
                m_ui->m_comboCountryDivision, m_ui->m_buttonDefaultCountryDivision );
    m_kcmLocale->setCountryDivisionCode( m_kcmSettings.readEntry( "CountryDivision", QString() ) );
}

void KCMLocale::initTranslations()
{
    m_ui->m_selectTranslations->blockSignals( true );

    m_ui->m_selectTranslations->setAvailableLabel( ki18n( "Available Languages:" ).toString() );
    QString availableHelp = ki18n( "<p>This is the list of installed KDE Workspace language "
                                   "translations not currently being used.  To use a language "
                                   "translation move it to the 'Preferred Languages' list in "
                                   "the order of preference.  If no suitable languages are "
                                   "listed, then you may need to install more language packages "
                                   "using your usual installation method.</p>" ).toString();
    m_ui->m_selectTranslations->availableListWidget()->setToolTip( availableHelp );
    m_ui->m_selectTranslations->availableListWidget()->setWhatsThis( availableHelp );

    m_ui->m_selectTranslations->setSelectedLabel( ki18n( "Preferred Languages:" ).toString() );
    QString selectedHelp = ki18n( "<p>This is the list of installed KDE Workspace language "
                                  "translations currently being used, listed in order of "
                                  "preference.  If a translation is not available for the "
                                  "first language in the list, the next language will be used.  "
                                  "If no other translations are available then US English will "
                                  "be used.</p>").toString();
    m_ui->m_selectTranslations->selectedListWidget()->setToolTip( selectedHelp );
    m_ui->m_selectTranslations->selectedListWidget()->setWhatsThis( selectedHelp );

    QString enUS;
    QString defaultLang = ki18nc( "%1 = default language name", "%1 (Default)" ).subs( enUS ).toString();

    // Clear the selector before reloading
    m_ui->m_selectTranslations->availableListWidget()->clear();
    m_ui->m_selectTranslations->selectedListWidget()->clear();

    // Load each user selected language into the selected list
    int i = 0;
    foreach ( const QString &languageCode, m_kcmTranslations ) {
        QListWidgetItem *listItem = new QListWidgetItem( m_ui->m_selectTranslations->selectedListWidget() );
        listItem->setText( m_kcmLocale->languageCodeToName( languageCode ) );
        listItem->setData( Qt::UserRole, languageCode );
        ++i;
    }

    // Load all the available languages the user hasn't selected into the available list
    i = 0;
    foreach ( const QString &languageCode, m_installedTranslations ) {
        if ( !m_kcmTranslations.contains( languageCode ) ) {
            QListWidgetItem *listItem = new QListWidgetItem( m_ui->m_selectTranslations->availableListWidget() );
            listItem->setText( m_kcmLocale->languageCodeToName( languageCode ) );
            listItem->setData( Qt::UserRole, languageCode );
            ++i;
        }
    }
    m_ui->m_selectTranslations->availableListWidget()->sortItems();

    // Default to selecting the first Selected language,
    // otherwise the first Available language,
    // otherwise no languages so disable all buttons
    if ( m_ui->m_selectTranslations->selectedListWidget()->count() > 0 ) {
        m_ui->m_selectTranslations->selectedListWidget()->setCurrentRow( 0 );
    } else if ( m_ui->m_selectTranslations->availableListWidget()->count() > 0 ) {
        m_ui->m_selectTranslations->availableListWidget()->setCurrentRow( 0 );
    }

    enableItemWidgets( "Language",
                       &m_userSettings, &m_kcmSettings, &m_defaultSettings,
                       m_ui->m_selectTranslations, m_ui->m_buttonDefaultTranslations );

    m_ui->m_selectTranslations->blockSignals( false );
}

void KCMLocale::defaultTranslations()
{
    setTranslations( m_defaultSettings.readEntry( "Language", QString() ) );
}

void KCMLocale::changedTranslationsAvailable( QListWidgetItem *item )
{
    Q_UNUSED( item );
    m_ui->m_selectTranslations->availableListWidget()->sortItems();
    int row = m_ui->m_selectTranslations->availableListWidget()->currentRow();
    changedTranslations();
    m_ui->m_selectTranslations->availableListWidget()->setCurrentRow( row );
}

void KCMLocale::changedTranslationsSelected( QListWidgetItem *item )
{
    Q_UNUSED( item );
    int row = m_ui->m_selectTranslations->selectedListWidget()->currentRow();
    changedTranslations();
    m_ui->m_selectTranslations->selectedListWidget()->setCurrentRow( row );
}

void KCMLocale::changedTranslations()
{
    // Read the list of all Selected translations from the selector widget
    QStringList selectedTranslations;
    for ( int i = 0; i < m_ui->m_selectTranslations->selectedListWidget()->count(); ++i ) {
        selectedTranslations.append(  m_ui->m_selectTranslations->selectedListWidget()->item( i )->data( Qt::UserRole ).toString() );
    }

    setTranslations( selectedTranslations.join( ":" ) );
}

void KCMLocale::setTranslations( const QString &newValue )
{
    setItem( "Language", newValue, m_ui->m_selectTranslations, m_ui->m_buttonDefaultTranslations );

    // Create the kcm translations list
    m_kcmTranslations.clear();
    m_kcmTranslations = m_kcmSettings.readEntry( "Language", QString() ).split( ':', QString::SkipEmptyParts );
    m_kcmLocale->setLanguage( m_kcmTranslations );

    // Do merge again as may be localized settings that need to be reloaded
    mergeSettings();

    initAllWidgets();
}

void KCMLocale::initTranslationsInstall()
{
    m_ui->m_buttonTranslationsInstall->blockSignals( true );
    m_ui->m_buttonTranslationsInstall->setText( ki18n( "Install more languages" ).toString() );
    QString helpText = ki18n( "<p>Click here to install more languages</p>" ).toString();
    m_ui->m_buttonTranslationsInstall->setToolTip( helpText );
    m_ui->m_buttonTranslationsInstall->setWhatsThis( helpText );
    m_ui->m_buttonTranslationsInstall->blockSignals( false );
}

void KCMLocale::installTranslations()
{
    // User has clicked Install Languages button, trigger distro specific install routine
}

void KCMLocale::initNumericDigitGrouping()
{
    m_ui->m_comboNumericDigitGrouping->blockSignals( true );

    m_ui->m_labelNumericDigitGrouping->setText( ki18n( "Digit grouping:" ).toString() );
    QString helpText = ki18n( "<p>Here you can define the digit grouping used to display "
                              "numbers.</p><p>Note that the digit grouping used to display "
                              "monetary values has to be set separately (see the 'Money' tab).</p>" ).toString();
    m_ui->m_comboNumericDigitGrouping->setToolTip( helpText );
    m_ui->m_comboNumericDigitGrouping->setWhatsThis( helpText );

    initDigitGroupingCombo( m_ui->m_comboNumericDigitGrouping, "DigitGroupFormat" );

    setNumericDigitGrouping( m_kcmSettings.readEntry( "DigitGroupFormat", "3" ) );

    m_ui->m_comboNumericDigitGrouping->blockSignals( false );

    updateSample();
}

void KCMLocale::defaultNumericDigitGrouping()
{
    setNumericDigitGrouping( m_defaultSettings.readEntry( "DigitGroupFormat", "3" ) );
}

void KCMLocale::changedNumericDigitGroupingIndex( int index )
{
    setNumericDigitGrouping( m_ui->m_comboNumericDigitGrouping->itemData( index ).toString() );
}

void KCMLocale::setNumericDigitGrouping( const QString &newValue )
{
    setComboItem( "DigitGroupFormat", newValue,
                  m_ui->m_comboNumericDigitGrouping, m_ui->m_buttonDefaultNumericDigitGrouping );

    // No api to set, so need to force reload the locale
    m_kcmConfig->markAsClean();
    m_kcmLocale->setCountry( m_kcmSettings.readEntry( "Country", QString() ), m_kcmConfig.data() );

    updateSample();
}

void KCMLocale::initNumericThousandsSeparator()
{
    m_ui->m_comboThousandsSeparator->blockSignals( true );

    m_ui->m_labelThousandsSeparator->setText( ki18n( "Group separator:" ).toString() );
    QString helpText = ki18n( "<p>Here you can define the digit group separator used to display "
                              "numbers.</p><p>Note that the digit group separator used to display "
                              "monetary values has to be set separately (see the 'Money' tab).</p>" ).toString();
    m_ui->m_comboThousandsSeparator->setToolTip( helpText );
    m_ui->m_comboThousandsSeparator->setWhatsThis( helpText );

    initSeparatorCombo( m_ui->m_comboThousandsSeparator );

    setNumericThousandsSeparator( m_kcmSettings.readEntry( "ThousandsSeparator", QString() )
                                               .remove( QString::fromLatin1("$0") ) );

    m_ui->m_comboThousandsSeparator->blockSignals( false );
}

void KCMLocale::defaultNumericThousandsSeparator()
{
    setNumericThousandsSeparator( m_defaultSettings.readEntry( "ThousandsSeparator", QString() )
                                                   .remove( QString::fromLatin1("$0") ) );
}

// Change in response to user input, doesn't set edit text as causes cursor to EOL
void KCMLocale::changedNumericThousandsSeparator( const QString &newValue )
{
    QString useValue = newValue;
    int item = m_ui->m_comboThousandsSeparator->findText( newValue );
    if ( item >= 0 ) {
        useValue = m_ui->m_comboThousandsSeparator->itemData( item ).toString();
        m_ui->m_comboThousandsSeparator->setEditText( useValue );
    }
    if ( useValue == QString(' ') ) {
        useValue = "$0 $0";
    }
    setItem( "ThousandsSeparator", useValue,
             m_ui->m_comboThousandsSeparator, m_ui->m_buttonDefaultThousandsSeparator );
    m_kcmLocale->setThousandsSeparator( m_kcmSettings.readEntry( "ThousandsSeparator", QString() )
                                                     .remove( QString::fromLatin1("$0") ) );

    // Update the numeric format samples to relect new setting
    initNumericDigitGrouping();
}

// Change programatically, does set edit text so user can see it
void KCMLocale::setNumericThousandsSeparator( const QString &newValue )
{
    changedNumericThousandsSeparator( newValue );
    m_ui->m_comboThousandsSeparator->setEditText( m_kcmSettings.readEntry( "ThousandsSeparator", QString() )
                                                               .remove( QString::fromLatin1("$0") ) );

    // Update the numeric format samples to relect new setting
    initNumericDigitGrouping();
}

void KCMLocale::initNumericDecimalSymbol()
{
    m_ui->m_comboDecimalSymbol->blockSignals( true );

    m_ui->m_labelDecimalSymbol->setText( ki18n( "Decimal separator:" ).toString() );
    QString helpText = ki18n( "<p>Here you can define the decimal separator used to display "
                              "numbers (i.e. a dot or a comma in most countries).</p><p>Note "
                              "that the decimal separator used to display monetary values has "
                              "to be set separately (see the 'Money' tab).</p>" ).toString();
    m_ui->m_comboDecimalSymbol->setToolTip( helpText );
    m_ui->m_comboDecimalSymbol->setWhatsThis( helpText );

    initSeparatorCombo( m_ui->m_comboDecimalSymbol );

    setNumericDecimalSymbol( m_kcmSettings.readEntry( "DecimalSymbol", QString() ) );

    m_ui->m_comboDecimalSymbol->blockSignals( false );
}

void KCMLocale::defaultNumericDecimalSymbol()
{
    setNumericDecimalSymbol( m_defaultSettings.readEntry( "DecimalSymbol", QString() ) );
}

// Change in response to user input, doesn't set edit text as causes cursor to EOL
void KCMLocale::changedNumericDecimalSymbol( const QString &newValue )
{
    QString useValue = newValue;
    int item = m_ui->m_comboDecimalSymbol->findText( newValue );
    if ( item >= 0 ) {
        useValue = m_ui->m_comboDecimalSymbol->itemData( item ).toString();
    }
    setItem( "DecimalSymbol", useValue,
             m_ui->m_comboDecimalSymbol, m_ui->m_buttonDefaultDecimalSymbol );
    m_kcmLocale->setDecimalSymbol( m_kcmSettings.readEntry( "DecimalSymbol", QString() ) );

    // Update the numeric format samples to relect new setting
    initNumericDigitGrouping();
}

// Change programatically, does set edit text so user can see it
void KCMLocale::setNumericDecimalSymbol( const QString &newValue )
{
    setEditComboItem( "DecimalSymbol", newValue,
                      m_ui->m_comboDecimalSymbol, m_ui->m_buttonDefaultDecimalSymbol );
    m_kcmLocale->setDecimalSymbol( m_kcmSettings.readEntry( "DecimalSymbol", QString() ) );

    // Update the numeric format samples to relect new setting
    initNumericDigitGrouping();
}

void KCMLocale::initNumericDecimalPlaces()
{
    m_ui->m_intDecimalPlaces->blockSignals( true );

    m_ui->m_labelDecimalPlaces->setText( ki18n( "Decimal places:" ).toString() );
    QString helpText = ki18n( "<p>Here you can set the number of decimal places displayed for "
                              "numeric values, i.e. the number of digits <em>after</em> the "
                              "decimal separator.</p><p>Note that the decimal places used "
                              "to display monetary values has to be set separately (see the "
                              "'Money' tab).</p>" ).toString();
    m_ui->m_intDecimalPlaces->setToolTip( helpText );
    m_ui->m_intDecimalPlaces->setWhatsThis( helpText );

    setNumericDecimalPlaces( m_kcmSettings.readEntry( "DecimalPlaces", 0 ) );

    m_ui->m_intDecimalPlaces->blockSignals( false );
}

void KCMLocale::defaultNumericDecimalPlaces()
{
    setNumericDecimalPlaces( m_defaultSettings.readEntry( "DecimalPlaces", 0 ) );
}

void KCMLocale::changedNumericDecimalPlaces( int newValue )
{
    setNumericDecimalPlaces( newValue );
}

void KCMLocale::setNumericDecimalPlaces( int newValue )
{
    setIntItem( "DecimalPlaces", newValue,
                m_ui->m_intDecimalPlaces, m_ui->m_buttonDefaultDecimalPlaces );
    m_kcmLocale->setDecimalPlaces( m_kcmSettings.readEntry( "DecimalPlaces", 0 ) );

    // Update the numeric format samples to relect new setting
    initNumericDigitGrouping();
}

void KCMLocale::initNumericPositiveSign()
{
    m_ui->m_comboPositiveSign->blockSignals( true );

    m_ui->m_labelPositiveFormat->setText( ki18n( "Positive sign:" ).toString() );
    QString helpText = ki18n( "<p>Here you can specify text used to prefix positive numbers. "
                              "Most locales leave this blank.</p><p>Note that the positive sign "
                              "used to display monetary values has to be set separately (see the "
                              "'Money' tab).</p>" ).toString();
    m_ui->m_comboPositiveSign->setToolTip( helpText );
    m_ui->m_comboPositiveSign->setWhatsThis( helpText );

    m_ui->m_comboPositiveSign->clear();
    m_ui->m_comboPositiveSign->addItem( ki18nc( "No positive symbol", "None" ).toString(), QString() );
    m_ui->m_comboPositiveSign->addItem( QString('+'), QString('+') );

    setNumericPositiveSign( m_kcmSettings.readEntry( "PositiveSign", QString() ) );

    m_ui->m_comboPositiveSign->blockSignals( false );
}

void KCMLocale::defaultNumericPositiveSign()
{
    setNumericPositiveSign( m_defaultSettings.readEntry( "PositiveSign", QString() ) );
}

// Change in response to user input, doesn't set edit text as causes cursor to EOL
void KCMLocale::changedNumericPositiveSign( const QString &newValue )
{
    QString useValue = newValue;
    int item = m_ui->m_comboPositiveSign->findText( newValue );
    if ( item >= 0 ) {
        useValue = m_ui->m_comboPositiveSign->itemData( item ).toString();
    }
    setItem( "PositiveSign", useValue,
             m_ui->m_comboPositiveSign, m_ui->m_buttonDefaultPositiveSign );
    m_kcmLocale->setPositiveSign( m_kcmSettings.readEntry( "PositiveSign", QString() ) );

    // Update the format samples to relect new setting
    initNumericDigitGrouping();
    initMonetaryDigitGrouping();
    initMonetaryPositiveFormat();
}

// Change programatically, does set edit text so user can see it
void KCMLocale::setNumericPositiveSign( const QString &newValue )
{
    setEditComboItem( "PositiveSign", newValue,
                      m_ui->m_comboPositiveSign, m_ui->m_buttonDefaultPositiveSign );
    m_kcmLocale->setPositiveSign( m_kcmSettings.readEntry( "PositiveSign", QString() ) );

    // Update the format samples to relect new setting
    initNumericDigitGrouping();
    initMonetaryDigitGrouping();
    initMonetaryPositiveFormat();
}

void KCMLocale::initNumericNegativeSign()
{
    m_ui->m_comboNegativeSign->blockSignals( true );

    m_ui->m_labelNegativeFormat->setText( ki18n( "Negative sign:" ).toString() );
    QString helpText = ki18n( "<p>Here you can specify text used to prefix negative numbers. "
                              "This should not be empty, so you can distinguish positive and "
                              "negative numbers. It is normally set to minus (-).</p><p>Note "
                              "that the negative sign used to display monetary values has to "
                              "be set separately (see the 'Money' tab).</p>" ).toString();
    m_ui->m_comboNegativeSign->setToolTip( helpText );
    m_ui->m_comboNegativeSign->setWhatsThis( helpText );

    m_ui->m_comboNegativeSign->clear();
    m_ui->m_comboNegativeSign->addItem( ki18nc("No negative symbol", "None" ).toString(), QString() );
    m_ui->m_comboNegativeSign->addItem( QString('-'), QString('-') );

    setNumericNegativeSign( m_kcmSettings.readEntry( "NegativeSign", QString() ) );

    m_ui->m_comboNegativeSign->blockSignals( false );
}

void KCMLocale::defaultNumericNegativeSign()
{
    setNumericNegativeSign( m_defaultSettings.readEntry( "NegativeSign", QString() ) );
}

// Change in response to user input, doesn't set edit text as causes cursor to EOL
void KCMLocale::changedNumericNegativeSign( const QString &newValue )
{
    QString useValue = newValue;
    int item = m_ui->m_comboNegativeSign->findText( newValue );
    if ( item >= 0 ) {
        useValue = m_ui->m_comboNegativeSign->itemData( item ).toString();
    }
    setItem( "NegativeSign", useValue,
             m_ui->m_comboNegativeSign, m_ui->m_buttonDefaultNegativeSign );
    m_kcmLocale->setNegativeSign( m_kcmSettings.readEntry( "NegativeSign", QString() ) );

    // Update the monetary format samples to relect new setting
    initMonetaryNegativeFormat();
}

// Change programatically, does set edit text so user can see it
void KCMLocale::setNumericNegativeSign( const QString &newValue )
{
    setEditComboItem( "NegativeSign", newValue,
                      m_ui->m_comboNegativeSign, m_ui->m_buttonDefaultNegativeSign );
    m_kcmLocale->setNegativeSign( m_kcmSettings.readEntry( "NegativeSign", QString() ) );

    // Update the monetary format samples to relect new setting
    initMonetaryNegativeFormat();
}

void KCMLocale::initNumericDigitSet()
{
    m_ui->m_comboDigitSet->blockSignals( true );

    m_ui->m_labelDigitSet->setText( ki18n( "Digit set:" ).toString() );
    QString helpText = ki18n( "<p>Here you can define the set of digits used to display numbers. "
                              "If digits other than Arabic are selected, they will appear only if "
                              "used in the language of the application or the piece of text where "
                              "the number is shown.</p> <p>Note that the set of digits used to "
                              "display monetary values has to be set separately (see the 'Money' "
                              "tab).</p>" ).toString();
    m_ui->m_comboDigitSet->setToolTip( helpText );
    m_ui->m_comboDigitSet->setWhatsThis( helpText );

    initDigitSetCombo( m_ui->m_comboDigitSet );

    setNumericDigitSet( m_kcmSettings.readEntry( "DigitSet", 0 ) );

    m_ui->m_comboDigitSet->blockSignals( false );
}

void KCMLocale::defaultNumericDigitSet()
{
    setNumericDigitSet( m_defaultSettings.readEntry( "DigitSet", 0 ) );
}

void KCMLocale::changedNumericDigitSetIndex( int index )
{
    setNumericDigitSet( m_ui->m_comboDigitSet->itemData( index ).toInt() );
}

void KCMLocale::setNumericDigitSet( int newValue )
{
    setComboItem( "DigitSet", newValue,
                  m_ui->m_comboDigitSet, m_ui->m_buttonDefaultDigitSet );
    m_kcmLocale->setDigitSet( (KLocale::DigitSet) m_kcmSettings.readEntry( "DigitSet", 0 ) );

    // Update the numeric format samples to relect new setting
    initNumericDigitGrouping();
}

void KCMLocale::initCurrencyCode()
{
    m_ui->m_comboCurrencyCode->blockSignals( true );

    m_ui->m_labelCurrencyCode->setText( ki18n( "Currency:" ).toString() );
    QString helpText = ki18n( "<p>Here you can choose the currency to be used when displaying "
                              "monetary values, e.g. United States Dollar or Pound Sterling.</p>" ).toString();
    m_ui->m_comboCurrencyCode->setToolTip( helpText );
    m_ui->m_comboCurrencyCode->setWhatsThis( helpText );

    //Create the list of Currency Codes to choose from.
    //Visible text will be localised name plus unlocalised code.
    m_ui->m_comboCurrencyCode->clear();
    //First put all the preferred currencies first in order of priority
    QStringList currencyCodeList = m_kcmLocale->currencyCodeList();
    foreach ( const QString &currencyCode, currencyCodeList ) {
        QString text = ki18nc( "@item currency name and currency code", "%1 (%2)")
                       .subs( m_kcmLocale->currency()->currencyCodeToName( currencyCode ) )
                       .subs( currencyCode )
                       .toString();
        m_ui->m_comboCurrencyCode->addItem( text, QVariant( currencyCode ) );
    }
    //Next put all currencies available in alphabetical name order
    m_ui->m_comboCurrencyCode->insertSeparator(m_ui->m_comboCurrencyCode->count());
    currencyCodeList = m_kcmLocale->currency()->allCurrencyCodesList();
    QStringList currencyNameList;
    foreach ( const QString &currencyCode, currencyCodeList ) {
        currencyNameList.append( ki18nc( "@item currency name and currency code", "%1 (%2)")
                                 .subs( m_kcmLocale->currency()->currencyCodeToName( currencyCode ) )
                                 .subs( currencyCode )
                                 .toString() );
    }
    currencyNameList.sort();
    foreach ( const QString &name, currencyNameList ) {
        m_ui->m_comboCurrencyCode->addItem( name, QVariant( name.mid( name.length()-4, 3 ) ) );
    }

    setCurrencyCode( m_kcmSettings.readEntry( "CurrencyCode", QString() ) );

    m_ui->m_comboCurrencyCode->blockSignals( false );
}

void KCMLocale::defaultCurrencyCode()
{
    setCurrencyCode( m_defaultSettings.readEntry( "CurrencyCode", QString() ) );
}

void KCMLocale::changedCurrencyCodeIndex( int index )
{
    setCurrencyCode( m_ui->m_comboCurrencyCode->itemData( index ).toString() );
}

void KCMLocale::setCurrencyCode( const QString &newValue )
{
    setComboItem( "CurrencyCode", newValue,
                  m_ui->m_comboCurrencyCode, m_ui->m_buttonDefaultCurrencyCode );
    m_kcmLocale->setCurrencyCode( m_kcmSettings.readEntry( "CurrencyCode", QString() ) );
    // Update the Currency dependent widgets with the new Currency details
    initCurrencySymbol();
}

void KCMLocale::initCurrencySymbol()
{
    m_ui->m_comboCurrencySymbol->blockSignals( true );

    m_ui->m_labelCurrencySymbol->setText( ki18n( "Currency symbol:" ).toString() );
    QString helpText = ki18n( "<p>Here you can choose the symbol to be used when displaying "
                              "monetary values, e.g. $, US$ or USD.</p>" ).toString();
    m_ui->m_comboCurrencySymbol->setToolTip( helpText );
    m_ui->m_comboCurrencySymbol->setWhatsThis( helpText );

    //Create the list of Currency Symbols for the selected Currency Code
    m_ui->m_comboCurrencySymbol->clear();
    QStringList currencySymbolList = m_kcmLocale->currency()->symbolList();
    foreach ( const QString &currencySymbol, currencySymbolList ) {
        if ( currencySymbol == m_kcmLocale->currency()->defaultSymbol() ) {
            m_ui->m_comboCurrencySymbol->addItem( currencySymbol, QVariant( QString() ) );
        } else {
            m_ui->m_comboCurrencySymbol->addItem( currencySymbol, QVariant( currencySymbol ) );
        }
    }

    if ( !currencySymbolList.contains( m_kcmSettings.readEntry( "CurrencySymbol", QString() ) ) ) {
        m_kcmSettings.deleteEntry( "CurrencySymbol" );
        m_userSettings.deleteEntry( "CurrencySymbol", KConfig::Persistent | KConfig::Global );
    }

    setCurrencySymbol( m_kcmSettings.readEntry( "CurrencySymbol", QString() ) );

    m_ui->m_comboCurrencySymbol->blockSignals( false );
}

void KCMLocale::defaultCurrencySymbol()
{
    setCurrencySymbol( m_defaultSettings.readEntry( "CurrencySymbol", QString() ) );
}

void KCMLocale::changedCurrencySymbolIndex( int index )
{
    setCurrencySymbol( m_ui->m_comboCurrencySymbol->itemData( index ).toString() );
}

void KCMLocale::setCurrencySymbol( const QString &newValue )
{
    setComboItem( "CurrencySymbol", newValue,
                  m_ui->m_comboCurrencySymbol, m_ui->m_buttonDefaultCurrencySymbol );
    if ( m_kcmSettings.readEntry( "CurrencySymbol", QString() ) != QString() ) {
        m_kcmLocale->setCurrencySymbol( m_kcmSettings.readEntry( "CurrencySymbol", QString() ) );
    } else {
        m_kcmLocale->setCurrencySymbol( m_kcmLocale->currency()->defaultSymbol() );
    }

    // Update the monetary format samples to relect new setting
    initMonetaryDigitGrouping();
    initMonetaryPositiveFormat();
    initMonetaryNegativeFormat();
}

void KCMLocale::initMonetaryDigitGrouping()
{
    m_ui->m_comboMonetaryDigitGrouping->blockSignals( true );

    m_ui->m_labelMonetaryDigitGrouping->setText( ki18n( "Digit grouping:" ).toString() );
    QString helpText = ki18n( "<p>Here you can define the digit grouping used to display monetary "
                              "values.</p><p>Note that the digit grouping used to display "
                              "other numbers has to be defined separately (see the 'Numbers' tab)."
                              "</p>" ).toString();
    m_ui->m_comboMonetaryDigitGrouping->setToolTip( helpText );
    m_ui->m_comboMonetaryDigitGrouping->setWhatsThis( helpText );

    initDigitGroupingCombo( m_ui->m_comboMonetaryDigitGrouping, "MonetaryDigitGroupFormat" );

    setMonetaryDigitGrouping( m_kcmSettings.readEntry( "MonetaryDigitGroupFormat", "3" ) );

    m_ui->m_comboMonetaryDigitGrouping->blockSignals( false );
}

void KCMLocale::defaultMonetaryDigitGrouping()
{
    setMonetaryDigitGrouping( m_defaultSettings.readEntry( "MonetaryDigitGroupFormat", "3" ) );
}

void KCMLocale::changedMonetaryDigitGroupingIndex( int index )
{
    setMonetaryDigitGrouping( m_ui->m_comboMonetaryDigitGrouping->itemData( index ).toString() );
}

void KCMLocale::setMonetaryDigitGrouping( const QString &newValue )
{
    setComboItem( "MonetaryDigitGroupFormat", newValue,
                  m_ui->m_comboMonetaryDigitGrouping, m_ui->m_buttonDefaultMonetaryDigitGrouping );

    // No api to set, so need to force reload the locale
    m_kcmConfig->markAsClean();
    m_kcmLocale->setCountry( m_kcmSettings.readEntry( "Country", QString() ), m_kcmConfig.data() );
    updateSample();
}

void KCMLocale::initMonetaryThousandsSeparator()
{
    m_ui->m_comboMonetaryThousandsSeparator->blockSignals( true );

    m_ui->m_labelMonetaryThousandsSeparator->setText( ki18n( "Group separator:" ).toString() );
    QString helpText = ki18n( "<p>Here you can define the group separator used to display monetary "
                              "values.</p><p>Note that the thousands separator used to display "
                              "other numbers has to be defined separately (see the 'Numbers' tab)."
                              "</p>" ).toString();
    m_ui->m_comboMonetaryThousandsSeparator->setToolTip( helpText );
    m_ui->m_comboMonetaryThousandsSeparator->setWhatsThis( helpText );

    initSeparatorCombo( m_ui->m_comboMonetaryThousandsSeparator );

    setMonetaryThousandsSeparator( m_kcmSettings.readEntry( "MonetaryThousandsSeparator", QString() )
                                                .remove( QString::fromLatin1("$0") ) );

    m_ui->m_comboMonetaryThousandsSeparator->blockSignals( false );
}

void KCMLocale::defaultMonetaryThousandsSeparator()
{
    setMonetaryThousandsSeparator( m_defaultSettings.readEntry( "MonetaryThousandsSeparator", QString() )
                                                    .remove( QString::fromLatin1("$0") ) );
}

void KCMLocale::changedMonetaryThousandsSeparator( const QString &newValue )
{
    QString useValue = newValue;
    int item = m_ui->m_comboMonetaryThousandsSeparator->findText( newValue );
    if ( item >= 0 ) {
        useValue = m_ui->m_comboMonetaryThousandsSeparator->itemData( item ).toString();
        m_ui->m_comboMonetaryThousandsSeparator->setEditText( useValue );
    }
    if ( useValue == QString(' ') ) {
        useValue = "$0 $0";
    }
    setItem( "MonetaryThousandsSeparator", useValue,
             m_ui->m_comboMonetaryThousandsSeparator, m_ui->m_buttonDefaultMonetaryThousandsSeparator );
    m_kcmLocale->setMonetaryThousandsSeparator( m_kcmSettings.readEntry( "MonetaryThousandsSeparator", QString() )
                                                             .remove( QString::fromLatin1("$0") ) );

    // Update the monetary format samples to relect new setting
    initMonetaryDigitGrouping();
    initMonetaryPositiveFormat();
    initMonetaryNegativeFormat();
    updateSample();
}

void KCMLocale::setMonetaryThousandsSeparator( const QString &newValue )
{
    changedMonetaryThousandsSeparator( newValue );
    m_ui->m_comboMonetaryThousandsSeparator->setEditText( m_kcmSettings.readEntry( "MonetaryThousandsSeparator", QString() )
                                                                       .remove( QString::fromLatin1("$0") ) );
}

void KCMLocale::initMonetaryDecimalSymbol()
{
    m_ui->m_comboMonetaryDecimalSymbol->blockSignals( true );

    m_ui->m_labelMonetaryDecimalSymbol->setText( ki18n( "Decimal separator:" ).toString() );
    QString helpText = ki18n( "<p>Here you can define the decimal separator used to display "
                              "monetary values.</p><p>Note that the decimal separator used to "
                              "display other numbers has to be defined separately (see the "
                              "'Numbers' tab).</p>" ).toString();
    m_ui->m_comboMonetaryDecimalSymbol->setToolTip( helpText );
    m_ui->m_comboMonetaryDecimalSymbol->setWhatsThis( helpText );

    initSeparatorCombo( m_ui->m_comboMonetaryDecimalSymbol );

    setMonetaryDecimalSymbol( m_kcmSettings.readEntry( "MonetaryDecimalSymbol", QString() ) );

    m_ui->m_comboMonetaryDecimalSymbol->blockSignals( false );
}

void KCMLocale::defaultMonetaryDecimalSymbol()
{
    setMonetaryDecimalSymbol( m_defaultSettings.readEntry( "MonetaryDecimalSymbol", QString() ) );
}

void KCMLocale::changedMonetaryDecimalSymbol( const QString &newValue )
{
    QString useValue = newValue;
    int item = m_ui->m_comboMonetaryDecimalSymbol->findText( newValue );
    if ( item >= 0 ) {
        useValue = m_ui->m_comboMonetaryDecimalSymbol->itemData( item ).toString();
    }
    setItem( "MonetaryDecimalSymbol", useValue,
             m_ui->m_comboMonetaryDecimalSymbol, m_ui->m_buttonDefaultMonetaryDecimalSymbol );
    m_kcmLocale->setMonetaryDecimalSymbol( m_kcmSettings.readEntry( "MonetaryDecimalSymbol", QString() ) );

    // Update the monetary format samples to relect new setting
    initMonetaryDigitGrouping();
    initMonetaryPositiveFormat();
    initMonetaryNegativeFormat();
}

void KCMLocale::setMonetaryDecimalSymbol( const QString &newValue )
{
    setEditComboItem( "MonetaryDecimalSymbol", newValue,
                      m_ui->m_comboMonetaryDecimalSymbol, m_ui->m_buttonDefaultMonetaryDecimalSymbol );
    m_kcmLocale->setMonetaryDecimalSymbol( m_kcmSettings.readEntry( "MonetaryDecimalSymbol", QString() ) );

    // Update the monetary format samples to relect new setting
    initMonetaryDigitGrouping();
    initMonetaryPositiveFormat();
    initMonetaryNegativeFormat();
}

void KCMLocale::initMonetaryDecimalPlaces()
{
    m_ui->m_intMonetaryDecimalPlaces->blockSignals( true );

    m_ui->m_labelMonetaryDecimalPlaces->setText( ki18n( "Decimal places:" ).toString() );
    QString helpText = ki18n( "<p>Here you can set the number of decimal places displayed for "
                              "monetary values, i.e. the number of digits <em>after</em> the "
                              "decimal separator.</p><p>Note that the decimal places used to "
                              "display other numbers has to be defined separately (see the "
                              "'Numbers' tab).</p>" ).toString();
    m_ui->m_intMonetaryDecimalPlaces->setToolTip( helpText );
    m_ui->m_intMonetaryDecimalPlaces->setWhatsThis( helpText );

    setMonetaryDecimalPlaces( m_kcmSettings.readEntry( "MonetaryDecimalPlaces", 0 ) );

    m_ui->m_intMonetaryDecimalPlaces->blockSignals( false );
}

void KCMLocale::defaultMonetaryDecimalPlaces()
{
    setMonetaryDecimalPlaces( m_defaultSettings.readEntry( "MonetaryDecimalPlaces", 0 ) );
}

void KCMLocale::changedMonetaryDecimalPlaces( int newValue )
{
    setMonetaryDecimalPlaces( newValue );
}

void KCMLocale::setMonetaryDecimalPlaces( int newValue )
{
    setIntItem( "MonetaryDecimalPlaces", newValue,
                m_ui->m_intMonetaryDecimalPlaces, m_ui->m_buttonDefaultMonetaryDecimalPlaces );
    m_kcmLocale->setMonetaryDecimalPlaces( m_kcmSettings.readEntry( "MonetaryDecimalPlaces", 0 ) );

    // Update the monetary format samples to relect new setting
    initMonetaryDigitGrouping();
    initMonetaryPositiveFormat();
    initMonetaryNegativeFormat();
}

void KCMLocale::insertMonetaryPositiveFormat( bool prefixCurrencySymbol, KLocale::SignPosition signPosition )
{
    KLocale custom( *m_kcmLocale );
    custom.setPositivePrefixCurrencySymbol( prefixCurrencySymbol );
    custom.setPositiveMonetarySignPosition( signPosition );
    QVariantList options;
    options.append( QVariant( prefixCurrencySymbol ) );
    options.append( QVariant( signPosition ) );
    m_ui->m_comboMonetaryPositiveFormat->addItem( custom.formatMoney( 123456.78 ), options );
}

void KCMLocale::initMonetaryPositiveFormat()
{
    m_ui->m_comboMonetaryPositiveFormat->blockSignals( true );

    m_ui->m_labelMonetaryPositiveFormat->setText( ki18n( "Positive format:" ).toString() );
    QString helpText = ki18n( "<p>Here you can set the format of positive monetary values.</p>"
                              "<p>Note that the positive sign used to display other numbers has "
                              "to be defined separately (see the 'Numbers' tab).</p>" ).toString();
    m_ui->m_comboMonetaryPositiveFormat->setToolTip( helpText );
    m_ui->m_comboMonetaryPositiveFormat->setWhatsThis( helpText );

    m_ui->m_comboMonetaryPositiveFormat->clear();
    // If the positive sign is null, then all the sign options will look the same, so only show a
    // choice between parens and no sign, but preserve original position in case they do set sign
    // Also keep options in same order, i.e. sign options then parens option
    if ( m_kcmSettings.readEntry( "PositiveSign", QString() ).isEmpty() ) {
        KLocale::SignPosition currentSignPosition = (KLocale::SignPosition) m_currentSettings.readEntry( "PositiveMonetarySignPosition", 0 );
        KLocale::SignPosition kcmSignPosition = (KLocale::SignPosition) m_kcmSettings.readEntry( "PositiveMonetarySignPosition", 0 );
        if ( currentSignPosition == KLocale::ParensAround && kcmSignPosition == KLocale::ParensAround ) {
            //Both are parens, so also give a sign option
            insertMonetaryPositiveFormat( true, KLocale::BeforeQuantityMoney );
            insertMonetaryPositiveFormat( false, KLocale::BeforeQuantityMoney );
            insertMonetaryPositiveFormat( true, kcmSignPosition );
            insertMonetaryPositiveFormat( false, kcmSignPosition );
        } else if ( kcmSignPosition == KLocale::ParensAround ) {
            //kcm is parens, current is sign, use both in right order
            insertMonetaryPositiveFormat( true, currentSignPosition );
            insertMonetaryPositiveFormat( false, currentSignPosition );
            insertMonetaryPositiveFormat( true, kcmSignPosition );
            insertMonetaryPositiveFormat( false, kcmSignPosition );
        } else {
            // kcm is sign, current is parens, use both in right order
            insertMonetaryPositiveFormat( true, kcmSignPosition );
            insertMonetaryPositiveFormat( false, kcmSignPosition );
            insertMonetaryPositiveFormat( true, currentSignPosition );
            insertMonetaryPositiveFormat( false, currentSignPosition );
        }
    } else {
        // Show the sign options first, then parens
        // Could do a loop, but lets keep it simple
        insertMonetaryPositiveFormat( true, KLocale::BeforeQuantityMoney );
        insertMonetaryPositiveFormat( false, KLocale::BeforeQuantityMoney );
        insertMonetaryPositiveFormat( true, KLocale::AfterQuantityMoney );
        insertMonetaryPositiveFormat( false, KLocale::AfterQuantityMoney );
        insertMonetaryPositiveFormat( true, KLocale::BeforeMoney );
        insertMonetaryPositiveFormat( false, KLocale::BeforeMoney );
        insertMonetaryPositiveFormat( true, KLocale::AfterMoney );
        insertMonetaryPositiveFormat( false, KLocale::AfterMoney );
        insertMonetaryPositiveFormat( true, KLocale::ParensAround );
        insertMonetaryPositiveFormat( false, KLocale::ParensAround );
    }

    setMonetaryPositiveFormat( m_kcmSettings.readEntry( "PositivePrefixCurrencySymbol", false ),
                               (KLocale::SignPosition) m_defaultSettings.readEntry( "PositiveMonetarySignPosition", 0 ) );

    // These are the old strings, keep around for now in case new implementation isn't usable
    QString format = ki18n( "Sign position:" ).toString();
    format = ki18n( "Parentheses Around" ).toString();
    format = ki18n( "Before Quantity Money" ).toString();
    format = ki18n( "After Quantity Money" ).toString();
    format = ki18n( "Before Money" ).toString();
    format = ki18n( "After Money" ).toString();
    format = ki18n( "Here you can select how a positive sign will be "
                    "positioned. This only affects monetary values." ).toString();

    QString check = ki18n( "Prefix currency symbol" ).toString();
    check = ki18n( "If this option is checked, the currency sign "
                    "will be prefixed (i.e. to the left of the "
                    "value) for all positive monetary values. If "
                    "not, it will be postfixed (i.e. to the right)." ).toString();

    m_ui->m_comboMonetaryPositiveFormat->blockSignals( false );
}

void KCMLocale::defaultMonetaryPositiveFormat()
{
    setMonetaryPositiveFormat( m_defaultSettings.readEntry( "PositivePrefixCurrencySymbol", false ),
                               (KLocale::SignPosition) m_defaultSettings.readEntry( "PositiveMonetarySignPosition", 0 ) );
}

void KCMLocale::changedMonetaryPositiveFormatIndex( int index )
{
    QVariantList options = m_ui->m_comboMonetaryPositiveFormat->itemData( index ).toList();
    bool prefixCurrencySymbol = options.at( 0 ).toBool();
    KLocale::SignPosition signPosition = (KLocale::SignPosition) options.at( 1 ).toInt();
    setMonetaryPositiveFormat( prefixCurrencySymbol, signPosition );
}

void KCMLocale::setMonetaryFormat( const QString &prefixCurrencySymbolKey, bool prefixCurrencySymbol,
                                   const QString &signPositionKey, KLocale::SignPosition signPosition,
                                   QWidget *formatWidget, KPushButton *formatDefaultButton )
{
    // If either setting is locked down by Kiosk, then don't let the user make any changes, and disable the widgets
    if ( m_userSettings.isEntryImmutable( prefixCurrencySymbolKey ) ||
         m_userSettings.isEntryImmutable( signPositionKey ) ) {
            formatWidget->setEnabled( false );
            formatDefaultButton->setEnabled( false );
    } else {
        formatWidget->setEnabled( true );
        formatDefaultButton->setEnabled( false );

        m_kcmSettings.writeEntry( prefixCurrencySymbolKey, prefixCurrencySymbol );
        m_kcmSettings.writeEntry( signPositionKey, (int) signPosition );

        // If the new value is not the default (i.e. is set in user), then save it and enable the default button
        if ( prefixCurrencySymbol != m_defaultSettings.readEntry( prefixCurrencySymbolKey, false ) ) {
            m_userSettings.writeEntry( prefixCurrencySymbolKey, prefixCurrencySymbol, KConfig::Persistent | KConfig::Global );
            formatDefaultButton->setEnabled( true );
        } else {  // Is the default so delete any user setting
            m_userSettings.deleteEntry( prefixCurrencySymbolKey, KConfig::Persistent | KConfig::Global );
        }

        // If the new value is not the default (i.e. is set in user), then save it and enable the default button
        if ( signPosition != m_defaultSettings.readEntry( signPositionKey, 0 ) ) {
            m_userSettings.writeEntry( signPositionKey, (int) signPosition, KConfig::Persistent | KConfig::Global );
            formatDefaultButton->setEnabled( true );
        } else {  // Is the default so delete any user setting
            m_userSettings.deleteEntry( signPositionKey, KConfig::Persistent | KConfig::Global );
        }

        checkIfChanged();
    }
}

void KCMLocale::setMonetaryPositiveFormat( bool prefixCurrencySymbol, KLocale::SignPosition signPosition )
{
    setMonetaryFormat( "PositivePrefixCurrencySymbol", prefixCurrencySymbol,
                       "PositiveMonetarySignPosition", signPosition,
                       m_ui->m_comboMonetaryPositiveFormat, m_ui->m_buttonDefaultMonetaryPositiveFormat );

    // Read back the kcm values and use them in the sample locale
    prefixCurrencySymbol = m_kcmSettings.readEntry( "PositivePrefixCurrencySymbol", false );
    signPosition = (KLocale::SignPosition) m_kcmSettings.readEntry( "PositiveMonetarySignPosition", 0 );
    m_kcmLocale->setPositivePrefixCurrencySymbol( prefixCurrencySymbol );
    m_kcmLocale->setPositiveMonetarySignPosition( signPosition );

    // Set the combo to the kcm value
    QVariantList options;
    options.append( QVariant( prefixCurrencySymbol ) );
    options.append( QVariant( signPosition ) );
    int index = m_ui->m_comboMonetaryPositiveFormat->findData( options );
    m_ui->m_comboMonetaryPositiveFormat->setCurrentIndex( index );
}

void KCMLocale::insertMonetaryNegativeFormat( bool prefixCurrencySymbol, KLocale::SignPosition signPosition )
{
    KLocale custom( *m_kcmLocale );
    custom.setNegativePrefixCurrencySymbol( prefixCurrencySymbol );
    custom.setNegativeMonetarySignPosition( signPosition );
    QVariantList options;
    options.append( QVariant( prefixCurrencySymbol ) );
    options.append( QVariant( signPosition ) );
    m_ui->m_comboMonetaryNegativeFormat->addItem( custom.formatMoney( -123456.78 ), options );
}

void KCMLocale::initMonetaryNegativeFormat()
{
    m_ui->m_comboMonetaryNegativeFormat->blockSignals( true );

    m_ui->m_labelMonetaryNegativeFormat->setText( ki18n( "Negative format:" ).toString() );
    QString helpText = ki18n( "<p>Here you can set the format of negative monetary values.</p>"
                              "<p>Note that the negative sign used to display other numbers has "
                              "to be defined separately (see the 'Numbers' tab).</p>" ).toString();
    m_ui->m_comboMonetaryNegativeFormat->setToolTip( helpText );
    m_ui->m_comboMonetaryNegativeFormat->setWhatsThis( helpText );

    QString format = ki18n( "Sign position:" ).toString();
    format = ki18n( "Parentheses Around" ).toString();
    format = ki18n( "Before Quantity Money" ).toString();
    format = ki18n( "After Quantity Money" ).toString();
    format = ki18n( "Before Money" ).toString();
    format = ki18n( "After Money" ).toString();
    format = ki18n( "Here you can select how a negative sign will "
                    "be positioned. This only affects monetary "
                    "values." ).toString();

    QString check = ki18n( "Prefix currency symbol" ).toString();
    check = ki18n( "If this option is checked, the currency sign "
                    "will be prefixed (i.e. to the left of the "
                    "value) for all negative monetary values. If "
                    "not, it will be postfixed (i.e. to the right)." ).toString();

    m_ui->m_comboMonetaryNegativeFormat->clear();
    // If the negative sign is null, then all the sign options will look the same, so only show a
    // choice between parens and no sign, but preserve original position in case they do set sign
    // Also keep options in same order, i.e. sign options then parens option
    if ( m_kcmSettings.readEntry( "NegativeSign", QString() ).isEmpty() ) {
        KLocale::SignPosition currentSignPosition = (KLocale::SignPosition) m_currentSettings.readEntry( "NegativeMonetarySignPosition", 0 );
        KLocale::SignPosition kcmSignPosition = (KLocale::SignPosition) m_kcmSettings.readEntry( "NegativeMonetarySignPosition", 0 );
        if ( currentSignPosition == KLocale::ParensAround && kcmSignPosition == KLocale::ParensAround ) {
            //Both are parens, so also give a sign option
            insertMonetaryNegativeFormat( true, KLocale::BeforeQuantityMoney );
            insertMonetaryNegativeFormat( false, KLocale::BeforeQuantityMoney );
            insertMonetaryNegativeFormat( true, kcmSignPosition );
            insertMonetaryNegativeFormat( false, kcmSignPosition );
        } else if ( kcmSignPosition == KLocale::ParensAround ) {
            //kcm is parens, current is sign, use both in right order
            insertMonetaryNegativeFormat( true, currentSignPosition );
            insertMonetaryNegativeFormat( false, currentSignPosition );
            insertMonetaryNegativeFormat( true, kcmSignPosition );
            insertMonetaryNegativeFormat( false, kcmSignPosition );
        } else {
            // kcm is sign, current is parens, use both in right order
            insertMonetaryNegativeFormat( true, kcmSignPosition );
            insertMonetaryNegativeFormat( false, kcmSignPosition );
            insertMonetaryNegativeFormat( true, currentSignPosition );
            insertMonetaryNegativeFormat( false, currentSignPosition );
        }
    } else {
        // Show the sign options first, then parens
        insertMonetaryNegativeFormat( true, KLocale::BeforeQuantityMoney );
        insertMonetaryNegativeFormat( false, KLocale::BeforeQuantityMoney );
        insertMonetaryNegativeFormat( true, KLocale::AfterQuantityMoney );
        insertMonetaryNegativeFormat( false, KLocale::AfterQuantityMoney );
        insertMonetaryNegativeFormat( true, KLocale::BeforeMoney );
        insertMonetaryNegativeFormat( false, KLocale::BeforeMoney );
        insertMonetaryNegativeFormat( true, KLocale::AfterMoney );
        insertMonetaryNegativeFormat( false, KLocale::AfterMoney );
        insertMonetaryNegativeFormat( true, KLocale::ParensAround );
        insertMonetaryNegativeFormat( false, KLocale::ParensAround );
    }

    setMonetaryNegativeFormat( m_kcmSettings.readEntry( "NegativePrefixCurrencySymbol", false ),
                               (KLocale::SignPosition) m_defaultSettings.readEntry( "NegativeMonetarySignPosition", 0 ) );

    m_ui->m_comboMonetaryNegativeFormat->blockSignals( false );
}

void KCMLocale::defaultMonetaryNegativeFormat()
{
    setMonetaryNegativeFormat( m_defaultSettings.readEntry( "NegativePrefixCurrencySymbol", false ),
                               (KLocale::SignPosition) m_defaultSettings.readEntry( "NegativeMonetarySignPosition", 0 ) );
}

void KCMLocale::changedMonetaryNegativeFormatIndex( int index )
{
    QVariantList options = m_ui->m_comboMonetaryNegativeFormat->itemData( index ).toList();
    bool prefixCurrencySymbol = options.at( 0 ).toBool();
    KLocale::SignPosition signPosition = (KLocale::SignPosition) options.at( 1 ).toInt();
    setMonetaryNegativeFormat( prefixCurrencySymbol, signPosition );
}

void KCMLocale::setMonetaryNegativeFormat( bool prefixCurrencySymbol, KLocale::SignPosition signPosition )
{
    setMonetaryFormat( "NegativePrefixCurrencySymbol", prefixCurrencySymbol,
                       "NegativeMonetarySignPosition", signPosition,
                       m_ui->m_comboMonetaryNegativeFormat, m_ui->m_buttonDefaultMonetaryNegativeFormat );

    // Read back the kcm values and use them in the sample locale
    prefixCurrencySymbol = m_kcmSettings.readEntry( "NegativePrefixCurrencySymbol", false );
    signPosition = (KLocale::SignPosition) m_kcmSettings.readEntry( "NegativeMonetarySignPosition", 0 );
    m_kcmLocale->setNegativePrefixCurrencySymbol( prefixCurrencySymbol );
    m_kcmLocale->setNegativeMonetarySignPosition( signPosition );

    // Set the combo to the kcm value
    QVariantList options;
    options.append( QVariant( prefixCurrencySymbol ) );
    options.append( QVariant( signPosition ) );
    int index = m_ui->m_comboMonetaryNegativeFormat->findData( options );
    m_ui->m_comboMonetaryNegativeFormat->setCurrentIndex( index );

    updateSample();
}

void KCMLocale::initMonetaryDigitSet()
{
    m_ui->m_comboMonetaryDigitSet->blockSignals( true );

    m_ui->m_labelMonetaryDigitSet->setText( ki18n( "Digit set:" ).toString() );
    QString helpText = ki18n( "<p>Here you can define the set of digits used to display monetary "
                              "values. If digits other than Arabic are selected, they will appear "
                              "only if used in the language of the application or the piece of "
                              "text where the number is shown.</p><p>Note that the set of digits "
                              "used to display other numbers has to be defined separately (see the "
                              "'Numbers' tab).</p>" ).toString();
    m_ui->m_comboMonetaryDigitSet->setToolTip( helpText );
    m_ui->m_comboMonetaryDigitSet->setWhatsThis( helpText );

    initDigitSetCombo( m_ui->m_comboMonetaryDigitSet );

    setMonetaryDigitSet( m_kcmSettings.readEntry( "MonetaryDigitSet", 0 ) );

    m_ui->m_comboMonetaryDigitSet->blockSignals( false );
}

void KCMLocale::defaultMonetaryDigitSet()
{
    setMonetaryDigitSet( m_defaultSettings.readEntry( "MonetaryDigitSet", 0 ) );
}

void KCMLocale::changedMonetaryDigitSetIndex( int index )
{
    setMonetaryDigitSet( m_ui->m_comboMonetaryDigitSet->itemData( index ).toInt() );
}

void KCMLocale::setMonetaryDigitSet( int newValue )
{
    setComboItem( "MonetaryDigitSet", newValue,
                  m_ui->m_comboMonetaryDigitSet, m_ui->m_buttonDefaultMonetaryDigitSet );
    m_kcmLocale->setMonetaryDigitSet( (KLocale::DigitSet) m_kcmSettings.readEntry( "MonetaryDigitSet", 0 ) );

    // Update the monetary format samples to relect new setting
    initMonetaryDigitGrouping();
    initMonetaryPositiveFormat();
    initMonetaryNegativeFormat();
}

void KCMLocale::initCalendarSystem()
{
    m_ui->m_comboCalendarSystem->blockSignals( true );

    m_ui->m_labelCalendarSystem->setText( ki18n( "Calendar system:" ).toString() );
    QString helpText = ki18n( "<p>Here you can set the Calendar System to use to display dates.</p>" ).toString();
    m_ui->m_comboCalendarSystem->setToolTip( helpText );
    m_ui->m_comboCalendarSystem->setWhatsThis( helpText );

    m_ui->m_comboCalendarSystem->clear();

//     QStringList calendarSystems = KCalendarSystem::calendarSystems();
//
//     foreach ( const QString &calendarType, calendarSystems ) {
//         m_ui->m_comboCalendarSystem->addItem( KCalendarSystem::calendarLabel(
//                                               KCalendarSystem::calendarSystemForCalendarType( calendarType ), m_kcmLocale ),
//                                               QVariant( calendarType ) );
//     }
#warning "FIXME: Port calendar sytems"
    setCalendarSystem( m_kcmSettings.readEntry( "CalendarSystem", QString() ) );

    m_ui->m_comboCalendarSystem->blockSignals( false );
}

void KCMLocale::defaultCalendarSystem()
{
    setCalendarSystem( m_defaultSettings.readEntry( "CalendarSystem", QString() ) );
}

void KCMLocale::changedCalendarSystemIndex( int index )
{
    setCalendarSystem( m_ui->m_comboCalendarSystem->itemData( index ).toString() );
}

void KCMLocale::setCalendarSystem( const QString &newValue )
{
    setComboItem( "CalendarSystem", newValue,
                  m_ui->m_comboCalendarSystem, m_ui->m_buttonDefaultCalendarSystem );

    // Load the correct settings group for the new calendar
    initCalendarSettings();
    mergeCalendarSettings();

    // If item was changed, i.e. not immutable, then update locale
    m_kcmLocale->setCalendar( m_kcmSettings.readEntry( "CalendarSystem", QString() ) );

    // Update the Calendar dependent widgets with the new Calendar System details
    initUseCommonEra();
    initShortYearWindow();
    initWeekNumberSystem();
    initWeekStartDay();
    initWorkingWeekStartDay();
    initWorkingWeekEndDay();
    initWeekDayOfPray();
    updateSample();
}

void KCMLocale::initUseCommonEra()
{
    m_ui->m_checkCalendarGregorianUseCommonEra->blockSignals( true );

    m_ui->m_checkCalendarGregorianUseCommonEra->setText( ki18n( "Use Common Era" ).toString() );
    QString helpText = ki18n( "<p>This option determines if the Common Era (CE/BCE) should be used "
                              "instead of the Christian Era (AD/BC).</p>" ).toString();
    m_ui->m_checkCalendarGregorianUseCommonEra->setToolTip( helpText );
    m_ui->m_checkCalendarGregorianUseCommonEra->setWhatsThis( helpText );

    QString calendarType = m_kcmSettings.readEntry( "CalendarSystem", QString() );
    if ( calendarType == "gregorian" || calendarType == "gregorian-proleptic" ) {
        setUseCommonEra( m_kcmCalendarSettings.readEntry( "UseCommonEra", false ) );
    } else {
        setUseCommonEra( false );
        m_ui->m_checkCalendarGregorianUseCommonEra->setEnabled( false );
        m_ui->m_buttonDefaultCalendarGregorianUseCommonEra->setEnabled( false );
    }

    m_ui->m_checkCalendarGregorianUseCommonEra->blockSignals( false );
}

void KCMLocale::defaultUseCommonEra()
{
    setUseCommonEra( m_defaultCalendarSettings.readEntry( "UseCommonEra", false ) );
}

void KCMLocale::changedUseCommonEra( bool newValue )
{
    setUseCommonEra( newValue );
}

void KCMLocale::setUseCommonEra( bool newValue )
{
    setCalendarItem( "UseCommonEra", newValue,
                     m_ui->m_checkCalendarGregorianUseCommonEra, m_ui->m_buttonDefaultCalendarGregorianUseCommonEra );
    m_ui->m_checkCalendarGregorianUseCommonEra->setChecked( m_kcmCalendarSettings.readEntry( "UseCommonEra", false ) );

    // No api to set, so need to force reload the locale
    m_kcmConfig->markAsClean();
    m_kcmLocale->setCountry( m_kcmSettings.readEntry( "Country", QString() ), m_kcmConfig.data() );
    m_kcmLocale->setCalendar( m_kcmSettings.readEntry( "CalendarSystem", QString() ) );
}

void KCMLocale::initShortYearWindow()
{
    m_ui->m_intShortYearWindowStartYear->blockSignals( true );

    m_ui->m_labelShortYearWindow->setText( ki18n( "Short year window:" ).toString() );
    m_ui->m_labelShortYearWindowTo->setText( ki18nc( "label between two year inputs, i.e. 1930 to 2029", "to" ).toString() );
    QString helpText = ki18n( "<p>This option determines what year range a two digit date is "
                              "interpreted as, for example with a range of 1950 to 2049 the "
                              "value 10 is interpreted as 2010.  This range is only applied when "
                              "reading the Short Year (YY) date format.</p>" ).toString();
    m_ui->m_intShortYearWindowStartYear->setToolTip( helpText );
    m_ui->m_intShortYearWindowStartYear->setWhatsThis( helpText );
    m_ui->m_spinShortYearWindowEndYear->setToolTip( helpText );
    m_ui->m_spinShortYearWindowEndYear->setWhatsThis( helpText );

    setShortYearWindow( m_kcmCalendarSettings.readEntry( "ShortYearWindowStartYear", 0 ) );

    m_ui->m_intShortYearWindowStartYear->blockSignals( false );
}

void KCMLocale::defaultShortYearWindow()
{
    setShortYearWindow( m_defaultCalendarSettings.readEntry( "ShortYearWindowStartYear", 0 ) );
}

void KCMLocale::changedShortYearWindow( int newValue )
{
    setShortYearWindow( newValue );
}

void KCMLocale::setShortYearWindow( int newValue )
{
    setCalendarItem( "ShortYearWindowStartYear", newValue,
                     m_ui->m_intShortYearWindowStartYear, m_ui->m_buttonDefaultShortYearWindow );
    int startYear = m_kcmCalendarSettings.readEntry( "ShortYearWindowStartYear", 0 );
    m_ui->m_intShortYearWindowStartYear->setValue( startYear );
    m_ui->m_spinShortYearWindowEndYear->setValue( startYear + 99 );

    // No api to set, so need to force reload the locale and calendar
    m_kcmConfig->markAsClean();
    m_kcmLocale->setCountry( m_kcmSettings.readEntry( "Country", QString() ), m_kcmConfig.data() );
    m_kcmLocale->setCalendar( m_kcmSettings.readEntry( "CalendarSystem", QString() ) );
}

void KCMLocale::initWeekNumberSystem()
{
    m_ui->m_comboWeekNumberSystem->blockSignals( true );

    m_ui->m_labelWeekNumberSystem->setText( ki18n( "Week number system:" ).toString() );
    QString helpText = ki18n( "<p>This option determines how the Week Number will be calculated. "
                              "There are four options available:</p> "
                              "<ul>"
                              "<li><b>ISO Week</b> Use the ISO standard Week Number. This will always "
                              "use Monday as the first day of the ISO week. This is the most commonly "
                              "used system.</li>"
                              "<li><b>Full First Week</b> The first week of the year starts on the "
                              "first occurrence of the <i>First day of the week</i>, and lasts for "
                              "seven days.  Any days before Week 1 are considered part of the last "
                              "week of the previous year. This system is most commonly used in the "
                              "USA.</li>"
                              "<li><b>Partial First Week</b> The first week starts on the first day "
                              "of the year. The second week of the year starts on the first "
                              "occurrence of the <i>First day of the week</i>, and lasts for "
                              "seven days. The first week may not contain seven days.</li>"
                              "<li><b>Simple Week</b> The first week starts on the first day of the "
                              "year and lasts seven days, with all new weeks starting on the same "
                              "weekday as the first day of the year.</li>"
                              "</ul>"
                            ).toString();
    m_ui->m_comboWeekNumberSystem->setToolTip( helpText );
    m_ui->m_comboWeekNumberSystem->setWhatsThis( helpText );

    m_ui->m_comboWeekNumberSystem->clear();
    m_ui->m_comboWeekNumberSystem->addItem( ki18n( "ISO Week" ).toString(),
                                            QVariant( KLocale::IsoWeekNumber ) );
    m_ui->m_comboWeekNumberSystem->addItem( ki18n( "Full First Week" ).toString(),
                                            QVariant( KLocale::FirstFullWeek ) );
    m_ui->m_comboWeekNumberSystem->addItem( ki18n( "Partial First Week" ).toString(),
                                            QVariant( KLocale::FirstPartialWeek ) );
    m_ui->m_comboWeekNumberSystem->addItem( ki18n( "Simple Week" ).toString(),
                                            QVariant( KLocale::SimpleWeek ) );

    setWeekNumberSystem( (KLocale::WeekNumberSystem) m_kcmSettings.readEntry( "WeekNumberSystem", 0 ) );

    m_ui->m_comboWeekNumberSystem->blockSignals( false );
}

void KCMLocale::defaultWeekNumberSystem()
{
    setWeekNumberSystem( m_defaultSettings.readEntry( "WeekNumberSystem", 0 ) );
}

void KCMLocale::changedWeekNumberSystemIndex( int index )
{
    setWeekNumberSystem( m_ui->m_comboWeekNumberSystem->itemData( index ).toInt() );
}

void KCMLocale::setWeekNumberSystem( int newValue )
{
    setComboItem( "WeekNumberSystem", newValue,
                  m_ui->m_comboWeekNumberSystem, m_ui->m_buttonDefaultWeekNumberSystem );
    m_kcmLocale->setWeekNumberSystem( (KLocale::WeekNumberSystem) m_kcmSettings.readEntry( "WeekNumberSystem", 0 ) );
}

void KCMLocale::initWeekStartDay()
{
    m_ui->m_comboWeekStartDay->blockSignals( true );

    m_ui->m_labelWeekStartDay->setText( ki18n( "First day of week:" ).toString() );
    QString helpText = ki18n( "<p>This option determines which day will be considered as the first "
                              "one of the week.  This value may affect the Week Number System.</p> "
                            ).toString();
    m_ui->m_comboWeekStartDay->setToolTip( helpText );
    m_ui->m_comboWeekStartDay->setWhatsThis( helpText );

    initWeekDayCombo( m_ui->m_comboWeekStartDay );

    setWeekStartDay( m_kcmSettings.readEntry( "WeekStartDay", 0 ) );

    m_ui->m_comboWeekStartDay->blockSignals( false );
}

void KCMLocale::defaultWeekStartDay()
{
    setWeekStartDay( m_defaultSettings.readEntry( "WeekStartDay", 0 ) );
}

void KCMLocale::changedWeekStartDayIndex( int index )
{
    setWeekStartDay( m_ui->m_comboWeekStartDay->itemData( index ).toInt() );
}

void KCMLocale::setWeekStartDay( int newValue )
{
    setComboItem( "WeekStartDay", newValue,
                  m_ui->m_comboWeekStartDay, m_ui->m_buttonDefaultWeekStartDay );
    m_kcmLocale->setWeekStartDay( m_kcmSettings.readEntry( "WeekStartDay", 0 ) );
}

void KCMLocale::initWorkingWeekStartDay()
{
    m_ui->m_comboWorkingWeekStartDay->blockSignals( true );

    m_ui->m_labelWorkingWeekStartDay->setText( ki18n( "First working day of week:" ).toString() );
    QString helpText = ki18n( "<p>This option determines which day will be considered as the first "
                              "working day of the week.</p>" ).toString();
    m_ui->m_comboWorkingWeekStartDay->setToolTip( helpText );
    m_ui->m_comboWorkingWeekStartDay->setWhatsThis( helpText );

    initWeekDayCombo( m_ui->m_comboWorkingWeekStartDay );

    setWorkingWeekStartDay( m_kcmSettings.readEntry( "WorkingWeekStartDay", 0 ) );

    m_ui->m_comboWorkingWeekStartDay->blockSignals( false );
}

void KCMLocale::defaultWorkingWeekStartDay()
{
    setWorkingWeekStartDay( m_defaultSettings.readEntry( "WorkingWeekStartDay", 0 ) );
}

void KCMLocale::changedWorkingWeekStartDayIndex( int index )
{
    setWorkingWeekStartDay( m_ui->m_comboWorkingWeekStartDay->itemData( index ).toInt() );
}

void KCMLocale::setWorkingWeekStartDay( int newValue )
{
    setComboItem( "WorkingWeekStartDay", newValue,
                  m_ui->m_comboWorkingWeekStartDay, m_ui->m_buttonDefaultWorkingWeekStartDay );
    m_kcmLocale->setWorkingWeekStartDay( m_kcmSettings.readEntry( "WorkingWeekStartDay", 0 ) );
}

void KCMLocale::initWorkingWeekEndDay()
{
    m_ui->m_comboWorkingWeekEndDay->blockSignals( true );

    m_ui->m_labelWorkingWeekEndDay->setText( ki18n( "Last working day of week:" ).toString() );
    QString helpText = ki18n( "<p>This option determines which day will be considered as the last "
                              "working day of the week.</p>" ).toString();
    m_ui->m_comboWorkingWeekEndDay->setToolTip( helpText );
    m_ui->m_comboWorkingWeekEndDay->setWhatsThis( helpText );

    initWeekDayCombo( m_ui->m_comboWorkingWeekEndDay );

    setWorkingWeekEndDay( m_kcmSettings.readEntry( "WorkingWeekEndDay", 0 ) );

    m_ui->m_comboWorkingWeekEndDay->blockSignals( false );
}

void KCMLocale::defaultWorkingWeekEndDay()
{
    setWorkingWeekEndDay( m_defaultSettings.readEntry( "WorkingWeekEndDay", 0 ) );
}

void KCMLocale::changedWorkingWeekEndDayIndex( int index )
{
    setWorkingWeekEndDay( m_ui->m_comboWorkingWeekEndDay->itemData( index ).toInt() );
}

void KCMLocale::setWorkingWeekEndDay( int newValue )
{
    setComboItem( "WorkingWeekEndDay", newValue,
                  m_ui->m_comboWorkingWeekEndDay, m_ui->m_buttonDefaultWorkingWeekEndDay );
    m_kcmLocale->setWorkingWeekEndDay( m_kcmSettings.readEntry( "WorkingWeekEndDay", 0 ) );
}

void KCMLocale::initWeekDayOfPray()
{
    m_ui->m_comboWeekDayOfPray->blockSignals( true );

    m_ui->m_labelWeekDayOfPray->setText( ki18n( "Week day for special religious observance:" ).toString() );
    QString helpText = ki18n( "<p>This option determines which day if any will be considered as "
                              "the day of the week for special religious observance.</p>" ).toString();
    m_ui->m_comboWeekDayOfPray->setToolTip( helpText );
    m_ui->m_comboWeekDayOfPray->setWhatsThis( helpText );

    initWeekDayCombo( m_ui->m_comboWeekDayOfPray );
    m_ui->m_comboWeekDayOfPray->insertItem( 0, ki18nc( "Day name list, option for no special day of religious observance", "None / None in particular" ).toString() );

    setWeekDayOfPray( m_kcmSettings.readEntry( "WeekDayOfPray", 0 ) );

    m_ui->m_comboWeekDayOfPray->blockSignals( false );
}

void KCMLocale::defaultWeekDayOfPray()
{
    setWeekDayOfPray( m_defaultSettings.readEntry( "WeekDayOfPray", 0 ) );
}

void KCMLocale::changedWeekDayOfPrayIndex( int index )
{
    setWeekDayOfPray( m_ui->m_comboWeekDayOfPray->itemData( index ).toInt() );
}

void KCMLocale::setWeekDayOfPray( int newValue )
{
    setComboItem( "WeekDayOfPray", newValue,
                  m_ui->m_comboWeekDayOfPray, m_ui->m_buttonDefaultWeekDayOfPray );
    m_kcmLocale->setWeekDayOfPray( m_kcmSettings.readEntry( "WeekDayOfPray", 0 ) );
}

void KCMLocale::initTimeFormat()
{
    m_ui->m_comboTimeFormat->blockSignals( true );

    m_ui->m_labelTimeFormat->setText( ki18n( "Time format:" ).toString() );
    QString helpText = ki18n( "<p>The text in this textbox will be used to format time strings. "
                              "The sequences below will be replaced:</p>"
                              "<table>"
                              "<tr><td><b>HH</b></td>"
                                  "<td>The hour as a decimal number using a 24-hour clock (00-23).</td></tr>"
                              "<tr><td><b>hH</b></td>"
                                  "<td>The hour (24-hour clock) as a decimal number (0-23).</td></tr>"
                              "<tr><td><b>PH</b></td>"
                                  "<td>The hour as a decimal number using a 12-hour clock (01-12).</td></tr>"
                              "<tr><td><b>pH</b></td>"
                                  "<td>The hour (12-hour clock) as a decimal number (1-12).</td></tr>"
                              "<tr><td><b>MM</b></td>"
                                  "<td>The minutes as a decimal number (00-59).</td></tr>"
                              "<tr><td><b>SS</b></td>"
                                  "<td>The seconds as a decimal number (00-59).</td></tr>"
                              "<tr><td><b>AMPM</b></td>"
                                  "<td>Either 'AM' or 'PM' according to the given time value. "
                                      "Noon is treated as 'PM' and midnight as 'AM'.</td></tr>"
                              "</table>" ).toString();
    m_ui->m_comboTimeFormat->setToolTip( helpText );
    m_ui->m_comboTimeFormat->setWhatsThis( helpText );

    m_timeFormatMap.clear();
    m_timeFormatMap.insert( QString( 'H' ), ki18n( "HH" ).toString() );
    m_timeFormatMap.insert( QString( 'k' ), ki18n( "hH" ).toString() );
    m_timeFormatMap.insert( QString( 'I' ), ki18n( "PH" ).toString() );
    m_timeFormatMap.insert( QString( 'l' ), ki18n( "pH" ).toString() );
    m_timeFormatMap.insert( QString( 'M' ), ki18nc( "Minute", "MM" ).toString() );
    m_timeFormatMap.insert( QString( 'S' ), ki18n( "SS" ).toString() );
    m_timeFormatMap.insert( QString( 'p' ), ki18n( "AMPM" ).toString() );

    QStringList formatList;
    QString cValue = m_cSettings.readEntry( "TimeFormat", QString() );
    formatList.append( posixToUserTime( m_kcmSettings.readEntry( "TimeFormat", cValue ) ) );
    formatList.append( posixToUserTime( m_defaultSettings.readEntry( "TimeFormat", cValue ) ) );
    formatList.append( posixToUserTime( m_countrySettings.readEntry( "TimeFormat", cValue ) ) );
    formatList.append( posixToUserTime( cValue ) );
    // TODO convert these to POSIX and US format!
    QString formats =ki18nc( "some reasonable time formats for the language",
                             "HH:MM:SS\n"
                             "pH:MM:SS AMPM").toString();
    formatList.append( formats.split( QString::fromLatin1("\n") ) );
    formatList.removeDuplicates();
    m_ui->m_comboTimeFormat->clear();
    m_ui->m_comboTimeFormat->addItems( formatList );

    setTimeFormat( m_kcmSettings.readEntry( "TimeFormat", QString() ) );

    m_ui->m_comboTimeFormat->blockSignals( false );
}

void KCMLocale::defaultTimeFormat()
{
    setTimeFormat( m_defaultSettings.readEntry( "TimeFormat", QString() ) );
}

void KCMLocale::changedTimeFormat( const QString &newValue )
{
    setItem( "TimeFormat", userToPosixTime( newValue ),
             m_ui->m_comboTimeFormat, m_ui->m_buttonDefaultTimeFormat );
    m_kcmLocale->setTimeFormat( m_kcmSettings.readEntry( "TimeFormat", QString() ) );
    updateSample();
}

void KCMLocale::setTimeFormat( const QString &newValue )
{
    setItem( "TimeFormat", newValue,
             m_ui->m_comboTimeFormat, m_ui->m_buttonDefaultTimeFormat );
    QString value = m_kcmSettings.readEntry( "TimeFormat", QString() );
    m_ui->m_comboTimeFormat->setEditText( posixToUserTime( value ) );
    m_kcmLocale->setTimeFormat( value );
    updateSample();
}

QString KCMLocale::dayPeriodText( const QString &dayPeriod )
{
    // If you get here with an empty dayPeriod, this is not the cause, merely a symptom of the
    // real bug.  You need to find out why it is empty otherwise time parse/format will break.
    Q_ASSERT( !dayPeriod.isEmpty() );
    return dayPeriod.isEmpty() ? QString() : dayPeriod.split( QChar::fromLatin1(',') ).at( 2 );
}

QString KCMLocale::amPeriod( const QString &longName, const QString &shortName, const QString &narrowName )
{
    QStringList dayPeriod;
    dayPeriod.append( QString::fromLatin1("am") );
    dayPeriod.append( longName );
    dayPeriod.append( shortName );
    dayPeriod.append( narrowName );
    dayPeriod.append( QTime( 0, 0, 0 ).toString( QString::fromLatin1("HH:mm:ss.zzz") ) );
    dayPeriod.append( QTime( 11, 59, 59, 999 ).toString( QString::fromLatin1("HH:mm:ss.zzz") ) );
    dayPeriod.append( QChar::fromLatin1('0') );
    dayPeriod.append( QString::fromLatin1("12") );
    return dayPeriod.join( QChar::fromLatin1(',') );
}

QString KCMLocale::pmPeriod( const QString &longName, const QString &shortName, const QString &narrowName )
{
    QStringList dayPeriod;
    dayPeriod.append( QString::fromLatin1("pm") );
    dayPeriod.append( longName );
    dayPeriod.append( shortName );
    dayPeriod.append( narrowName );
    dayPeriod.append( QTime( 12, 0, 0 ).toString( QString::fromLatin1("HH:mm:ss.zzz") ) );
    dayPeriod.append( QTime( 23, 59, 59, 999 ).toString( QString::fromLatin1("HH:mm:ss.zzz") ) );
    dayPeriod.append( QChar::fromLatin1('0') );
    dayPeriod.append( QString::fromLatin1("12") );
    return dayPeriod.join( QChar::fromLatin1(',') );
}

void KCMLocale::initAmPmSymbols()
{
    m_ui->m_comboAmSymbol->blockSignals( true );
    m_ui->m_comboPmSymbol->blockSignals( true );

    m_ui->m_labelAmSymbol->setText( ki18n( "AM symbol:" ).toString() );
    QString helpText = ki18n( "<p>Here you can set the text to be displayed for AM.</p>" ).toString();
    m_ui->m_comboAmSymbol->setToolTip( helpText );
    m_ui->m_comboAmSymbol->setWhatsThis( helpText );

    m_ui->m_labelPmSymbol->setText( ki18n( "PM symbol:" ).toString() );
    helpText = ki18n( "<p>Here you can set the text to be displayed for PM.</p>" ).toString();
    m_ui->m_comboPmSymbol->setToolTip( helpText );
    m_ui->m_comboPmSymbol->setWhatsThis( helpText );

    QStringList formatList;
    formatList.append( m_kcmLocale->dayPeriodText( QTime( 0, 0, 0 ) ) );
    formatList.append( m_defaultLocale->dayPeriodText( QTime( 0, 0, 0 ) ) );
    formatList.removeDuplicates();
    m_ui->m_comboAmSymbol->clear();
    m_ui->m_comboAmSymbol->addItems( formatList );

    formatList.clear();
    formatList.append( m_kcmLocale->dayPeriodText( QTime( 12, 0, 0 ) ) );
    formatList.append( m_defaultLocale->dayPeriodText( QTime( 12, 0, 0 ) ) );
    formatList.removeDuplicates();
    m_ui->m_comboPmSymbol->clear();
    m_ui->m_comboPmSymbol->addItems( formatList );

    setAmPmPeriods( m_kcmSettings.readEntry( "DayPeriod1", QString() ),
                    m_kcmSettings.readEntry( "DayPeriod2", QString() ) );

    m_ui->m_comboAmSymbol->setEditText( dayPeriodText( m_kcmSettings.readEntry( "DayPeriod1", QString() ) ) );
    m_ui->m_comboPmSymbol->setEditText( dayPeriodText( m_kcmSettings.readEntry( "DayPeriod2", QString() ) ) );

    m_ui->m_comboAmSymbol->blockSignals( false );
    m_ui->m_comboPmSymbol->blockSignals( false );
}

void KCMLocale::setAmPmPeriods( const QString &amPeriod, const QString &pmPeriod )
{
    // If either setting is locked down by Kiosk, then don't let the user make any changes, and disable the widgets
    if ( m_userSettings.isEntryImmutable( "DayPeriod1" ) ||
         m_userSettings.isEntryImmutable( "DayPeriod2" ) ) {
        m_ui->m_comboAmSymbol->setEnabled( false );
        m_ui->m_buttonDefaultAmSymbol->setEnabled( false );
        m_ui->m_comboPmSymbol->setEnabled( false );
        m_ui->m_buttonDefaultPmSymbol->setEnabled( false );
    } else {
        m_ui->m_comboAmSymbol->setEnabled( true );
        m_ui->m_comboPmSymbol->setEnabled( true );
        m_ui->m_buttonDefaultAmSymbol->setEnabled( false );
        m_ui->m_buttonDefaultPmSymbol->setEnabled( false );

        m_kcmSettings.writeEntry( "DayPeriod1", amPeriod );
        m_kcmSettings.writeEntry( "DayPeriod2", pmPeriod );

        // If either value is not the default then both values must be set in the user settings
        if ( amPeriod != m_defaultSettings.readEntry( "DayPeriod1", QString() ) ||
             pmPeriod != m_defaultSettings.readEntry( "DayPeriod2", QString() ) ) {
            m_userSettings.writeEntry( "DayPeriod1", amPeriod, KConfig::Persistent | KConfig::Global );
            m_userSettings.writeEntry( "DayPeriod2", pmPeriod, KConfig::Persistent | KConfig::Global );
        } else {  // Is the default so delete any user setting
            m_userSettings.deleteEntry( "DayPeriod1", KConfig::Persistent | KConfig::Global );
            m_userSettings.deleteEntry( "DayPeriod2", KConfig::Persistent | KConfig::Global );
        }

        if ( m_kcmSettings.readEntry( "DayPeriod1", QString() ) !=
            m_defaultSettings.readEntry( "DayPeriod1", QString() ) ) {
            m_ui->m_buttonDefaultAmSymbol->setEnabled( true );
        }
        if ( m_kcmSettings.readEntry( "DayPeriod2", QString() ) !=
            m_defaultSettings.readEntry( "DayPeriod2", QString() ) ) {
            m_ui->m_buttonDefaultPmSymbol->setEnabled( true );
        }

        checkIfChanged();

        // No api to set, so need to force reload the locale
        m_kcmConfig->markAsClean();
        m_kcmLocale->setCountry( m_kcmSettings.readEntry( "Country", QString() ), m_kcmConfig.data() );
        m_kcmLocale->setCalendar( m_kcmSettings.readEntry( "CalendarSystem", QString() ) );
    }

    updateSample();
}

void KCMLocale::defaultAmSymbol()
{
    setAmPmPeriods( m_defaultSettings.readEntry( "DayPeriod1", QString() ),
                    m_kcmSettings.readEntry( "DayPeriod2", QString() ) );
    m_ui->m_comboAmSymbol->setEditText( dayPeriodText( m_kcmSettings.readEntry( "DayPeriod1", QString() ) ) );
}

void KCMLocale::changedAmSymbol( const QString &newValue )
{
    QStringList dayPeriod = m_defaultSettings.readEntry( "DayPeriod1", QString() ).split(',');
    dayPeriod[2] = newValue;
    setAmPmPeriods( dayPeriod.join( QChar::fromLatin1(',') ), m_kcmSettings.readEntry( "DayPeriod2", QString() ) );
}

void KCMLocale::defaultPmSymbol()
{
    setAmPmPeriods( m_kcmSettings.readEntry( "DayPeriod1", QString() ),
                    m_defaultSettings.readEntry( "DayPeriod2", QString() ) );
    m_ui->m_comboPmSymbol->setEditText( dayPeriodText( m_kcmSettings.readEntry( "DayPeriod2", QString() ) ) );
}

void KCMLocale::changedPmSymbol( const QString &newValue )
{
    QStringList dayPeriod = m_defaultSettings.readEntry( "DayPeriod2", QString() ).split(',');
    dayPeriod[2] = newValue;
    setAmPmPeriods( m_kcmSettings.readEntry( "DayPeriod1", QString() ), dayPeriod.join( QChar::fromLatin1(',') ) );
}

void KCMLocale::initDateFormat()
{
    m_ui->m_comboDateFormat->blockSignals( true );

    m_ui->m_labelDateFormat->setText( ki18n( "Long date format:" ).toString() );
    QString helpText = ki18n( "<p>The text in this textbox will be used to format long dates. "
                              "The sequences below will be replaced:</p>"
                              "<table>"
                              "<tr>"
                                  "<td><b>YYYY</b></td>"
                                  "<td>The year with century as a decimal number.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>YY</b></td>"
                                  "<td>The year without century as a decimal number (00-99).</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>MM</b></td>"
                                  "<td>The month as a decimal number (01-12).</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>mM</b></td>"
                                  "<td>The month as a decimal number (1-12).</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>SHORTMONTH</b></td>"
                                  "<td>The first three characters of the month name.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>MONTH</b></td>"
                                  "<td>The full month name.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>DD</b></td>"
                                  "<td>The day of month as a decimal number (01-31).</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>dD</b></td>"
                                  "<td>The day of month as a decimal number (1-31).</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>SHORTWEEKDAY</b></td>"
                                  "<td>The first three characters of the weekday name.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>WEEKDAY</b></td>"
                                  "<td>The full weekday name.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>ERAYEAR</b></td>"
                                  "<td>The Era Year in local format (e.g. 2000 AD).</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>SHORTERANAME</b></td>"
                                  "<td>The short Era Name.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>YEARINERA</b></td>"
                                  "<td>The Year in Era as a decimal number.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>DAYOFYEAR</b></td>"
                                  "<td>The Day of Year as a decimal number.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>ISOWEEK</b></td>"
                                  "<td>The ISO Week as a decimal number.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>DAYOFISOWEEK</b></td>"
                                  "<td>The Day of the ISO Week as a decimal number.</td>"
                              "</tr>"
                              "</table>" ).toString();
    m_ui->m_comboDateFormat->setToolTip( helpText );
    m_ui->m_comboDateFormat->setWhatsThis( helpText );

    m_dateFormatMap.clear();
    m_dateFormatMap.insert( QString( 'Y' ), ki18n("YYYY").toString() );
    m_dateFormatMap.insert( QString( 'y' ), ki18n( "YY" ).toString() );
    m_dateFormatMap.insert( QString( 'n' ), ki18n( "mM" ).toString() );
    m_dateFormatMap.insert( QString( 'm' ), ki18nc( "Month", "MM" ).toString() );
    m_dateFormatMap.insert( QString( 'b' ), ki18n( "SHORTMONTH" ).toString() );
    m_dateFormatMap.insert( QString( 'B' ), ki18n( "MONTH" ).toString() );
    m_dateFormatMap.insert( QString( 'e' ), ki18n( "dD" ).toString() );
    m_dateFormatMap.insert( QString( 'd' ), ki18n( "DD" ).toString() );
    m_dateFormatMap.insert( QString( 'a' ), ki18n( "SHORTWEEKDAY" ).toString() );
    m_dateFormatMap.insert( QString( 'A' ), ki18n( "WEEKDAY" ).toString() );
    m_dateFormatMap.insert( "EY", ki18n( "ERAYEAR" ).toString() );
    m_dateFormatMap.insert( "Ey", ki18n( "YEARINERA" ).toString() );
    m_dateFormatMap.insert( "EC", ki18n( "SHORTERANAME" ).toString() );
    m_dateFormatMap.insert( QString( 'j' ), ki18n( "DAYOFYEAR" ).toString() );
    m_dateFormatMap.insert( QString( 'V' ), ki18n( "ISOWEEK" ).toString() );
    m_dateFormatMap.insert( QString( 'u' ), ki18n( "DAYOFISOWEEK" ).toString() );

    QStringList formatList;
    QString cValue = m_cSettings.readEntry( "DateFormat", QString() );
    formatList.append( posixToUserDate( m_kcmSettings.readEntry( "DateFormat", cValue ) ) );
    formatList.append( posixToUserDate( m_defaultSettings.readEntry( "DateFormat", cValue ) ) );
    formatList.append( posixToUserDate( m_countrySettings.readEntry( "DateFormat", cValue ) ) );
    formatList.append( posixToUserDate( cValue ) );
    // TODO convert these to POSIX and US format!
    QString formats = ki18nc("some reasonable date formats for the language",
                             "WEEKDAY MONTH dD YYYY\n"
                             "SHORTWEEKDAY MONTH dD YYYY").toString();
    formatList.append( formats.split( QString::fromLatin1("\n") ) );
    formatList.removeDuplicates();
    m_ui->m_comboDateFormat->clear();
    m_ui->m_comboDateFormat->addItems( formatList );

    setDateFormat( m_kcmSettings.readEntry( "DateFormat", QString() ) );

    m_ui->m_comboDateFormat->blockSignals( false );
}

void KCMLocale::defaultDateFormat()
{
    setDateFormat( m_defaultSettings.readEntry( "DateFormat", QString() ) );
}

void KCMLocale::changedDateFormat( const QString &newValue )
{
    setItem( "DateFormat", userToPosixDate( newValue ),
             m_ui->m_comboDateFormat, m_ui->m_buttonDefaultDateFormat );
    m_kcmLocale->setDateFormat( m_kcmSettings.readEntry( "DateFormat", QString() ) );
    updateSample();
}

void KCMLocale::setDateFormat( const QString &newValue )
{
    setItem( "DateFormat", newValue,
             m_ui->m_comboDateFormat, m_ui->m_buttonDefaultDateFormat );
    QString value = m_kcmSettings.readEntry( "DateFormat", QString() );
    m_ui->m_comboDateFormat->setEditText( posixToUserDate( value ) );
    m_kcmLocale->setDateFormat( value );
    updateSample();
}

void KCMLocale::initShortDateFormat()
{
    m_ui->m_comboShortDateFormat->blockSignals( true );

    m_ui->m_labelShortDateFormat->setText( ki18n( "Short date format:" ).toString() );
    QString helpText = ki18n( "<p>The text in this textbox will be used to format short dates. "
                              "For instance, this is used when listing files. The sequences below "
                              "will be replaced:</p>"
                              "<table>"
                              "<tr>"
                                  "<td><b>YYYY</b></td>"
                                  "<td>The year with century as a decimal number.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>YY</b></td>"
                                  "<td>The year without century as a decimal number (00-99).</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>MM</b></td>"
                                  "<td>The month as a decimal number (01-12).</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>mM</b></td>"
                                  "<td>The month as a decimal number (1-12).</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>SHORTMONTH</b></td>"
                                  "<td>The first three characters of the month name.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>MONTH</b></td>"
                                  "<td>The full month name.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>DD</b></td>"
                                  "<td>The day of month as a decimal number (01-31).</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>dD</b></td>"
                                  "<td>The day of month as a decimal number (1-31).</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>SHORTWEEKDAY</b></td>"
                                  "<td>The first three characters of the weekday name.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>WEEKDAY</b></td>"
                                  "<td>The full weekday name.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>ERAYEAR</b></td>"
                                  "<td>The Era Year in local format (e.g. 2000 AD).</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>SHORTERANAME</b></td>"
                                  "<td>The short Era Name.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>YEARINERA</b></td>"
                                  "<td>The Year in Era as a decimal number.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>DAYOFYEAR</b></td>"
                                  "<td>The Day of Year as a decimal number.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>ISOWEEK</b></td>"
                                  "<td>The ISO Week as a decimal number.</td>"
                              "</tr>"
                              "<tr>"
                                  "<td><b>DAYOFISOWEEK</b></td>"
                                  "<td>The Day of the ISO Week as a decimal number.</td>"
                              "</tr>"
                              "</table>" ).toString();
    m_ui->m_comboShortDateFormat->setToolTip( helpText );
    m_ui->m_comboShortDateFormat->setWhatsThis( helpText );

    QStringList formatList;
    QString cValue = m_cSettings.readEntry( "DateFormatShort", QString() );
    formatList.append( posixToUserDate( m_kcmSettings.readEntry( "DateFormatShort", cValue ) ) );
    formatList.append( posixToUserDate( m_defaultSettings.readEntry( "DateFormatShort", cValue ) ) );
    formatList.append( posixToUserDate( m_countrySettings.readEntry( "DateFormatShort", cValue ) ) );
    formatList.append( posixToUserDate( cValue ) );
    formatList.append( "YYYY-MM-DD" );
    // TODO convert these to POSIX and US format!
    QString formats = ki18nc("some reasonable short date formats for the language",
                             "YYYY-MM-DD\n"
                             "dD.mM.YYYY\n"
                             "DD.MM.YYYY").toString();
    formatList.append( formats.split( QString::fromLatin1("\n") ) );
    formatList.removeDuplicates();
    m_ui->m_comboShortDateFormat->clear();
    m_ui->m_comboShortDateFormat->addItems( formatList );

    setShortDateFormat( m_kcmSettings.readEntry( "DateFormatShort", QString() ) );

    m_ui->m_comboShortDateFormat->blockSignals( false );
}

void KCMLocale::defaultShortDateFormat()
{
    setShortDateFormat( m_defaultSettings.readEntry( "DateFormatShort", QString() ) );
}

void KCMLocale::changedShortDateFormat( const QString &newValue )
{
    setItem( "DateFormatShort", userToPosixDate( newValue ),
             m_ui->m_comboShortDateFormat, m_ui->m_buttonDefaultShortDateFormat );
    m_kcmLocale->setDateFormatShort( m_kcmSettings.readEntry( "DateFormatShort", QString() ) );
    updateSample();
}

void KCMLocale::setShortDateFormat( const QString &newValue )
{
    setItem( "DateFormatShort", newValue,
             m_ui->m_comboShortDateFormat, m_ui->m_buttonDefaultShortDateFormat );
    QString value = m_kcmSettings.readEntry( "DateFormatShort", QString() );
    m_ui->m_comboShortDateFormat->setEditText( posixToUserDate( value ) );
    m_kcmLocale->setDateFormatShort( value );
    updateSample();
}

void KCMLocale::initMonthNamePossessive()
{
    m_ui->m_checkMonthNamePossessive->blockSignals( true );

    m_ui->m_labelMonthNamePossessive->setText( ki18n( "Possessive month names:" ).toString() );
    QString helpText = ki18n( "<p>This option determines whether possessive form of month names "
                              "should be used in dates.</p>" ).toString();
    m_ui->m_checkMonthNamePossessive->setToolTip( helpText );
    m_ui->m_checkMonthNamePossessive->setWhatsThis( helpText );

    m_ui->m_checkMonthNamePossessive->setChecked( m_kcmLocale->dateMonthNamePossessive() );
    setCheckItem( "DateMonthNamePossessive", m_kcmSettings.readEntry( "DateMonthNamePossessive", false ),
                  m_ui->m_checkMonthNamePossessive, m_ui->m_buttonDefaultMonthNamePossessive );

    setMonthNamePossessive( m_kcmSettings.readEntry( "DateMonthNamePossessive", false ) );

    // Hide the option as it's not usable without ordinal day numbers
    m_ui->m_labelMonthNamePossessive->setHidden( true );
    m_ui->m_checkMonthNamePossessive->setHidden( true );
    m_ui->m_buttonDefaultMonthNamePossessive->setHidden( true );

    m_ui->m_checkMonthNamePossessive->blockSignals( false );
}

void KCMLocale::defaultMonthNamePossessive()
{
    setMonthNamePossessive( m_defaultSettings.readEntry( "DateMonthNamePossessive", false ) );
}

void KCMLocale::changedMonthNamePossessive( bool newValue )
{
    setMonthNamePossessive( newValue );
}

void KCMLocale::setMonthNamePossessive( bool newValue )
{
    setCheckItem( "DateMonthNamePossessive", newValue,
                  m_ui->m_checkMonthNamePossessive, m_ui->m_buttonDefaultMonthNamePossessive );
    m_kcmLocale->setDateMonthNamePossessive( m_kcmSettings.readEntry( "DateMonthNamePossessive", 0 ) );
    updateSample();
}

void KCMLocale::initDateTimeDigitSet()
{
    m_ui->m_comboDateTimeDigitSet->blockSignals( true );

    m_ui->m_labelDateTimeDigitSet->setText( ki18n( "Digit set:" ).toString() );
    QString helpText = ki18n( "<p>Here you can define the set of digits used to display dates and "
                              "times.  If digits other than Arabic are selected, they will appear "
                              "only if used in the language of the application or the piece of "
                              "text where the date or time is shown.</p><p>Note that the set of "
                              "digits used to display numeric and monetary values have to be set "
                              "separately (see the 'Numbers' or 'Money' tabs).</p>" ).toString();
    m_ui->m_comboDateTimeDigitSet->setToolTip( helpText );
    m_ui->m_comboDateTimeDigitSet->setWhatsThis( helpText );

    initDigitSetCombo( m_ui->m_comboDateTimeDigitSet );

    setDateTimeDigitSet( m_kcmSettings.readEntry( "DateTimeDigitSet", 0 ) );

    m_ui->m_comboDateTimeDigitSet->blockSignals( false );
}

void KCMLocale::defaultDateTimeDigitSet()
{
    setDateTimeDigitSet( m_defaultSettings.readEntry( "DateTimeDigitSet", 0 ) );
}

void KCMLocale::changedDateTimeDigitSetIndex( int index )
{
    setDateTimeDigitSet( m_ui->m_comboDateTimeDigitSet->itemData( index ).toInt() );
}

void KCMLocale::setDateTimeDigitSet( int newValue )
{
    setComboItem( "DateTimeDigitSet", newValue,
                  m_ui->m_comboDateTimeDigitSet, m_ui->m_buttonDefaultDateTimeDigitSet );
    m_kcmLocale->setDateTimeDigitSet( (KLocale::DigitSet) m_kcmSettings.readEntry( "DateTimeDigitSet", 0 ) );
}

void KCMLocale::initPageSize()
{
    m_ui->m_comboPageSize->blockSignals( true );

    m_ui->m_labelPageSize->setText( ki18n( "Page size:" ).toString() );
    QString helpText = ki18n( "<p>Here you can define the default page size to be used in new "
                              "documents.</p><p>Note that this setting has no effect on printer "
                              "paper size.</p>" ).toString();
    m_ui->m_comboPageSize->setToolTip( helpText );
    m_ui->m_comboPageSize->setWhatsThis( helpText );

    m_ui->m_comboPageSize->clear();

    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "A4").toString(),
                                    QVariant( QPrinter::A4 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "US Letter").toString(),
                                    QVariant( QPrinter::Letter ) );
    m_ui->m_comboPageSize->insertSeparator( m_ui->m_comboPageSize->count() );

    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "A0").toString(),
                                    QVariant( QPrinter::A0 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "A1").toString(),
                                    QVariant( QPrinter::A1 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "A2").toString(),
                                    QVariant( QPrinter::A2 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "A3").toString(),
                                    QVariant( QPrinter::A3 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "A4").toString(),
                                    QVariant( QPrinter::A4 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "A5").toString(),
                                    QVariant( QPrinter::A5 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "A6").toString(),
                                    QVariant( QPrinter::A6 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "A7").toString(),
                                    QVariant( QPrinter::A7 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "A8").toString(),
                                    QVariant( QPrinter::A8 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "A9").toString(),
                                    QVariant( QPrinter::A9 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "B0").toString(),
                                    QVariant( QPrinter::B0 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "B1").toString(),
                                    QVariant( QPrinter::B1 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "B2").toString(),
                                    QVariant( QPrinter::B2 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "B3").toString(),
                                    QVariant( QPrinter::B3 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "B4").toString(),
                                    QVariant( QPrinter::B4 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "B5").toString(),
                                    QVariant( QPrinter::B5 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "B6").toString(),
                                    QVariant( QPrinter::B6 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "B7").toString(),
                                    QVariant( QPrinter::B7 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "B8").toString(),
                                    QVariant( QPrinter::B8 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "B9").toString(),
                                    QVariant( QPrinter::B9 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "B10").toString(),
                                    QVariant( QPrinter::B10 ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "C5 Envelope").toString(),
                                    QVariant( QPrinter::C5E ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "US Common 10 Envelope").toString(),
                                    QVariant( QPrinter::Comm10E ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "DLE Envelope").toString(),
                                    QVariant( QPrinter::DLE ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "Executive").toString(),
                                    QVariant( QPrinter::Executive ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "Folio").toString(),
                                    QVariant( QPrinter::Folio ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "Ledger").toString(),
                                    QVariant( QPrinter::Ledger ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "US Legal").toString(),
                                    QVariant( QPrinter::Legal ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "US Letter").toString(),
                                    QVariant( QPrinter::Letter ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "Tabloid").toString(),
                                    QVariant( QPrinter::Tabloid ) );
    m_ui->m_comboPageSize->addItem( ki18nc("Page size", "Custom").toString(),
                                    QVariant( QPrinter::Custom ) );

    setPageSize( m_kcmSettings.readEntry( "PageSize", 0 ) );

    m_ui->m_comboPageSize->blockSignals( false );
}

void KCMLocale::defaultPageSize()
{
    setPageSize( m_defaultSettings.readEntry( "PageSize", 0 ) );
}

void KCMLocale::changedPageSizeIndex( int index )
{
    setPageSize( m_ui->m_comboPageSize->itemData( index ).toInt() );
}

void KCMLocale::setPageSize( int newValue )
{
    setComboItem( "PageSize", newValue,
                  m_ui->m_comboPageSize, m_ui->m_buttonDefaultPageSize );
    m_kcmLocale->setPageSize( m_kcmSettings.readEntry( "PageSize", 0 ) );
}

void KCMLocale::initMeasureSystem()
{
    m_ui->m_comboMeasureSystem->blockSignals( true );

    m_ui->m_labelMeasureSystem->setText( ki18n( "Measurement system:" ).toString() );
    QString helpText = ki18n( "<p>Here you can define the measurement system to use.</p>" ).toString();
    m_ui->m_comboMeasureSystem->setToolTip( helpText );
    m_ui->m_comboMeasureSystem->setWhatsThis( helpText );

    m_ui->m_comboMeasureSystem->clear();

    m_ui->m_comboMeasureSystem->addItem( ki18n("Metric System").toString(), (int) KLocale::Metric );
    m_ui->m_comboMeasureSystem->addItem( ki18n("Imperial System").toString(), (int) KLocale::Imperial );

    setMeasureSystem( m_kcmSettings.readEntry( "MeasureSystem", 0 ) );

    m_ui->m_comboMeasureSystem->blockSignals( false );
}

void KCMLocale::defaultMeasureSystem()
{
    setMeasureSystem( m_defaultSettings.readEntry( "MeasureSystem", 0 ) );
}

void KCMLocale::changedMeasureSystemIndex( int index )
{
    setMeasureSystem( m_ui->m_comboMeasureSystem->itemData( index ).toInt() );
}

void KCMLocale::setMeasureSystem( int newValue )
{
    setComboItem( "MeasureSystem", newValue,
                  m_ui->m_comboMeasureSystem, m_ui->m_buttonDefaultMeasureSystem );
    m_kcmLocale->setMeasureSystem( (KLocale::MeasureSystem) m_kcmSettings.readEntry( "MeasureSystem", 0 ) );
}

void KCMLocale::initBinaryUnitDialect()
{
    m_ui->m_comboBinaryUnitDialect->blockSignals( true );

    m_ui->m_labelBinaryUnitDialect->setText( ki18n( "Byte size units:" ).toString() );
    QString helpText = ki18n( "<p>This changes the units used by most KDE programs to display "
                              "numbers counted in bytes. Traditionally \"kilobytes\" meant units "
                              "of 1024, instead of the metric 1000, for most (but not all) byte "
                              "sizes."
                              "<ul>"
                              "<li>To reduce confusion you can use the recently standardized IEC "
                                  "units which are always in multiples of 1024.</li>"
                              "<li>You can also select metric, which is always in units of 1000.</li>"
                              "<li>Selecting JEDEC restores the older-style units used in KDE 3.5 "
                                  "and some other operating systems.</li>"
			      "</ul>"
                              "</p>" ).toString();
    m_ui->m_comboBinaryUnitDialect->setToolTip( helpText );
    m_ui->m_comboBinaryUnitDialect->setWhatsThis( helpText );

    m_ui->m_comboBinaryUnitDialect->clear(),
    m_ui->m_comboBinaryUnitDialect->addItem( ki18nc("Unit of binary measurement", "IEC Units (KiB, MiB, etc)").toString(),
                                             QVariant( KLocale::IECBinaryDialect ) );
    m_ui->m_comboBinaryUnitDialect->addItem( ki18nc("Unit of binary measurement", "JEDEC Units (KB, MB, etc)").toString(),
                                             QVariant( KLocale::JEDECBinaryDialect ) );
    m_ui->m_comboBinaryUnitDialect->addItem( ki18nc("Unit of binary measurement", "Metric Units (kB, MB, etc)").toString(),
                                             QVariant( KLocale::MetricBinaryDialect ) );

    setBinaryUnitDialect( m_kcmSettings.readEntry( "BinaryUnitDialect", 0 ) );

    m_ui->m_comboBinaryUnitDialect->blockSignals( false );
}

void KCMLocale::defaultBinaryUnitDialect()
{
    setBinaryUnitDialect( m_defaultSettings.readEntry( "BinaryUnitDialect", 0 ) );
}

void KCMLocale::changedBinaryUnitDialectIndex( int index )
{
    setBinaryUnitDialect( m_ui->m_comboBinaryUnitDialect->itemData( index ).toInt() );
}

void KCMLocale::setBinaryUnitDialect( int newValue )
{
    setComboItem( "BinaryUnitDialect", newValue,
                  m_ui->m_comboBinaryUnitDialect, m_ui->m_buttonDefaultBinaryUnitDialect );
    m_kcmLocale->setBinaryUnitDialect( (KLocale::BinaryUnitDialect)
                                       m_kcmSettings.readEntry( "BinaryUnitDialect", 0 ) );
    m_ui->m_labelBinaryUnitSample->setText( ki18nc("Example test for binary unit dialect",
                                                   "Example: 2000 bytes equals %1")
                                                  .subs( m_kcmLocale->formatByteSize( 2000, 2 ) )
                                                  .toString() );
}

QString KCMLocale::posixToUserTime( const QString &posixFormat ) const
{
    return posixToUser( posixFormat, m_timeFormatMap );
}

QString KCMLocale::posixToUserDate( const QString &posixFormat ) const
{
    return posixToUser( posixFormat, m_dateFormatMap );
}

QString KCMLocale::posixToUser( const QString &posixFormat, const QMap<QString, QString> &map ) const
{
    QString result;

    bool escaped = false;
    for ( int pos = 0; pos < posixFormat.length(); ++pos ) {
        QChar c = posixFormat.at( pos );
        if ( escaped ) {
            QString key = c;
            if ( c == 'E' ) {
                key += posixFormat.at( ++pos );
            }
            QString val = map.value( key, QString() );
            if ( !val.isEmpty() ) {
                result += val;
            } else {
                result += key;
            }
            escaped = false;
        } else if ( c == '%' ) {
            escaped = true;
        } else {
            result += c;
        }
    }

    return result;
}

QString KCMLocale::userToPosixTime( const QString &userFormat ) const
{
    return userToPosix( userFormat, m_timeFormatMap );
}

QString KCMLocale::userToPosixDate( const QString &userFormat ) const
{
    return userToPosix( userFormat, m_dateFormatMap );
}

QString KCMLocale::userToPosix( const QString &userFormat, const QMap<QString, QString> &map ) const
{
    QMultiMap<int, QString> sizeMap;
    QMap<QString, QString>::const_iterator it = map.constBegin();
    while ( it != map.constEnd() ) {
        sizeMap.insert( it.value().length(), it.key() );
        ++it;
    }

    QString result;

    for ( int pos = 0; pos < userFormat.length(); ++pos ) {
        bool matchFound = false;
        QMapIterator<int, QString> it2(sizeMap);
        it2.toBack();
        while (!matchFound && it2.hasPrevious()) {
            it2.previous();
            QString s = map.value(it2.value());

            if ( userFormat.mid( pos, s.length() ) == s ) {
                result += '%';
                result += it2.value();
                pos += s.length() - 1;
                matchFound = true;
            }
        }

        if ( !matchFound ) {
            QChar c = userFormat.at( pos );
            if ( c == '%' ) {
                result += c;
            }
            result += c;
        }
    }

    return result;
}

#include "kcmlocale.moc"
