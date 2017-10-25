/*
 * mouse.cpp
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Layout management, enhancements:
 * Copyright (c) 1999 Dirk A. Mueller <dmuell@gmx.net>
 *
 * SC/DC/AutoSelect/ChangeCursor:
 * Copyright (c) 2000 David Faure <faure@kde.org>
 *
 * Double click interval, drag time & dist
 * Copyright (c) 2000 Bernd Gehrmann
 *
 * Large cursor support
 * Visual activation TODO: speed
 * Copyright (c) 2000 Rik Hemsley <rik@kde.org>
 *
 * White cursor support
 * TODO: give user the option to choose a certain cursor font
 * -> Theming
 *
 * General/Advanced tabs
 * Copyright (c) 2000 Brad Hughes <bhughes@trolltech.com>
 *
 * redesign for KDE 2.2
 * Copyright (c) 2001 Ralf Nolden <nolden@kde.org>
 *
 * Logitech mouse support
 * Copyright (C) 2004 Brad Hards <bradh@frogmouth.net>
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

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QWhatsThis>
#include <QTabWidget>

#include <ktoolinvocation.h>
#include <klocalizedstring.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <KPluginFactory>
#include <KPluginLoader>

#include <config-workspace.h>

#include "mouse.h"
#include "mousebackend.h"
#include "mousesettings.h"

#include <kglobalsettings.h>

#undef Below

#include "../migrationlib/kdelibs4config.h"

K_PLUGIN_FACTORY(MouseConfigFactory,
        registerPlugin<MouseConfig>(); // mouse
        )

MouseConfig::MouseConfig(QWidget *parent, const QVariantList &args)
  : KCModule(parent, args),
    backend(MouseBackend::implementation())
{
    setupUi(this);

    handedGroup->setId(rightHanded, static_cast<int>(MouseHanded::Right));
    handedGroup->setId(leftHanded, static_cast<int>(MouseHanded::Left));

    connect(handedGroup, SIGNAL(buttonClicked(int)), this, SLOT(changed()));
    connect(handedGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotHandedChanged(int)));
    connect(doubleClick, SIGNAL(clicked()), SLOT(changed()));

    connect(singleClick, SIGNAL(clicked()), this, SLOT(changed()));
    connect(cbScrollPolarity, SIGNAL(clicked()), this, SLOT(changed()));
    connect(cbScrollPolarity, SIGNAL(clicked()), this, SLOT(slotScrollPolarityChanged()));

    connect(accel, SIGNAL(valueChanged(double)), this, SLOT(changed()));
    connect(thresh, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(thresh, SIGNAL(valueChanged(int)), this, SLOT(slotThreshChanged(int)));
    slotThreshChanged(thresh->value());

    // It would be nice if the user had a test field.
    // Selecting such values in milliseconds is not intuitive
    connect(doubleClickInterval, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    connect(dragStartTime, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    connect(dragStartDist, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(dragStartDist, SIGNAL(valueChanged(int)), this, SLOT(slotDragStartDistChanged(int)));
    slotDragStartDistChanged(dragStartDist->value());

    connect(wheelScrollLines, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(wheelScrollLines, SIGNAL(valueChanged(int)), SLOT(slotWheelScrollLinesChanged(int)));
    slotWheelScrollLinesChanged(wheelScrollLines->value());

    connect(mouseKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));
    connect(mouseKeys, SIGNAL(clicked()), this, SLOT(changed()));
    connect(mk_delay, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(mk_interval, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(mk_time_to_max, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(mk_max_speed, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(mk_curve, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    settings = new MouseSettings;

    KAboutData* about = new KAboutData(QStringLiteral("kcmmouse"), i18n("Mouse"),
                                       QStringLiteral("1.0"), QString(),
                                       KAboutLicense::GPL,
                                       i18n("(c) 1997 - 2005 Mouse developers"));
    about->addAuthor(i18n("Patrick Dowler"));
    about->addAuthor(i18n("Dirk A. Mueller"));
    about->addAuthor(i18n("David Faure"));
    about->addAuthor(i18n("Bernd Gehrmann"));
    about->addAuthor(i18n("Rik Hemsley"));
    about->addAuthor(i18n("Brad Hughes"));
    about->addAuthor(i18n("Ralf Nolden"));
    about->addAuthor(i18n("Brad Hards"));
    setAboutData(about);
}

void MouseConfig::checkAccess()
{
    mk_delay->setEnabled(mouseKeys->isChecked());
    mk_interval->setEnabled(mouseKeys->isChecked());
    mk_time_to_max->setEnabled(mouseKeys->isChecked());
    mk_max_speed->setEnabled(mouseKeys->isChecked());
    mk_curve->setEnabled(mouseKeys->isChecked());
}


MouseConfig::~MouseConfig()
{
    delete settings;
}

double MouseConfig::getAccel()
{
    return accel->value();
}

void MouseConfig::setAccel(double val)
{
    accel->setValue(val);
}

int MouseConfig::getThreshold()
{
    return thresh->value();
}

void MouseConfig::setThreshold(int val)
{
    thresh->setValue(val);
}


MouseHanded MouseConfig::getHandedness()
{
    if (rightHanded->isChecked())
        return MouseHanded::Right;
    else
        return MouseHanded::Left;
}

void MouseConfig::setHandedness(MouseHanded val)
{
    rightHanded->setChecked(false);
    leftHanded->setChecked(false);
    if (val == MouseHanded::Right) {
        rightHanded->setChecked(true);
        mousePix->setPixmap(KStandardDirs::locate("data", "kcminput/pics/mouse_rh.png"));
    }
    else {
        leftHanded->setChecked(true);
        mousePix->setPixmap(KStandardDirs::locate("data", "kcminput/pics/mouse_lh.png"));
    }
    settings->handedNeedsApply = true;
}

void MouseConfig::load()
{
    KConfig config("kcminputrc");
    settings->load(&config, backend);

    // settings->load will trigger backend->load so information will be avaialbe
    // here.
    // Only allow setting reversing scroll polarity if we have scroll buttons
    if (backend) {
        if (backend->supportScrollPolarity())
        {
            cbScrollPolarity->setEnabled(true);
            cbScrollPolarity->show();
        }
        else
        {
            cbScrollPolarity->setEnabled(false);
            cbScrollPolarity->hide();
        }
    }

    rightHanded->setEnabled(settings->handedEnabled);
    leftHanded->setEnabled(settings->handedEnabled);
    if (cbScrollPolarity->isEnabled())
        cbScrollPolarity->setEnabled(settings->handedEnabled);
    cbScrollPolarity->setChecked(settings->reverseScrollPolarity);

    setAccel(settings->accelRate);
    setThreshold(settings->thresholdMove);
    setHandedness(settings->handed);

    doubleClickInterval->setValue(settings->doubleClickInterval);
    dragStartTime->setValue(settings->dragStartTime);
    dragStartDist->setValue(settings->dragStartDist);
    wheelScrollLines->setValue(settings->wheelScrollLines);

    singleClick->setChecked(settings->singleClick);
    doubleClick->setChecked(!settings->singleClick);

    KConfig ac("kaccessrc");

    KConfigGroup group = ac.group("Mouse");
    mouseKeys->setChecked(group.readEntry("MouseKeys", false));
    mk_delay->setValue(group.readEntry("MKDelay", 160));

    int interval = group.readEntry("MKInterval", 5);
    mk_interval->setValue(interval);

    // Default time to reach maximum speed: 5000 msec
    int time_to_max = group.readEntry("MKTimeToMax", (5000+interval/2)/interval);
    time_to_max = group.readEntry("MK-TimeToMax", time_to_max*interval);
    mk_time_to_max->setValue(time_to_max);

    // Default maximum speed: 1000 pixels/sec
    //     (The old default maximum speed from KDE <= 3.4
    //     (100000 pixels/sec) was way too fast)
    long max_speed = group.readEntry("MKMaxSpeed", interval);
    max_speed = max_speed * 1000 / interval;
    if (max_speed > 2000)
        max_speed = 2000;
    max_speed = group.readEntry("MK-MaxSpeed", int(max_speed));
    mk_max_speed->setValue(max_speed);

    mk_curve->setValue(group.readEntry("MKCurve", 0));

    checkAccess();
    emit changed(false);
}

void MouseConfig::save()
{
    settings->accelRate = getAccel();
    settings->thresholdMove = getThreshold();
    settings->handed = getHandedness();

    settings->doubleClickInterval = doubleClickInterval->value();
    settings->dragStartTime = dragStartTime->value();
    settings->dragStartDist = dragStartDist->value();
    settings->wheelScrollLines = wheelScrollLines->value();
    settings->singleClick = !doubleClick->isChecked();
    settings->reverseScrollPolarity = cbScrollPolarity->isChecked();

    settings->apply(backend);
    KConfig config("kcminputrc");
    settings->save(&config);

    KConfig ac("kaccessrc");

    KConfigGroup group = ac.group("Mouse");

    int interval = mk_interval->value();
    group.writeEntry("MouseKeys", mouseKeys->isChecked());
    group.writeEntry("MKDelay", mk_delay->value());
    group.writeEntry("MKInterval", interval);
    group.writeEntry("MK-TimeToMax", mk_time_to_max->value());
    group.writeEntry("MKTimeToMax", (mk_time_to_max->value() + interval/2)/interval);
    group.writeEntry("MK-MaxSpeed", mk_max_speed->value());
    group.writeEntry("MKMaxSpeed", (mk_max_speed->value()*interval + 500)/1000);
    group.writeEntry("MKCurve", mk_curve->value());
    group.sync();

    // restart kaccess
    KToolInvocation::startServiceByDesktopName("kaccess");

    emit changed(false);
}

void MouseConfig::defaults()
{
    setThreshold(2);
    setAccel(2);
    setHandedness(MouseHanded::Right);
    cbScrollPolarity->setChecked(false);
    doubleClickInterval->setValue(400);
    dragStartTime->setValue(500);
    dragStartDist->setValue(4);
    wheelScrollLines->setValue(3);
    doubleClick->setChecked(!KDE_DEFAULT_SINGLECLICK);
    singleClick->setChecked(KDE_DEFAULT_SINGLECLICK);

    mouseKeys->setChecked(false);
    mk_delay->setValue(160);
    mk_interval->setValue(5);
    mk_time_to_max->setValue(5000);
    mk_max_speed->setValue(1000);
    mk_curve->setValue(0);

    checkAccess();
    changed();
}

/** No descriptions */
void MouseConfig::slotHandedChanged(int val)
{
    if (val == static_cast<int>(MouseHanded::Right))
        mousePix->setPixmap(KStandardDirs::locate("data", "kcminput/pics/mouse_rh.png"));
    else
        mousePix->setPixmap(KStandardDirs::locate("data", "kcminput/pics/mouse_lh.png"));
    settings->handedNeedsApply = true;
}

void MouseConfig::slotThreshChanged(int value)
{
    thresh->setSuffix(i18np(" pixel", " pixels", value));
}

void MouseConfig::slotDragStartDistChanged(int value)
{
    dragStartDist->setSuffix(i18np(" pixel", " pixels", value));
}

void MouseConfig::slotWheelScrollLinesChanged(int value)
{
    wheelScrollLines->setSuffix(i18np(" line", " lines", value));
}

void MouseConfig::slotScrollPolarityChanged()
{
    settings->handedNeedsApply = true;
}

#include "mouse.moc"
