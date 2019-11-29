/*
 *  kcmformats.cpp
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>
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
 */

// own
#include "kcmformats.h"
#include "ui_kcmformatswidget.h"

// Qt
#include <QApplication>
#include <QComboBox>
#include <QFile>
#include <QDebug>
#include <QLocale>
#include <QStandardPaths>
#include <QTextStream>
#include <QTextCodec>

// Frameworks
#include <KPluginFactory>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

K_PLUGIN_FACTORY_WITH_JSON(KCMFormatsFactory, "formats.json", registerPlugin<KCMFormats>();)

KCMFormats::KCMFormats(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    setQuickHelp(i18n("<h1>Formats</h1>"
                      "You can configure the formats used for time, dates, money and other numbers here."));

    m_ui = new Ui::KCMFormatsWidget;
    m_ui->setupUi(this);
    m_combos << m_ui->comboGlobal
             << m_ui->comboNumbers
             << m_ui->comboTime
             << m_ui->comboCurrency
             << m_ui->comboMeasurement
             << m_ui->comboCollate;
}

KCMFormats::~KCMFormats()
{
    delete m_ui;
}

bool countryLessThan(const QLocale & c1, const QLocale & c2)
{
    // Ensure that the "Default (C)" locale always appears at the top
    if (c1.name()== QLatin1Char('C') && c2.name()!=QLatin1String("C")) return true;
    if (c2.name()== QLatin1Char('C')) return false;

    const QString ncn1 = !c1.nativeCountryName().isEmpty() ? c1.nativeCountryName() : QLocale::countryToString(c1.country());
    const QString ncn2 = !c2.nativeCountryName().isEmpty() ? c2.nativeCountryName() : QLocale::countryToString(c2.country());
    return QString::localeAwareCompare(ncn1, ncn2) < 0;
}

void KCMFormats::load()
{
    QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    std::sort(allLocales.begin(), allLocales.end(), countryLessThan);
    foreach(QComboBox * combo, m_combos) {
        initCombo(combo, allLocales);
    }

    readConfig();

    foreach(QComboBox * combo, m_combos) {
        connectCombo(combo);
    }

    connect(m_ui->checkDetailed, &QAbstractButton::toggled, [ = ]() {
        updateExample();
        updateEnabled();
        emit changed(true);
    });

    updateEnabled();
    updateExample();
    emit changed(false);
}

void KCMFormats::initCombo(QComboBox *combo, const QList<QLocale> & allLocales)
{
    combo->clear();
    const QString clabel = i18n("No change");
    combo->addItem(clabel, QString());
    foreach(const QLocale & l, allLocales) {
        addLocaleToCombo(combo, l);
    }
}

void KCMFormats::connectCombo(QComboBox *combo)
{
    connect(combo, &QComboBox::currentTextChanged, [ = ]() {
        emit changed(true);
        updateExample();
    });
}

void KCMFormats::addLocaleToCombo(QComboBox *combo, const QLocale &locale)
{
    const QString clabel = !locale.nativeCountryName().isEmpty() ? locale.nativeCountryName() : QLocale::countryToString(locale.country());
    // This needs to use name() rather than bcp47name() or later on the export will generate a non-sense locale (e.g. "it" instead of
    // "it_IT")
    // TODO: Properly handle scripts (@foo)
    QString cvalue = locale.name();
    if (!cvalue.contains(QLatin1Char('.')) && cvalue != QLatin1Char('C')) {
        // explicitly add the encoding,
        // otherwise Qt doesn't accept dead keys and garbles the output as well
        cvalue.append(QLatin1Char('.') + QTextCodec::codecForLocale()->name());
    }

    QString flagcode;
    const QStringList split = locale.name().split(QLatin1Char('_'));
    if (split.count() > 1) {
        flagcode = split[1].toLower();
    }
    QIcon flagIcon = loadFlagIcon(flagcode);

    const QString nativeLangName = locale.nativeLanguageName();
    if (!nativeLangName.isEmpty()) {
        combo->addItem(flagIcon, i18n("%1 - %2 (%3)", clabel, nativeLangName, locale.name()), cvalue);
    } else {
        combo->addItem(flagIcon, i18n("%1 (%2)", clabel, locale.name()), cvalue);
    }
}

void setCombo(QComboBox *combo, const QString &key)
{
    const int ix = combo->findData(key);
    if (ix > -1) {
        combo->setCurrentIndex(ix);
    }
}

