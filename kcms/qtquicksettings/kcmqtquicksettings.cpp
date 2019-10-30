/*  This file is part of the KDE's Plasma desktop
 *  Copyright 2017 David Edmundson <davidedmundson@kde.org>
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

#include "kcmqtquicksettings.h"

#include <KPluginFactory>
#include <KAboutData>
#include <QDebug>

#include "ui_kcmqtquicksettingswidget.h"
#include "renderersettings.h"

K_PLUGIN_FACTORY(KCMQtQuickSettingsFactory, registerPlugin<KCMQtQuickSettingsModule>();)

KCMQtQuickSettingsModule::KCMQtQuickSettingsModule(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args),
      m_ui(new Ui::KCMQtQuickSettingsWidget),
      m_settings(new PlasmaQtQuickSettings::RendererSettings(KSharedConfig::openConfig(QStringLiteral("kdeglobals"))))
{
    KAboutData *about = new KAboutData(QStringLiteral("Plasma QtQuick Settings"),
                                       i18n("Plasma QtQuick Settings"),
                                       QString(),
                                       i18n("Configure Plasma QtQuick Settings"),
                                       KAboutLicense::GPL);
    about->addAuthor(i18n("David Edmundson"), i18n("Maintainer"), QStringLiteral("davidedmundson@kde.org"));
    setAboutData(about);

    m_ui->setupUi(this);
    addConfig(m_settings.get(), this);
}

KCMQtQuickSettingsModule::~KCMQtQuickSettingsModule()
{
}

#include "kcmqtquicksettings.moc"
