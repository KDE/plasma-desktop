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
#include <KSharedConfig>


K_PLUGIN_FACTORY_WITH_JSON(KCMFormatsFactory, "formats.json", registerPlugin<KCMFormats>();)

const static QString configFile = QStringLiteral("kdeformats");
const static QString exportFile = QStringLiteral("export-formats-settings.sh");

const static QString lcNumeric = QStringLiteral("LC_NUMERIC");
const static QString lcTime = QStringLiteral("LC_TIME");
const static QString lcMonetary = QStringLiteral("LC_MONETARY");
const static QString lcMeasurement = QStringLiteral("LC_MEASUREMENT");
const static QString lcGlobal = QStringLiteral("Global");


KCMFormats::KCMFormats(QWidget *parent, const QVariantList &args)
  : KCModule(parent, args)
{
    setQuickHelp( i18n("<h1>Formats</h1>"
    " You can configure the formats used for time, dates, money and other numbers here."));

    m_ui = new Ui::KCMFormatsWidget;
    m_ui->setupUi(this);
    m_combos << m_ui->comboGlobal
             << m_ui->comboNumbers
             << m_ui->comboTime
             << m_ui->comboCurrency
             << m_ui->comboMeasurement;
}

void KCMFormats::load()
{
    QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);

    foreach (QComboBox* combo, m_combos) {
        initCombo(combo);
    }

    foreach (const QLocale &l, allLocales) {
        foreach (QComboBox* combo, m_combos) {
            addLocaleToCombo(combo, l);
        }
    }

    readConfig();

    foreach (QComboBox* combo, m_combos) {
        connectCombo(combo);
    }
    connect(m_ui->radioAll, &QAbstractButton::toggled, [=](bool enabled){
        qDebug() << "Changed" << enabled;
        emit changed(true);
    } );

    qDebug() << "Loaded locales: " << allLocales.count();
    emit changed(false);
}

void KCMFormats::initCombo(QComboBox* combo)
{
    const QString clabel = i18n("Use system default");
    combo->addItem(clabel, QString());
}

void KCMFormats::connectCombo(QComboBox* combo)
{
    connect(combo, &QComboBox::currentTextChanged, [=](const QString txt){
        qDebug() << "Changed" << txt;
        emit changed(true);
    } );
}

void KCMFormats::addLocaleToCombo(QComboBox* combo, const QLocale &locale)
{
    const QString clabel = locale.countryToString(locale.country());
    const QString cvalue = locale.bcp47Name();
    combo->addItem(i18n("%1 (%2)", clabel, cvalue) , QVariant(cvalue));
}

void setCombo(QComboBox* combo, const QString &key) {
    int ix = combo->findData(key);
    if (ix > -1) {
        combo->setCurrentIndex(ix);
    }
}

void KCMFormats::readConfig()
{
    m_config = KConfigGroup(KSharedConfig::openConfig(configFile), "Formats");

    bool useDetailed = m_config.readEntry("useDetailed", false);
    m_ui->radioAll->setChecked(!useDetailed);
    m_ui->radioDetailed->setChecked(useDetailed);

    setCombo(m_ui->comboGlobal, m_config.readEntry(lcGlobal, QString()));

    setCombo(m_ui->comboNumbers, m_config.readEntry(lcNumeric, QString()));
    setCombo(m_ui->comboTime, m_config.readEntry(lcTime, QString()));
    setCombo(m_ui->comboCurrency, m_config.readEntry(lcMonetary, QString()));
    setCombo(m_ui->comboMeasurement, m_config.readEntry(lcMeasurement, QString()));
}

void KCMFormats::writeConfig()
{
    m_config = KConfigGroup(KSharedConfig::openConfig(configFile), "Formats");
    const QString _global = m_ui->comboGlobal->currentData().toString();
    if (m_ui->radioAll->isChecked()) {
        m_config.deleteEntry("useDetailed");
        if (_global.isEmpty()) {
            qDebug() << "Deleting them all";
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
            qDebug() << "_numeric: " << _global;
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
            qDebug() << "_time: " << _time;
            m_config.writeEntry(lcTime, _time);
        }

        const QString _monetary = m_ui->comboCurrency->currentData().toString();
        if (_monetary.isEmpty()) {
            m_config.deleteEntry(lcMonetary);
        } else {
            qDebug() << "_monetary: " << _monetary;
            m_config.writeEntry(lcMonetary, _monetary);
        }

        const QString _measurement = m_ui->comboMeasurement->currentData().toString();
        if (_measurement.isEmpty()) {
            m_config.deleteEntry(lcMeasurement);
        } else {
            qDebug() << "_measurement: " << _measurement;
            m_config.writeEntry(lcMeasurement, _measurement);
        }
    }

    m_config.sync();
}

void KCMFormats::writeExports()
{

    QString cvalue = m_ui->comboGlobal->currentData().toString();

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    configPath.append("/"+exportFile);
    qDebug() << "Saving to filename: " << configPath;

    QString script;
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

    const QString _measurement = m_config.readEntry(lcMeasurement, QString());
    if (!_measurement.isEmpty()) {
        script.append(_export + lcMeasurement+ QLatin1Char('=') + _measurement + QLatin1Char('\n'));
    }

    const QString _global = m_config.readEntry(lcGlobal, QString());
    if (!_global.isEmpty()) {
        script.append(_export + QStringLiteral("LC_COLLATE") + QLatin1Char('=') + _global + QLatin1Char('\n'));
        script.append(_export + QStringLiteral("LC_CTYPE") + QLatin1Char('=') + _global + QLatin1Char('\n'));
    }

    QFile file(configPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    qDebug() << "Wrote shellscript: " << "\n" << script;
    out << script;
    file.close();
}

void KCMFormats::save()
{
    qDebug() << "Formats save:";
    writeConfig();
    writeExports();
}

void KCMFormats::defaults()
{
    qDebug() << "Formats defaults:";
}

#include "kcmformats.moc"
