/*
 *  kcmformats.cpp
 *  Copyright 2014 Sebastian Kuegler <sebas@kde.org>
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

const static QString configFile = QStringLiteral("kdeformats");
const static QString exportFile = QStringLiteral("export-formats-settings.sh");

const static QString lcGlobal = QStringLiteral("Global");

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
             << m_ui->comboMeasurement;
}

KCMFormats::~KCMFormats()
{
    delete m_ui;
}

void KCMFormats::load()
{
    QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);

    foreach(QComboBox * combo, m_combos) {
        initCombo(combo);
    }

    foreach(const QLocale & l, allLocales) {
        foreach(QComboBox * combo, m_combos) {
            if (combo != m_ui->comboMeasurement) {
                addLocaleToCombo(combo, l);
            }
        }
    }

    // Standard says that Imperial == en_US
    m_ui->comboMeasurement->addItem(i18nc("Measurement units combo", "Imperial"), QVariant("en_US"));
    // Random sane country
    m_ui->comboMeasurement->addItem(i18nc("Measurement units combo", "Metric"), QVariant("nl_NL"));

    readConfig();

    foreach(QComboBox * combo, m_combos) {
        connectCombo(combo);
    }

    connect(m_ui->checkDetailed, &QAbstractButton::toggled, [ = ]() {
        //qDebug() << "Changed" << enabled;
        //Q_UNUSED(enabled)
        updateExample();
        updateEnabled();
        emit changed(true);
    });

    qDebug() << "Loaded locales: " << allLocales.count();

    updateEnabled();
    updateExample();
    emit changed(false);
}

void KCMFormats::initCombo(QComboBox *combo)
{
    const QString clabel = i18n("No change");
    combo->addItem(clabel, QString());
}

void KCMFormats::connectCombo(QComboBox *combo)
{
    connect(combo, &QComboBox::currentTextChanged, [ = ]() {
        //qDebug() << "Changed combo" << txt;
        emit changed(true);
        updateExample();
    });
}

void KCMFormats::addLocaleToCombo(QComboBox *combo, const QLocale &locale)
{
    const QString clabel = locale.countryToString(locale.country());
    //const QString clabel2 = locale.uiLanguages().join(", ");
    const QString cvalue = locale.bcp47Name();

    QString flagcode = "de";
    const QStringList _split = locale.name().split('_');
    if (_split.count() > 1) {
        flagcode = _split[1].toLower();
    }
    QString flag(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("locale/") + QString::fromLatin1("l10n/%1/flag.png").arg(flagcode)));
    QIcon _icon;
    if (!flag.isEmpty()) {
        _icon = QIcon(QPixmap(flag));
    }
    combo->addItem(_icon, i18n("%1 (%2)", clabel, locale.name()) , QVariant(cvalue));
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

    setCombo(m_ui->comboGlobal, m_config.readEntry(lcGlobal, QString()));

    setCombo(m_ui->comboNumbers, m_config.readEntry(lcNumeric, QString()));
    setCombo(m_ui->comboTime, m_config.readEntry(lcTime, QString()));
    setCombo(m_ui->comboCurrency, m_config.readEntry(lcMonetary, QString()));

    QString _ms = m_config.readEntry(lcMeasurement, QString());
    if (_ms.isEmpty()) {
        m_ui->comboMeasurement->setCurrentIndex(0);
    } else {
        if (QLocale(_ms).measurementSystem() == QLocale::ImperialSystem) {
            m_ui->comboMeasurement->setCurrentIndex(1);
        } else {
            m_ui->comboMeasurement->setCurrentIndex(2);
        }
    }
    updateEnabled();
}

void KCMFormats::writeConfig()
{
    m_config = KConfigGroup(KSharedConfig::openConfig(configFile), "Formats");

    // _global ends up empty here when OK button is clicked from kcmshell5,
    // apparently the data in the combo is gone by the time save() is called.
    // This might be a problem in KCModule, but does not directly affect us
    // since within systemsettings, it works fine.
    if (m_ui->comboGlobal->count() == 0) {
        qWarning() << "Couldn't read data from UI, writing configuration failed.";
        return;
    }
    const QString _global = m_ui->comboGlobal->currentData().toString();

    qDebug() << "GLOBAL : " << _global << m_ui->comboGlobal->count();
    if (!m_ui->checkDetailed->isChecked()) {
        m_config.deleteEntry("useDetailed");
        if (_global.isEmpty()) {
            //qDebug() << "Deleting them all";
            m_config.deleteEntry(lcGlobal);
            m_config.deleteEntry(lcNumeric);
            m_config.deleteEntry(lcTime);
            m_config.deleteEntry(lcMonetary);
            m_config.deleteEntry(lcMeasurement);
        } else {
            m_config.writeEntry(lcGlobal, _global);
            m_config.writeEntry(lcNumeric, _global);
            m_config.writeEntry(lcTime, _global);
            m_config.writeEntry(lcMonetary, _global);
            m_config.writeEntry(lcMeasurement, _global);
        }
    } else { // Save detailed settings
        m_config.writeEntry("useDetailed", true);

        if (_global.isEmpty()) {
            m_config.deleteEntry(lcGlobal);
        } else {
            //qDebug() << "_numeric: " << _global;
            m_config.writeEntry(lcGlobal, _global);
        }

        const QString _numeric = m_ui->comboNumbers->currentData().toString();
        if (_numeric.isEmpty()) {
            m_config.deleteEntry(lcNumeric);
        } else {
            qDebug() << "_numeric: " << _numeric;
            m_config.writeEntry(lcNumeric, _numeric);
        }

        const QString _time = m_ui->comboTime->currentData().toString();
        if (_time.isEmpty()) {
            m_config.deleteEntry(lcTime);
        } else {
            //qDebug() << "_time: " << _time;
            m_config.writeEntry(lcTime, _time);
        }

        const QString _monetary = m_ui->comboCurrency->currentData().toString();
        if (_monetary.isEmpty()) {
            m_config.deleteEntry(lcMonetary);
        } else {
            //qDebug() << "_monetary: " << _monetary;
            m_config.writeEntry(lcMonetary, _monetary);
        }

        const QString _measurement = m_ui->comboMeasurement->currentData().toString();
        if (_measurement.isEmpty()) {
            m_config.deleteEntry(lcMeasurement);
        } else {
            //qDebug() << "_measurement: " << _measurement;
            m_config.writeEntry(lcMeasurement, _measurement);
        }
    }

    m_config.sync();
}

void KCMFormats::writeExports()
{

    QString cvalue = m_ui->comboGlobal->currentData().toString();

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    configPath.append("/" + exportFile);
    //qDebug() << "Saving to filename: " << configPath;

    QString script(QStringLiteral("# Generated script, do not edit\n"));
    script.append(QStringLiteral("# Exports language-format specific env vars from startkde.\n"));
    script.append(QStringLiteral("# This script has been generated from kcmshell5 formats.\n"));
    script.append(QStringLiteral("# It will automatically be overwritten from there.\n"));
    m_config = KConfigGroup(KSharedConfig::openConfig(configFile), "Formats");

    const QString _export = QStringLiteral("export ");

    const QString _numeric = m_config.readEntry(lcNumeric, QString());
    if (!_numeric.isEmpty()) {
        script.append(_export + lcNumeric + QLatin1Char('=') + _numeric + QLatin1Char('\n'));
    }

    const QString _time = m_config.readEntry(lcTime, QString());
    if (!_time.isEmpty()) {
        script.append(_export + lcTime + QLatin1Char('=') + _time + QLatin1Char('\n'));
    }

    const QString _monetary = m_config.readEntry(lcMonetary, QString());
    if (!_monetary.isEmpty()) {
        script.append(_export + lcMonetary + QLatin1Char('=') + _monetary + QLatin1Char('\n'));
    }

    QString _measurement = m_config.readEntry(lcMeasurement, QString());
    if (!_measurement.isEmpty()) {
        script.append(_export + lcMeasurement + QLatin1Char('=') + _measurement + QLatin1Char('\n'));
    }

    const QString _global = m_config.readEntry(lcGlobal, QString());
    if (!_global.isEmpty()) {
        script.append(_export + lcCollate + QLatin1Char('=') + _global + QLatin1Char('\n'));
        script.append(_export + lcCtype + QLatin1Char('=') + _global + QLatin1Char('\n'));
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
    //qDebug() << "Formats save:";
    writeConfig();
    writeExports();
    KMessageBox::information(this, i18n("Your changes take effect the next time you log in."),
                             i18n("Format Settings Changed"), "FormatSettingsChanged");
}

void KCMFormats::defaults()
{
    m_ui->checkDetailed->setChecked(false);
    m_ui->comboGlobal->setCurrentIndex(0);
}

void KCMFormats::updateEnabled()
{
    bool enabled = m_ui->checkDetailed->isChecked();

    m_ui->labelNumbers->setEnabled(enabled);
    m_ui->labelTime->setEnabled(enabled);
    m_ui->labelCurrency->setEnabled(enabled);
    m_ui->labelMeasurement->setEnabled(enabled);
    m_ui->comboNumbers->setEnabled(enabled);
    m_ui->comboTime->setEnabled(enabled);
    m_ui->comboCurrency->setEnabled(enabled);
    m_ui->comboMeasurement->setEnabled(enabled);
}

void KCMFormats::updateExample()
{
    bool useDetailed = m_ui->checkDetailed->isChecked();

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

    QString numberExample = nloc.toString(1000.01);
    QString timeExample = tloc.toString(QDateTime::currentDateTime());
    QString currencyExample = cloc.toCurrencyString(24);
    QString measurementExample;
    if (mloc.measurementSystem() == QLocale::ImperialSystem) {
        measurementExample = i18nc("Example for imperial units", "4 miles, 200 yards");
    } else {
        measurementExample = i18nc("Example for metric units", "6km, 620m");
    }

//     qDebug() << "NumberExample: " << numberExample;
//     qDebug() << "TimeExample: " << timeExample;
//    qDebug() << "CurrencyExample: " << currencyExample;
    //qDebug() << "MeasureExample: " << currencyExample << mloc.name();

    m_ui->exampleNumbers->setText(numberExample);
    m_ui->exampleTime->setText(timeExample);
    m_ui->exampleCurrency->setText(currencyExample);
    m_ui->exampleMeasurement->setText(measurementExample);
}

#include "kcmformats.moc"
