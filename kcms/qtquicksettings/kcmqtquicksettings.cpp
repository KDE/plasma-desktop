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
    m_ui->backendCombo->addItem(i18n("Automatic"), QVariant(QStringLiteral()));//so data matches empty config
    m_ui->backendCombo->addItem(i18n("Open GL"), QVariant(QStringLiteral("opengl")));
    m_ui->backendCombo->addItem(i18n("Software"), QVariant(QStringLiteral("software")));

    m_ui->renderLoopCombo->addItem(i18n("Automatic"), QVariant(QStringLiteral()));
    m_ui->renderLoopCombo->addItem(i18n("Basic"), QVariant(QStringLiteral("basic")));
    m_ui->renderLoopCombo->addItem(i18n("Threaded"), QVariant(QStringLiteral("threaded")));

    connect(m_ui->backendCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KCMQtQuickSettingsModule::*)()>(&KCMQtQuickSettingsModule::changed));
    connect(m_ui->renderLoopCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KCMQtQuickSettingsModule::*)()>(&KCMQtQuickSettingsModule::changed));

    connect(m_ui->glCoreProfileCheckbox, &QCheckBox::stateChanged, this, static_cast<void (KCMQtQuickSettingsModule::*)()>(&KCMQtQuickSettingsModule::changed));
}

KCMQtQuickSettingsModule::~KCMQtQuickSettingsModule()
{
}

void KCMQtQuickSettingsModule::load()
{
    m_ui->backendCombo->setCurrentIndex(m_ui->backendCombo->findData(m_settings->sceneGraphBackend()));
    m_ui->renderLoopCombo->setCurrentIndex(m_ui->renderLoopCombo->findData(m_settings->renderLoop()));
    m_ui->glCoreProfileCheckbox->setChecked(m_settings->forceGlCoreProfile());
}

void KCMQtQuickSettingsModule::save()
{
    m_settings->setSceneGraphBackend(m_ui->backendCombo->currentData().toString());
    m_settings->setRenderLoop(m_ui->renderLoopCombo->currentData().toString());
    m_settings->setForceGlCoreProfile(m_ui->glCoreProfileCheckbox->isChecked());
    m_settings->save();
}

void KCMQtQuickSettingsModule::defaults()
{
    m_ui->backendCombo->setCurrentIndex(0);
    m_ui->renderLoopCombo->setCurrentIndex(0);
    m_ui->glCoreProfileCheckbox->setChecked(false);
}

#include "kcmqtquicksettings.moc"
