/*
 * main.cpp
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 * KDE Frameworks 5 port Copyright (C) 2013 Jonathan Riddell <jr@jriddell.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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
 */

#include "main.h"

#include <QVBoxLayout>

#include <KAboutData>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KLocalizedString>

#include "icons.h"
#include "iconthemes.h"

K_PLUGIN_FACTORY(IconsFactory, registerPlugin<IconModule>();)

IconModule::IconModule(QWidget *parent, const QVariantList &)
  : KCModule(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0);

  tab = new QTabWidget(this);
  layout->addWidget(tab);

  tab1 = new IconThemesConfig(this);
  tab1->setObjectName( QLatin1String( "themes" ) );
  tab->addTab(tab1, i18n("&Theme"));
  connect(tab1, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  tab2 = new KIconConfig(this);
  tab2->setObjectName( QLatin1String( "effects" ) );
  tab->addTab(tab2, i18n("Ad&vanced"));
  connect(tab2, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  KAboutData* about = new KAboutData("kcmicons", i18n("Icons"), QStringLiteral("1.0"),
                                     i18n("Icons Control Panel Module"), KAboutLicense::GPL,
                                     i18n("(c) 2000-2003 Geert Jansen"));
  about->addAuthor(i18n("Geert Jansen"), QString(), "jansen@kde.org");
  about->addAuthor(i18n("Antonio Larrosa Jimenez"), QString(), "larrosa@kde.org");
  about->addCredit(i18n("Torsten Rahn"), QString(), "torsten@kde.org");
  about->addAuthor(i18n("Jonathan Riddell"), QString(), "jr@jriddell.org");
  setAboutData( about );
}


void IconModule::load()
{
  tab1->load();
  tab2->load();
}


void IconModule::save()
{
  tab1->save();
  tab2->save();
}


void IconModule::defaults()
{
  tab1->defaults();
  tab2->defaults();
}


void IconModule::moduleChanged(bool state)
{
  emit changed(state);
}

QString IconModule::quickHelp() const
{
  return i18n("<h1>Icons</h1>"
    "This module allows you to choose the icons for your desktop.<p>"
    "To choose an icon theme, click on its name and apply your choice by pressing the \"Apply\" button below. If you do not want to apply your choice you can press the \"Reset\" button to discard your changes.</p>"
    "<p>By pressing the \"Install Theme File...\" button you can install your new icon theme by writing its location in the box or browsing to the location."
    " Press the \"OK\" button to finish the installation.</p>"
    "<p>The \"Remove Theme\" button will only be activated if you select a theme that you installed using this module."
    " You are not able to remove globally installed themes here.</p>"
    "<p>You can also specify effects that should be applied to the icons.</p>");
}

#include "main.moc"
#include "moc_main.cpp"

