/*  This file is part of the KDE libraries
 *  Copyright 2010 John Layt <john@layt.net>
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

#ifndef KCMLOCALE_H
#define KCMLOCALE_H

#include <QMap>

#include <KCModule>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocale>

class QListWidgetItem;
class QCheckBox;
class KPushButton;
class KComboBox;
class KIntNumInput;

namespace Ui {
  class KCMLocaleWidget;
}

/**
 * @short A KCM to configure locale settings
 *
 * This module is for changing the User's Locale settings, which may override their Group and
 * Country defaults.
 *
 * The settings hierarchy is as follows:
 * - User settings from kdeglobals
 * - Group settings from $KDEDIRS
 * - Country settings from l10n
 * - C default settings from l10n
 *
 * The settings that apply to the User are a merger of all these.
 *
 * This may be restricted by Kiosk Group settings locking the user from updating some/all settings.
 *
 * The KCM starts by loading the fully merged settings including the User settings
 * In KCM terms, to Reload is to load the fully merged settings including the User settings
 * In KCM terms, to reset to Defaults is to remove the User settings only.
 * The user can also reset to Default each individual setting.
 */

class KCMLocale : public KCModule
{
    Q_OBJECT

public:
    KCMLocale(QWidget *parent, const QVariantList &);
    virtual ~KCMLocale();

    virtual void load();
    virtual void save();
    virtual void defaults();
    virtual QString quickHelp() const;

private:

    //Common load/save utilities

    // Initialise the different settings groups
    void initSettings();
    void initCountrySettings( const QString &countryCode );
    void initCalendarSettings();

    // Merge the different settings groups into the effective settings
    void mergeSettings();
    void mergeCalendarSettings();

    // Copy the supported settings between settings groups
    void copySettings( KConfigGroup *fromGroup, KConfigGroup *toGroup,
                       KConfig::WriteConfigFlags flags = KConfig::Normal );
    void copyCalendarSettings( KConfigGroup *fromGroup, KConfigGroup *toGroup,
                               KConfig::WriteConfigFlags flags = KConfig::Normal );
    void copySetting( KConfigGroup *fromGroup, KConfigGroup *toGroup, const QString &key,
                      KConfig::WriteConfigFlags flags = KConfig::Normal );

    // Enable / Disable an item in the gui
    void enableItemWidgets( const QString &itemKey, KConfigGroup *userSettings,
                            KConfigGroup *kcmSettings, KConfigGroup *defaultSettings,
                            QWidget *itemWidget, KPushButton *itemDefaultButton );

    // Set the item value in the required settings groups
    void setItemValue( const QString &itemKey, const QString &itemValue,
                       KConfigGroup *userSettings, KConfigGroup *kcmSettings, KConfigGroup *defaultSettings );

    // Set an item: set the item value, enable the widgets, set the changed flag, don't update the widget
    void setItem( const QString &itemKey, const QString &itemValue,
                  QWidget *itemWidget, KPushButton *itemDefaultButton );
    void setItem( const QString &itemKey, int itemValue,
                  QWidget *itemWidget, KPushButton *itemDefaultButton );
    void setItem( const QString &itemKey, bool itemValue,
                  QWidget *itemWidget, KPushButton *itemDefaultButton );
    void setCalendarItem( const QString &itemKey, const QString &itemValue,
                          QWidget *itemWidget, KPushButton *itemDefaultButton );
    void setCalendarItem( const QString &itemKey, int itemValue,
                          QWidget *itemWidget, KPushButton *itemDefaultButton );
    void setCalendarItem( const QString &itemKey, bool itemValue,
                          QWidget *itemWidget, KPushButton *itemDefaultButton );

