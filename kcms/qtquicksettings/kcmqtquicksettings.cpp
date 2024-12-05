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

K_PLUGIN_CLASS_WITH_JSON(KCMQtQuickSettingsModule, "kcm_qtquicksettings.json")

KCMQtQuickSettingsModule::KCMQtQuickSettingsModule(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_ui(new Ui::KCMQtQuickSettingsWidget)
    , m_settings(new PlasmaQtQuickSettings::RendererSettings(KSharedConfig::openConfig(QStringLiteral("kdeglobals"))))
{
    m_ui->setupUi(widget());
    addConfig(m_settings.get(), widget());
}

KCMQtQuickSettingsModule::~KCMQtQuickSettingsModule()
{
}

#include "kcmqtquicksettings.moc"

#include "moc_kcmqtquicksettings.cpp"
