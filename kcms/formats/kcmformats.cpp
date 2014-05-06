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


#include "ui_kcmformatswidget.h"
#include <QApplication>
#include <QLocale>
#include <QDebug>

#include "kcmformats.h"
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
        const QString cvalue = l.bcp47Name().replace('-', '_');
        qDebug() << "Found locale: " << clabel << l.bcp47Name();
        m_ui->countriesCombo->addItem(i18n("%1 (%2)", clabel, cvalue) , QVariant(cvalue));
    }
    qDebug() << "Loaded.";
    emit changed(false);
}

void KCMFormats::save()
{
    qDebug() << "Formats save:";
}

void KCMFormats::defaults()
{
    qDebug() << "Formats defaults:";
}

#include "kcmformats.moc"