    // Set an item: set the item value, enable the widgets, set the changed flag, update the widget
    void setComboItem( const QString &itemKey, const QString &itemValue,
                       KComboBox *itemCombo, KPushButton *itemDefaultButton );
    void setComboItem( const QString &itemKey, int itemValue,
                       KComboBox *itemCombo, KPushButton *itemDefaultButton );
    void setEditComboItem( const QString &itemKey, const QString &itemValue,
                           KComboBox *itemCombo, KPushButton *itemDefaultButton );
    void setIntItem( const QString &itemKey, int itemValue,
                     KIntNumInput *itemInput, KPushButton *itemDefaultButton );
    void setCheckItem( const QString &itemKey, bool itemValue,
                       QCheckBox *itemCheck, KPushButton *itemDefaultButton );
    void setMonetaryFormat( const QString &prefixCurrencySymbolKey, bool prefixCurrencySymbol,
                            const QString &signPositionKey, KLocale::SignPosition signPosition,
                            QWidget *formatWidget, KPushButton *formatDefaultButton );

    // Check if the chaged flag needs to be set
    void checkIfChanged();

    //Common init utilities
    void initSeparatorCombo( KComboBox *separatorCombo );
    void initWeekDayCombo( KComboBox *dayCombo );
    void initDigitSetCombo( KComboBox *digitSetCombo );
    void initDigitGroupingCombo( KComboBox *digitGroupingCombo, const QString &digitGroupingKey);
    void insertDigitGroupingItem( KComboBox *digitGroupingCombo,
                                  KSharedConfigPtr groupingConfig, KConfigGroup *groupingSettings,
                                  const QString &digitGroupingKey, const QString &digitGroupingFormat);
    void insertMonetaryPositiveFormat( bool prefixCurrencySymbol, KLocale::SignPosition signPosition );
    void insertMonetaryNegativeFormat( bool prefixCurrencySymbol, KLocale::SignPosition signPosition );

    void initAllWidgets();
    void initSettingsWidgets();
    void initResetButtons();
    void initTabs();
    void initSample();

    //Country tab

    void initCountry();
    void setCountry( const QString &newValue );

    void initCountryDivision();
    void setCountryDivision( const QString &newValue );

    //Translations/Languages tab

    void initTranslations();
    void setTranslations( const QString &newValue );

    void initTranslationsInstall();

    //Numeric tab

    void initNumericDigitGrouping();
    void setNumericDigitGrouping( const QString &newValue );

    void initNumericThousandsSeparator();
    void setNumericThousandsSeparator( const QString &newValue );

    void initNumericDecimalSymbol();
    void setNumericDecimalSymbol( const QString &newValue );

    void initNumericDecimalPlaces();
    void setNumericDecimalPlaces( int newValue );

    void initNumericPositiveSign();
    void setNumericPositiveSign( const QString &newValue );

    void initNumericNegativeSign();
    void setNumericNegativeSign( const QString &newValue );

    void initNumericDigitSet();
    void setNumericDigitSet( int newValue );

    //Monetary tab

    void initCurrencyCode();
    void setCurrencyCode( const QString &newValue );

    void initCurrencySymbol();
    void setCurrencySymbol( const QString &newValue );

    void initMonetaryDigitGrouping();
    void setMonetaryDigitGrouping( const QString &newValue );

    void initMonetaryThousandsSeparator();
    void setMonetaryThousandsSeparator( const QString &newValue );

    void initMonetaryDecimalSymbol();
    void setMonetaryDecimalSymbol( const QString &newValue );

    void initMonetaryDecimalPlaces();
    void setMonetaryDecimalPlaces( int newValue );

    void initMonetaryPositiveFormat();
    void setMonetaryPositiveFormat( bool prefixCurrencySymbol, KLocale::SignPosition signPosition );

    void initMonetaryNegativeFormat();
    void setMonetaryNegativeFormat( bool prefixCurrencySymbol, KLocale::SignPosition signPosition );

    void initMonetaryDigitSet();
    void setMonetaryDigitSet( int newValue );

    //Calendar Tab

    void initCalendarSystem();
    void setCalendarSystem( const QString &newValue );

    void initUseCommonEra();
    void setUseCommonEra( bool newValue );

    void initShortYearWindow();
    void setShortYearWindow( int newValue );

    void initWeekNumberSystem();
    void setWeekNumberSystem( int newValue );

    void initWeekStartDay();
    void setWeekStartDay( int newValue );

