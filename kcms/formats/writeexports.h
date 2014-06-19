/*
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

#include <QDebug>
#include <QFile>
#include <QStandardPaths>

#include <KSharedConfig>

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


void writeExports()
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/" + exportFile;

    QString script(QStringLiteral("# Generated script, do not edit\n"));
    script.append(QStringLiteral("# Exports language-format specific env vars from startkde.\n"));
    script.append(QStringLiteral("# This script has been generated from kcmshell5 formats.\n"));
    script.append(QStringLiteral("# It will automatically be overwritten from there.\n"));
    KConfigGroup formatsConfig = KConfigGroup(KSharedConfig::openConfig(configFile), "Formats");
    KConfigGroup languageConfig = KConfigGroup(KSharedConfig::openConfig(configFile), "Translations");

    const QString _export = QStringLiteral("export ");

    // Formats, uses LC_* and LANG variables
    const QString lang = formatsConfig.readEntry(lcLang, QString());
    if (!lang.isEmpty()) {
        script.append(_export + lcLang + QLatin1Char('=') + lang + QLatin1Char('\n'));
    }

    const QString numeric = formatsConfig.readEntry(lcNumeric, QString());
    if (!numeric.isEmpty()) {
        script.append(_export + lcNumeric + QLatin1Char('=') + numeric + QLatin1Char('\n'));
    }

    const QString time = formatsConfig.readEntry(lcTime, QString());
    if (!time.isEmpty()) {
        script.append(_export + lcTime + QLatin1Char('=') + time + QLatin1Char('\n'));
    }

    const QString monetary = formatsConfig.readEntry(lcMonetary, QString());
    if (!monetary.isEmpty()) {
        script.append(_export + lcMonetary + QLatin1Char('=') + monetary + QLatin1Char('\n'));
    }

    const QString measurement = formatsConfig.readEntry(lcMeasurement, QString());
    if (!measurement.isEmpty()) {
        script.append(_export + lcMeasurement + QLatin1Char('=') + measurement + QLatin1Char('\n'));
    }

    const QString collate = formatsConfig.readEntry(lcCollate, QString());
    if (!collate.isEmpty()) {
        script.append(_export + lcCollate + QLatin1Char('=') + collate + QLatin1Char('\n'));
    }

    const QString ctype = formatsConfig.readEntry(lcCtype, QString());
    if (!ctype.isEmpty()) {
        script.append(_export + lcCtype + QLatin1Char('=') + ctype + QLatin1Char('\n'));
    }

    // Translations, uses LANGUAGE variable
    const QString language = languageConfig.readEntry(lcLanguage, QString());
    if (!language.isEmpty()) {
        script.append(_export + lcLanguage + QLatin1Char('=') + language + QLatin1Char('\n'));
    }



    QFile file(configPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    qDebug() << "Wrote script: " << configPath << "\n" << script;
    out << script;
    file.close();
}
