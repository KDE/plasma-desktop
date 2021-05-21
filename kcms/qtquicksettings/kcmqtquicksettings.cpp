/*  This file is part of the KDE's Plasma desktop
    SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmqtquicksettings.h"

#include <KAboutData>
#include <KPluginFactory>
#include <QDebug>

#include "renderersettings.h"
#include "ui_kcmqtquicksettingswidget.h"

K_PLUGIN_FACTORY(KCMQtQuickSettingsFactory, registerPlugin<KCMQtQuickSettingsModule>();)

KCMQtQuickSettingsModule::KCMQtQuickSettingsModule(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_ui(new Ui::KCMQtQuickSettingsWidget)
    , m_settings(new PlasmaQtQuickSettings::RendererSettings(KSharedConfig::openConfig(QStringLiteral("kdeglobals"))))
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