    void initWorkingWeekStartDay();
    void setWorkingWeekStartDay( int newValue );

    void initWorkingWeekEndDay();
    void setWorkingWeekEndDay( int newValue );

    void initWeekDayOfPray();
    void setWeekDayOfPray( int newValue );

    //Date/Time tab

    void initTimeFormat();
    void setTimeFormat( const QString &newValue );

    void initAmPmSymbols();
    void setAmPmPeriods( const QString &amValue, const QString &pmValue );

    void initDateFormat();
    void setDateFormat( const QString &newValue );

    void initShortDateFormat();
    void setShortDateFormat( const QString &newValue );

    void initMonthNamePossessive();
    void setMonthNamePossessive( bool newValue );

    void initDateTimeDigitSet();
    void setDateTimeDigitSet( int newValue );

    //Other Tab

    void initPageSize();
    void setPageSize( int newValue );

    void initMeasureSystem();
    void setMeasureSystem( int newValue );

    void initBinaryUnitDialect();
    void setBinaryUnitDialect( int newValue );

private Q_SLOTS:

    void updateSample();

    //Country tab

    void defaultCountry();
    void changedCountryIndex( int index );

    void defaultCountryDivision();
    void changedCountryDivisionIndex( int index );

    //Translations/Languages tab

    void defaultTranslations();
    void changedTranslations();
    void changedTranslationsAvailable( QListWidgetItem * item );
    void changedTranslationsSelected( QListWidgetItem * item );

    void installTranslations();

    //Numeric tab

    void defaultNumericDigitGrouping();
    void changedNumericDigitGroupingIndex( int index );

    void defaultNumericThousandsSeparator();
    void changedNumericThousandsSeparator( const QString &newValue );

    void defaultNumericDecimalSymbol();
    void changedNumericDecimalSymbol( const QString &newValue );

    void defaultNumericDecimalPlaces();
    void changedNumericDecimalPlaces( int newValue );

    void defaultNumericPositiveSign();
    void changedNumericPositiveSign( const QString &newValue );

    void defaultNumericNegativeSign();
    void changedNumericNegativeSign( const QString &newValue );

    void defaultNumericDigitSet();
    void changedNumericDigitSetIndex( int index );

    //Monetary tab

    void defaultCurrencyCode();
    void changedCurrencyCodeIndex( int index );

    void defaultCurrencySymbol();
    void changedCurrencySymbolIndex( int index );

    void defaultMonetaryDigitGrouping();
    void changedMonetaryDigitGroupingIndex( int index );

    void defaultMonetaryThousandsSeparator();
    void changedMonetaryThousandsSeparator( const QString &newValue );

    void defaultMonetaryDecimalSymbol();
    void changedMonetaryDecimalSymbol( const QString &newValue );

    void defaultMonetaryDecimalPlaces();
    void changedMonetaryDecimalPlaces( int newValue );

    void defaultMonetaryPositiveFormat();
    void changedMonetaryPositiveFormatIndex( int index );

    void defaultMonetaryNegativeFormat();
    void changedMonetaryNegativeFormatIndex( int index );

    void defaultMonetaryDigitSet();
    void changedMonetaryDigitSetIndex( int index );

    //Calendar Tab

    void defaultCalendarSystem();
    void changedCalendarSystemIndex( int index );

    void defaultUseCommonEra();
    void changedUseCommonEra( bool newValue );

    void defaultShortYearWindow();
    void changedShortYearWindow( int newValue );

    void defaultWeekNumberSystem();
    void changedWeekNumberSystemIndex( int index );

    void defaultWeekStartDay();
    void changedWeekStartDayIndex( int index );

    void defaultWorkingWeekStartDay();
    void changedWorkingWeekStartDayIndex( int index );

    void defaultWorkingWeekEndDay();
    void changedWorkingWeekEndDayIndex( int index );

    void defaultWeekDayOfPray();
    void changedWeekDayOfPrayIndex( int index );

    //Date/Time tab

    void defaultTimeFormat();
    void changedTimeFormat( const QString &newValue );

    void defaultAmSymbol();
    void changedAmSymbol( const QString &newValue );

