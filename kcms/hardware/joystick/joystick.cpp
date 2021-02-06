/***************************************************************************
 *   Copyright (C) 2003 by Martin Koller                                   *
 *   kollix@aon.at                                                         *
 *   This file is part of the KDE Control Center Module for Joysticks      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

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

//---------------------------------------------------------------------------------------------

K_PLUGIN_FACTORY(JoystickFactory, registerPlugin<Joystick>();)

//---------------------------------------------------------------------------------------------

Joystick::Joystick(QWidget *parent, const QVariantList &)
    : KCModule(parent)
{
    setButtons(Help | Default);
    setAboutData(new KAboutData(QStringLiteral("kcmjoystick"),
                                i18n("KDE Joystick Control Module"),
                                QStringLiteral("1.0"),
                                i18n("KDE System Settings Module to test Joysticks"),
                                KAboutLicense::GPL,
                                i18n("(c) 2004, Martin Koller"),
                                QString(),
                                QStringLiteral("kollix@aon.at")));

    setQuickHelp(
        i18n("<h1>Joystick</h1>"
             "This module helps to check if your joystick is working correctly.<br />"
             "If it delivers wrong values for the axes, you can try to solve this with "
             "the calibration.<br />"
             "This module tries to find all available joystick devices "
             "by checking /dev/js[0-4] and /dev/input/js[0-4]<br />"
             "If you have another device file, enter it in the combobox.<br />"
             "The Buttons list shows the state of the buttons on your joystick, the Axes list "
             "shows the current value for all axes.<br />"
             "NOTE: the current Linux device driver (Kernel 2.4, 2.6) can only autodetect"
             "<ul>"
             "<li>2-axis, 4-button joystick</li>"
             "<li>3-axis, 4-button joystick</li>"
             "<li>4-axis, 4-button joystick</li>"
             "<li>Saitek Cyborg 'digital' joysticks</li>"
             "</ul>"
             "(For details you can check your Linux source/Documentation/input/joystick.txt)"));

    joyWidget = new JoyWidget(this);

    QVBoxLayout *top = new QVBoxLayout(this);
    top->setContentsMargins(0, 0, 0, 0);
    top->setSpacing(style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    top->addWidget(joyWidget);
}

//---------------------------------------------------------------------------------------------

void Joystick::load()
{
    joyWidget->init();
}

//---------------------------------------------------------------------------------------------

void Joystick::defaults()
{
    joyWidget->resetCalibration();
}

//---------------------------------------------------------------------------------------------

#include "joystick.moc"
