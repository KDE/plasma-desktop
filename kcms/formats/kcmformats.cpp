/*
 *  kcmformats.cpp
 *  Copyright 2014 Sebastian Kugler <sebas@kde.org>
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


K_PLUGIN_FACTORY_WITH_JSON(KCMFormatsFactory, "formats.json", registerPlugin<KCMFormats>();)

KCMFormats::KCMFormats(QWidget *parent, const QVariantList &args)
  : KCModule(parent, args)
{
    setQuickHelp( i18n("<h1>Formats</h1>"
    " You can configure the formats used for time, dates, money and other numbers here."));

    m_ui = new Ui::KCMFormatsWidget;
    m_ui->setupUi(this);
}

void KCMFormats::load()
{
    QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);

    foreach (const QLocale &l, allLocales) {
        const QString clabel = l.countryToString(l.country());
        const QString cvalue = l.bcp47Name();
        //qDebug() << "Found locale: " << clabel << l.bcp47Name();
        m_ui->comboGlobal->addItem(i18n("%1 (%2)", clabel, cvalue) , QVariant(cvalue));
    }
    connect(m_ui->comboGlobal, &QComboBox::currentTextChanged, [=](const QString txt){
        qDebug() << "Changed" << txt;
        emit changed(true);
    } );
    qDebug() << "Loaded locales: " << allLocales.count();
    emit changed(false);
}

void KCMFormats::save()
{
    qDebug() << "Formats save:";
    QString cvalue = m_ui->comboGlobal->currentData().toString();

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    configPath.append("/export-formats-settings.sh");
    qDebug() << "Saving to filename: " << configPath;

    QFile file(configPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    QString shellscript;
    shellscript = ("export LC_ALL=" + cvalue);
    //shellscript.append("\necho setting language to cvalue");
    out << shellscript;
    qDebug() << "WRote shellscript: " << shellscript;

    // optional, as QFile destructor will already do it:
    file.close();
}

void KCMFormats::defaults()
{
    qDebug() << "Formats defaults:";
}

#include "kcmformats.moc"