    void defaultPmSymbol();
    void changedPmSymbol( const QString &newValue );

    void defaultDateFormat();
    void changedDateFormat( const QString &newValue );

    void defaultShortDateFormat();
    void changedShortDateFormat( const QString &newValue );

    void defaultMonthNamePossessive();
    void changedMonthNamePossessive( bool newValue );

    void defaultDateTimeDigitSet();
    void changedDateTimeDigitSetIndex( int index );

    //Other Tab

    void defaultPageSize();
    void changedPageSizeIndex( int index );

    void defaultMeasureSystem();
    void changedMeasureSystemIndex( int index );

    void defaultBinaryUnitDialect();
    void changedBinaryUnitDialectIndex( int index );

private:
    // Date/Time format map utilities
    QString posixToUserDate( const QString &posixFormat ) const;
    QString posixToUserTime( const QString &posixFormat ) const;
    QString posixToUser( const QString &posixFormat, const QMap<QString, QString> &map ) const;
    QString userToPosixDate( const QString &userFormat ) const;
    QString userToPosixTime( const QString &userFormat ) const;
    QString userToPosix( const QString &userFormat, const QMap<QString, QString> &map ) const;

    // Day Period utilities
    QString dayPeriodText( const QString &dayPeriod );
    QString amPeriod( const QString &longName, const QString &shortName, const QString &narrowName );
    QString pmPeriod( const QString &longName, const QString &shortName, const QString &narrowName );

    // The current User settings from .kde/share/config/kdeglobals
    // This gets updated with the users changes in the kcm and saved when requested
    KSharedConfigPtr m_userConfig;
    KConfigGroup m_userSettings;
    KConfigGroup m_userCalendarSettings;
    // The kcm config/settings, a merger of C, Country, Group and User settings
    // This is used to build the displayed settings and to initialise the sample locale, but never saved
    KSharedConfigPtr m_kcmConfig;
    KConfigGroup m_kcmSettings;
    KConfigGroup m_kcmCalendarSettings;
    // The currently saved user config/settings
    // This is used to check if anything has changed, but never saved
    KSharedConfigPtr m_currentConfig;
    KConfigGroup m_currentSettings;
    KConfigGroup m_currentCalendarSettings;
    // The KCM Default settings, a merger of C, Country, and Group, i.e. excluding User
    KSharedConfigPtr m_defaultConfig;
    KConfigGroup m_defaultSettings;
    KConfigGroup m_defaultCalendarSettings;

    // The current Group settings, i.e. does NOT include the User or Country settings
    KSharedConfigPtr m_groupConfig;
    KConfigGroup m_groupSettings;
    KConfigGroup m_groupCalendarSettings;
    // The Country Locale config from l10n/<country>/entry.desktop
    KSharedConfigPtr m_countryConfig;
    KConfigGroup m_countrySettings;
    KConfigGroup m_countryCalendarSettings;
    // The default C Locale config/settings from l10n/C/entry.desktop
    KSharedConfigPtr m_cConfig;
    KConfigGroup m_cSettings;
    KConfigGroup m_cCalendarSettings;

    QMap<QString, QString> m_dateFormatMap;
    QMap<QString, QString> m_timeFormatMap;

    // The system country, not the KDE one
    QString m_systemCountry;

    // NOTE: we need to mantain our own language list instead of using KLocale's
    // because KLocale does not add a language if there is no translation
    // for the current application so it would not be possible to set
    // a language which has no systemsettings/kcontrol module translation
    // The list of translations used in the kcm, is users list plus us_EN
    QStringList m_kcmTranslations;
    // The currently saved list of user translations, used to check if value changed
    QString m_currentTranslations;
    // The currently installed translations, used to check if users translations are valid
    QStringList m_installedTranslations;

    // The locale we use when displaying the sample output, not used for anything else
    KLocale *m_kcmLocale;
    // The locale for the current defaults, needed for am/pm as the defaults are hard coded in KLocale
    KLocale *m_defaultLocale;

    Ui::KCMLocaleWidget *m_ui;
};

#endif //KCMLOCALE_H