QIcon KCMFormats::loadFlagIcon(const QString &flagCode)
{
    QIcon icon = m_cachedFlags.value(flagCode);
    if (!icon.isNull()) {
        return icon;
    }

    QString flag(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kf5/locale/countries/%1/flag.png").arg(flagCode)));
    if (!flag.isEmpty()) {
        icon = QIcon(flag);
    } else {
        if (m_cachedUnknown.isNull()) {
            m_cachedUnknown = QIcon::fromTheme("unknown");
        }
        icon = m_cachedUnknown;
    }
    m_cachedFlags.insert(flagCode, icon);

    return icon;
}

const static QString configFile = QStringLiteral("plasma-localerc");
const static QString exportFile = QStringLiteral("plasma-locale-settings.sh");

const static QString lcLang = QStringLiteral("LANG");
const static QString lcNumeric = QStringLiteral("LC_NUMERIC");
const static QString lcTime = QStringLiteral("LC_TIME");
const static QString lcMonetary = QStringLiteral("LC_MONETARY");
const static QString lcMeasurement = QStringLiteral("LC_MEASUREMENT");
const static QString lcCollate = QStringLiteral("LC_COLLATE");
const static QString lcCtype = QStringLiteral("LC_CTYPE");

const static QString lcLanguage = QStringLiteral("LANGUAGE");



void KCMFormats::readConfig()
{
    m_config = KConfigGroup(KSharedConfig::openConfig(configFile), "Formats");

    bool useDetailed = m_config.readEntry("useDetailed", false);
    m_ui->checkDetailed->setChecked(useDetailed);

    setCombo(m_ui->comboGlobal, m_config.readEntry(lcLang, qgetenv(lcLang.toLatin1())));

    setCombo(m_ui->comboNumbers, m_config.readEntry(lcNumeric, qgetenv(lcNumeric.toLatin1())));
    setCombo(m_ui->comboTime, m_config.readEntry(lcTime, qgetenv(lcTime.toLatin1())));
    setCombo(m_ui->comboCollate, m_config.readEntry(lcCollate, qgetenv(lcCollate.toLatin1())));
    setCombo(m_ui->comboCurrency, m_config.readEntry(lcMonetary, qgetenv(lcMonetary.toLatin1())));
    setCombo(m_ui->comboMeasurement, m_config.readEntry(lcMeasurement, qgetenv(lcMeasurement.toLatin1())));

    updateEnabled();
}

void KCMFormats::writeConfig()
{
    m_config = KConfigGroup(KSharedConfig::openConfig(configFile), "Formats");

    // global ends up empty here when OK button is clicked from kcmshell5,
    // apparently the data in the combo is gone by the time save() is called.
    // This might be a problem in KCModule, but does not directly affect us
    // since within systemsettings, it works fine.
    // See https://bugs.kde.org/show_bug.cgi?id=334624
    if (m_ui->comboGlobal->count() == 0) {
        qWarning() << "Couldn't read data from UI, writing configuration failed.";
        return;
    }
    const QString global = m_ui->comboGlobal->currentData().toString();

    if (!m_ui->checkDetailed->isChecked()) {
        // Global setting, clean up config
        m_config.deleteEntry("useDetailed");
        if (global.isEmpty()) {
            m_config.deleteEntry(lcLang);
        } else {
            m_config.writeEntry(lcLang, global);
        }
        m_config.deleteEntry(lcNumeric);
        m_config.deleteEntry(lcTime);
        m_config.deleteEntry(lcMonetary);
        m_config.deleteEntry(lcMeasurement);
        m_config.deleteEntry(lcCollate);
        m_config.deleteEntry(lcCtype);
    } else {
        // Save detailed settings
        m_config.writeEntry("useDetailed", true);

        if (global.isEmpty()) {
            m_config.deleteEntry(lcLang);
        } else {
            m_config.writeEntry(lcLang, global);
        }

        const QString numeric = m_ui->comboNumbers->currentData().toString();
        if (numeric.isEmpty()) {
            m_config.deleteEntry(lcNumeric);
        } else {
            m_config.writeEntry(lcNumeric, numeric);
        }

        const QString time = m_ui->comboTime->currentData().toString();
        if (time.isEmpty()) {
            m_config.deleteEntry(lcTime);
        } else {
            m_config.writeEntry(lcTime, time);
        }

        const QString monetary = m_ui->comboCurrency->currentData().toString();
        if (monetary.isEmpty()) {
            m_config.deleteEntry(lcMonetary);
        } else {
            m_config.writeEntry(lcMonetary, monetary);
        }

        const QString measurement = m_ui->comboMeasurement->currentData().toString();
        if (measurement.isEmpty()) {
            m_config.deleteEntry(lcMeasurement);
        } else {
            m_config.writeEntry(lcMeasurement, measurement);
        }

        const QString collate = m_ui->comboCollate->currentData().toString();
        if (collate.isEmpty()) {
            m_config.deleteEntry(lcCollate);
        } else {
            m_config.writeEntry(lcCollate, collate);
        }
    }

    m_config.sync();
}

void KCMFormats::save()
{
    writeConfig();
    KMessageBox::information(this, i18n("Your changes will take effect the next time you log in."),
                             i18n("Format Settings Changed"), QStringLiteral("FormatSettingsChanged"));
}

void KCMFormats::defaults()
{
    m_ui->checkDetailed->setChecked(false);

    // restore user defaults from env vars
    setCombo(m_ui->comboGlobal, qgetenv(lcLang.toLatin1()));
    setCombo(m_ui->comboNumbers, qgetenv(lcNumeric.toLatin1()));
    setCombo(m_ui->comboTime, qgetenv(lcTime.toLatin1()));
    setCombo(m_ui->comboCollate, qgetenv(lcCollate.toLatin1()));
    setCombo(m_ui->comboCurrency, qgetenv(lcMonetary.toLatin1()));
    setCombo(m_ui->comboMeasurement, qgetenv(lcMeasurement.toLatin1()));

    updateEnabled();
}

void KCMFormats::updateEnabled()
{
    const bool enabled = m_ui->checkDetailed->isChecked();

    m_ui->labelNumbers->setEnabled(enabled);
    m_ui->labelTime->setEnabled(enabled);
    m_ui->labelCurrency->setEnabled(enabled);
    m_ui->labelMeasurement->setEnabled(enabled);
    m_ui->labelCollate->setEnabled(enabled);
    m_ui->comboNumbers->setEnabled(enabled);
    m_ui->comboTime->setEnabled(enabled);
    m_ui->comboCurrency->setEnabled(enabled);
    m_ui->comboMeasurement->setEnabled(enabled);
    m_ui->comboCollate->setEnabled(enabled);
}

void KCMFormats::updateExample()
{
    const bool useDetailed = m_ui->checkDetailed->isChecked();

    QLocale nloc;
    QLocale tloc;
    QLocale cloc;
    QLocale mloc;

    QString str;
    QString glob = m_ui->comboGlobal->currentData().toString();

    if (useDetailed) {
        str = m_ui->comboNumbers->currentData().toString();
        nloc = str.isEmpty() ? QLocale(glob) : QLocale(str);

        str = m_ui->comboTime->currentData().toString();
        tloc = str.isEmpty() ? QLocale(glob) : QLocale(str);

        str = m_ui->comboCurrency->currentData().toString();
        cloc = str.isEmpty() ? QLocale(glob) : QLocale(str);

        str = m_ui->comboMeasurement->currentData().toString();
        mloc = str.isEmpty() ? QLocale(glob) : QLocale(str);
    } else {
        nloc = QLocale(glob);
        tloc = QLocale(glob);
        cloc = QLocale(glob);
        mloc = QLocale(glob);
    }

    const QString numberExample = nloc.toString(1000.01);
    const QString timeExample = i18n("%1 (long format)", tloc.toString(QDateTime::currentDateTime())) + QLatin1Char('\n') +
            i18n("%1 (short format)", tloc.toString(QDateTime::currentDateTime(), QLocale::ShortFormat));
    const QString currencyExample = cloc.toCurrencyString(24.00);

    QString measurementSetting;
    if (mloc.measurementSystem() == QLocale::ImperialUKSystem) {
        measurementSetting = i18nc("Measurement combobox", "Imperial UK");
    } else if (mloc.measurementSystem() == QLocale::ImperialUSSystem || mloc.measurementSystem() == QLocale::ImperialSystem) {
        measurementSetting = i18nc("Measurement combobox", "Imperial US");
    } else {
        measurementSetting = i18nc("Measurement combobox", "Metric");
    }

    m_ui->exampleNumbers->setText(numberExample);
    m_ui->exampleTime->setText(timeExample);
    m_ui->exampleCurrency->setText(currencyExample);
    m_ui->exampleMeasurement->setText(measurementSetting);
}

#include "kcmformats.moc"
