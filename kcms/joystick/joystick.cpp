/*
    This file is part of the KDE Control Center Module for Joysticks

    SPDX-FileCopyrightText: 2003 Martin Koller <kollix@aon.at>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joystick.h"
#include "joydevice.h"
#include "joywidget.h"

#include <KAboutData>
#include <KLocalizedString>
#include <QDialog>
#include <QStyle>

#include <KPluginFactory>
#include <stdio.h>

#include <QVBoxLayout>

K_PLUGIN_CLASS_WITH_JSON(Joystick, "kcm_joystick.json")

Joystick::Joystick(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    setButtons(Help | Default);

    joyWidget = new JoyWidget(widget());

    QVBoxLayout *top = new QVBoxLayout(widget());
    top->setContentsMargins(0, 0, 0, 0);
    top->addWidget(joyWidget);
}

void Joystick::load()
{
    joyWidget->init();
}

void Joystick::defaults()
{
    joyWidget->resetCalibration();
}

#include "joystick.moc"
