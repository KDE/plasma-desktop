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

// Frameworks
#include <KPluginFactory>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

K_PLUGIN_FACTORY_WITH_JSON(KCMFormatsFactory, "formats.json", registerPlugin<KCMFormats>();)

const static QString configFile = QStringLiteral("plasma-localerc");
const static QString exportFile = QStringLiteral("plasma-locale-settings.sh");

const static QString lcLang = QStringLiteral("LANG");

const static QString lcNumeric = QStringLiteral("LC_NUMERIC");
const static QString lcTime = QStringLiteral("LC_TIME");
const static QString lcMonetary = QStringLiteral("LC_MONETARY");
const static QString lcMeasurement = QStringLiteral("LC_MEASUREMENT");
const static QString lcCollate = QStringLiteral("LC_COLLATE");
const static QString lcCtype = QStringLiteral("LC_CTYPE");

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

void KCMFormats::load()
{
    QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
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

void KCMFormats::initCombo(QComboBox *combo, const QList<QLocale> allLocales)
{
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
    const QString clabel = !locale.nativeCountryName().isEmpty() ? locale.nativeCountryName() : locale.countryToString(locale.country());
    // This needs to use name() rather than bcp47name() or later on the export will generate a non-sense locale (e.g. "it" instead of
    // "it_IT")
    // TODO: Properly handle encodings (.UTF-8) and scripts (@foo)
    const QString cvalue = locale.name();

    QString flagcode;
    const QStringList split = locale.name().split('_');
    if (split.count() > 1) {
        flagcode = split[1].toLower();
    }
    QString flag(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("locale/") + QString::fromLatin1("l10n/%1/flag.png").arg(flagcode)));
    QIcon flagIcon;
    if (!flag.isEmpty()) {
        flagIcon = QIcon(QPixmap(flag));
    }
    combo->addItem(flagIcon, i18n("%1 - %2 (%3)", clabel, locale.nativeLanguageName(), locale.name()) , QVariant(cvalue));
}

void setCombo(QComboBox *combo, const QString &key)
{
    int ix = combo->findData(key);
    if (ix > -1) {
        combo->setCurrentIndex(ix);
    }
}

void KCMFormats::readConfig()
{
    m_config = KConfigGroup(KSharedConfig::openConfig(configFile), "Formats");

    bool useDetailed = m_config.readEntry("useDetailed", false);
    m_ui->checkDetailed->setChecked(useDetailed);

    setCombo(m_ui->comboGlobal, m_config.readEntry(lcLang, QString()));

    setCombo(m_ui->comboNumbers, m_config.readEntry(lcNumeric, QString()));
    setCombo(m_ui->comboTime, m_config.readEntry(lcTime, QString()));
    setCombo(m_ui->comboCollate, m_config.readEntry(lcCollate, QString()));
    setCombo(m_ui->comboCurrency, m_config.readEntry(lcMonetary, QString()));
    setCombo(m_ui->comboMeasurement, m_config.readEntry(lcMeasurement, QString()));

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

void KCMFormats::writeExports()
{

    const QString cvalue = m_ui->comboGlobal->currentData().toString();

    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/" + exportFile;

    QString script(QStringLiteral("# Generated script, do not edit\n"));
    script.append(QStringLiteral("# Exports language-format specific env vars from startkde.\n"));
    script.append(QStringLiteral("# This script has been generated from kcmshell5 formats.\n"));
    script.append(QStringLiteral("# It will automatically be overwritten from there.\n"));
    m_config = KConfigGroup(KSharedConfig::openConfig(configFile), "Formats");

    const QString _export = QStringLiteral("export ");

    const QString lang = m_config.readEntry(lcLang, QString());
    if (!lang.isEmpty()) {
        script.append(_export + lcLang + QLatin1Char('=') + lang + QLatin1Char('\n'));
    }

    const QString numeric = m_config.readEntry(lcNumeric, QString());
    if (!numeric.isEmpty()) {
        script.append(_export + lcNumeric + QLatin1Char('=') + numeric + QLatin1Char('\n'));
    }

    const QString time = m_config.readEntry(lcTime, QString());
    if (!time.isEmpty()) {
        script.append(_export + lcTime + QLatin1Char('=') + time + QLatin1Char('\n'));
    }

    const QString monetary = m_config.readEntry(lcMonetary, QString());
    if (!monetary.isEmpty()) {
        script.append(_export + lcMonetary + QLatin1Char('=') + monetary + QLatin1Char('\n'));
    }

    const QString measurement = m_config.readEntry(lcMeasurement, QString());
    if (!measurement.isEmpty()) {
        script.append(_export + lcMeasurement + QLatin1Char('=') + measurement + QLatin1Char('\n'));
    }

    const QString collate = m_config.readEntry(lcCollate, QString());
    if (!collate.isEmpty()) {
        script.append(_export + lcCollate + QLatin1Char('=') + collate + QLatin1Char('\n'));
    }

    const QString ctype = m_config.readEntry(lcCtype, QString());
    if (!ctype.isEmpty()) {
        script.append(_export + lcCtype + QLatin1Char('=') + ctype + QLatin1Char('\n'));
    }

    QFile file(configPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    qDebug() << "Wrote script: " << configPath << "\n" << script;
    out << script;
    file.close();
}

void KCMFormats::save()
{
    writeConfig();
    writeExports();
    KMessageBox::information(this, i18n("Your changes will take effect the next time you log in."),
                             i18n("Format Settings Changed"), "FormatSettingsChanged");
}

void KCMFormats::defaults()
{
    m_ui->checkDetailed->setChecked(false);
    m_ui->comboGlobal->setCurrentIndex(0);
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

    if (useDetailed) {
        nloc = QLocale(m_ui->comboNumbers->currentData().toString());
        tloc = QLocale(m_ui->comboTime->currentData().toString());
        cloc = QLocale(m_ui->comboCurrency->currentData().toString());
        mloc = QLocale(m_ui->comboMeasurement->currentData().toString());
    } else {
        nloc = QLocale(m_ui->comboGlobal->currentData().toString());
        tloc = QLocale(m_ui->comboGlobal->currentData().toString());
        cloc = QLocale(m_ui->comboGlobal->currentData().toString());
        mloc = QLocale(m_ui->comboGlobal->currentData().toString());
    }

    const QString numberExample = nloc.toString(1000.01);
    const QString timeExample = tloc.toString(QDateTime::currentDateTime());
    const QString currencyExample = cloc.toCurrencyString(24);

    QString measurementSetting;
    if (mloc.measurementSystem() == QLocale::ImperialUKSystem) {
        measurementSetting = i18nc("Measurement combobox", "Imperial UK");
    } else if (mloc.measurementSystem() == QLocale::ImperialUSSystem) {
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
