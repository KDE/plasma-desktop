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

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xutil.h>
#include <kglobalsettings.h>
#include <QX11Info>
#include <evdev-properties.h>

#undef Below

#include "../migrationlib/kdelibs4config.h"

K_PLUGIN_FACTORY(MouseConfigFactory,
        registerPlugin<MouseConfig>(); // mouse
        )
K_EXPORT_PLUGIN(MouseConfigFactory("kcminput"))

MouseConfig::MouseConfig(QWidget *parent, const QVariantList &args)
  : KCModule(parent, args)
{
    setupUi(this);

    handedGroup->setId(rightHanded, 0);
    handedGroup->setId(leftHanded, 1);

    connect(handedGroup, SIGNAL(buttonClicked(int)), this, SLOT(changed()));
    connect(handedGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotHandedChanged(int)));
    connect(doubleClick, SIGNAL(clicked()), SLOT(changed()));
    connect(singleClick, SIGNAL(clicked()), this, SLOT(changed()));

    // Only allow setting reversing scroll polarity if we have scroll buttons
    unsigned char map[20];
    if (QX11Info::isPlatformX11() && XGetPointerMapping(QX11Info::display(), map, 20) >= 5)
    {
      cbScrollPolarity->setEnabled(true);
      cbScrollPolarity->show();
    }
    else
    {
      cbScrollPolarity->setEnabled(false);
      cbScrollPolarity->hide();
    }
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


int MouseConfig::getHandedness()
{
    if (rightHanded->isChecked())
        return RIGHT_HANDED;
    else
        return LEFT_HANDED;
}

void MouseConfig::setHandedness(int val)
{
    rightHanded->setChecked(false);
    leftHanded->setChecked(false);
    if (val == RIGHT_HANDED) {
        rightHanded->setChecked(true);
        mousePix->setPixmap(KStandardDirs::locate("data", "kcminput/pics/mouse_rh.png"));
    }
    else {
        leftHanded->setChecked(true);
        mousePix->setPixmap(KStandardDirs::locate("data", "kcminput/pics/mouse_lh.png"));
    }
    settings->m_handedNeedsApply = true;
}

void MouseConfig::load()
{
    KConfig config("kcminputrc");
    settings->load(&config);

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

    settings->apply();
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
    setHandedness(RIGHT_HANDED);
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
    if (val==RIGHT_HANDED)
        mousePix->setPixmap(KStandardDirs::locate("data", "kcminput/pics/mouse_rh.png"));
    else
        mousePix->setPixmap(KStandardDirs::locate("data", "kcminput/pics/mouse_lh.png"));
    settings->m_handedNeedsApply = true;
}

void MouseSettings::load(KConfig *config, Display *dpy)
{
    // TODO: what's a good threshold default value
    int threshold = 0;
    int h = RIGHT_HANDED;
    double accel = 1.0;
    if (QX11Info::isPlatformX11()) {
        int accel_num, accel_den;

        XGetPointerControl(dpy, &accel_num, &accel_den, &threshold);
        accel = float(accel_num) / float(accel_den);

        // get settings from X server
        unsigned char map[20];
        num_buttons = XGetPointerMapping(dpy, map, 20);

        handedEnabled = true;

        // ## keep this in sync with KGlobalSettings::mouseSettings
        if (num_buttons == 1)
        {
            /* disable button remapping */
            handedEnabled = false;
        }
        else if (num_buttons == 2)
        {
            if ((int)map[0] == 1 && (int)map[1] == 2)
                h = RIGHT_HANDED;
            else if ((int)map[0] == 2 && (int)map[1] == 1)
                h = LEFT_HANDED;
            else
                /* custom button setup: disable button remapping */
                handedEnabled = false;
        }
        else
        {
            middle_button = (int)map[1];
            if ((int)map[0] == 1 && (int)map[2] == 3)
            h = RIGHT_HANDED;
            else if ((int)map[0] == 3 && (int)map[2] == 1)
            h = LEFT_HANDED;
            else
            {
            /* custom button setup: disable button remapping */
            handedEnabled = false;
            }
        }
    } else {
        // other platforms
        handedEnabled = true;
    }

    KConfigGroup group = config->group("Mouse");
    double a = group.readEntry("Acceleration", -1.0);
    if (a == -1)
        accelRate = accel;
    else
        accelRate = a;

    int t = group.readEntry("Threshold", -1);
    if (t == -1)
        thresholdMove = threshold;
    else
        thresholdMove = t;

    QString key = group.readEntry("MouseButtonMapping");
    if (key == "RightHanded")
        handed = RIGHT_HANDED;
    else if (key == "LeftHanded")
        handed = LEFT_HANDED;
#ifdef __GNUC__
#warning was key == NULL how was this working? is key.isNull() what the coder meant?
#endif
    else if (key.isNull())
        handed = h;
    reverseScrollPolarity = group.readEntry("ReverseScrollPolarity", false);
    m_handedNeedsApply = false;

    // SC/DC/AutoSelect/ChangeCursor
    group = config->group("KDE");
    doubleClickInterval = group.readEntry("DoubleClickInterval", 400);
    dragStartTime = group.readEntry("StartDragTime", 500);
    dragStartDist = group.readEntry("StartDragDist", 4);
    wheelScrollLines = group.readEntry("WheelScrollLines", 3);

    singleClick = group.readEntry("SingleClick", KDE_DEFAULT_SINGLECLICK);
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

void MouseSettings::apply(bool force)
{
    if (!QX11Info::isPlatformX11()) {
        return;
    }
    XChangePointerControl(QX11Info::display(),
                          true, true, int(qRound(accelRate*10)), 10, thresholdMove);

    // 256 might seems extreme, but X has already been known to return 32,
    // and we don't want to truncate things. Xlib limits the table to 256 bytes,
    // so it's a good upper bound..
    unsigned char map[256];
    num_buttons = XGetPointerMapping(QX11Info::display(), map, 256);

    int remap=(num_buttons>=1);
    if (handedEnabled && (m_handedNeedsApply || force)) {
        if (num_buttons == 1)
        {
            map[0] = (unsigned char) 1;
        }
        else if (num_buttons == 2)
        {
            if (handed == RIGHT_HANDED)
            {
                map[0] = (unsigned char) 1;
                map[1] = (unsigned char) 3;
            }
            else
            {
                map[0] = (unsigned char) 3;
                map[1] = (unsigned char) 1;
            }
        }
        else // 3 buttons and more
        {
            if (handed == RIGHT_HANDED)
            {
                map[0] = (unsigned char) 1;
                map[1] = (unsigned char) middle_button;
                map[2] = (unsigned char) 3;
            }
            else
            {
                map[0] = (unsigned char) 3;
                map[1] = (unsigned char) middle_button;
                map[2] = (unsigned char) 1;
            }
        }

        int retval;
        if (remap) {
            while ((retval=XSetPointerMapping(QX11Info::display(), map,
                                              num_buttons)) == MappingBusy)
              /* keep trying until the pointer is free */
            { };
        }

        // apply reverseScrollPolarity
        Display *dpy = QX11Info::display();
        Atom prop_wheel_emulation = XInternAtom(dpy, EVDEV_PROP_WHEEL, True);
        Atom prop_scroll_distance = XInternAtom(dpy, EVDEV_PROP_SCROLL_DISTANCE, True);
        Atom prop_wheel_emulation_axes = XInternAtom(dpy, EVDEV_PROP_WHEEL_AXES, True);
        int ndevices_return;
        XIDeviceInfo *info = XIQueryDevice(dpy, XIAllDevices, &ndevices_return);
        if (!info) {
            return;
        }
        for (int i = 0; i < ndevices_return; ++i) {
            if ((info + i)->use == XISlavePointer) {
                int deviceid = (info + i)->deviceid;
                Status status;
                Atom type_return;
                int format_return;
                unsigned long num_items_return;
                unsigned long bytes_after_return;

                unsigned char *data = nullptr;
                unsigned char *data2 = nullptr;
                //data returned is an 1 byte boolean
                status = XIGetProperty(dpy, deviceid, prop_wheel_emulation, 0, 1,
                                       False, XA_INTEGER, &type_return, &format_return,
                                       &num_items_return, &bytes_after_return, &data);
                if (status != Success) {
                    continue;
                }

                // pointer device without wheel emulation
                if (type_return != XA_INTEGER || data == NULL || *data == False) {
                    status = XIGetProperty(dpy, deviceid, prop_scroll_distance, 0, 3,
                                           False, XA_INTEGER, &type_return, &format_return,
                                           &num_items_return, &bytes_after_return, &data2);
                    // negate scroll distance
                    if (status == Success && type_return == XA_INTEGER &&
                          format_return == 32 && num_items_return == 3) {
                        int32_t *vals = (int32_t*)data2;
                        for (unsigned long i=0; i<num_items_return; ++i) {
                            int32_t val = *(vals+i);
                            *(vals+i) = (int32_t) (reverseScrollPolarity ? -abs(val) : abs(val));
                        }
                        XIChangeProperty(dpy, deviceid, prop_scroll_distance, XA_INTEGER,
                                         32, XIPropModeReplace, data2, 3);
                    }
                } else { // wheel emulation used, reverse wheel axes
                    status = XIGetProperty(dpy, deviceid, prop_wheel_emulation_axes, 0, 4,
                                           False, XA_INTEGER, &type_return, &format_return,
                                           &num_items_return, &bytes_after_return, &data2);
                    if (status == Success && type_return == XA_INTEGER &&
                          format_return == 8 && num_items_return == 4) {
                        // when scroll direction is not reversed,
                        // up button id should be smaller than down button id,
                        // up/left are odd elements, down/right are even elements
                        for (int i=0; i<2; ++i) {
                            unsigned char odd = *(data2 + i*2);
                            unsigned char even = *(data2 + i*2+1);
                            unsigned char max_elem = std::max(odd, even);
                            unsigned char min_elem = std::min(odd, even);
                            *(data2 + i * 2) = reverseScrollPolarity ? max_elem : min_elem;
                            *(data2 + i * 2 + 1) = reverseScrollPolarity ? min_elem : max_elem;
                        }
                        XIChangeProperty(dpy, deviceid, prop_wheel_emulation_axes, XA_INTEGER,
                                         8, XIPropModeReplace, data2, 4);
                    }
                }

                if (data != NULL) { XFree(data); }
                if (data2 != NULL) { XFree(data2); }
            }
        }
        XIFreeDeviceInfo(info);
        m_handedNeedsApply = false;
    }
}

void MouseSettings::save(KConfig *config)
{
    KSharedConfig::Ptr kcminputProfile = KSharedConfig::openConfig("kcminputrc");
    KConfigGroup kcminputGroup(kcminputProfile, "Mouse");
    kcminputGroup.writeEntry("Acceleration",accelRate);
    kcminputGroup.writeEntry("Threshold",thresholdMove);
    if (handed == RIGHT_HANDED)
        kcminputGroup.writeEntry("MouseButtonMapping",QString("RightHanded"));
    else
        kcminputGroup.writeEntry("MouseButtonMapping",QString("LeftHanded"));
    kcminputGroup.writeEntry("ReverseScrollPolarity", reverseScrollPolarity);
    kcminputGroup.sync();

    KSharedConfig::Ptr profile = KSharedConfig::openConfig("kdeglobals");
    KConfigGroup group(profile, "KDE");
    group.writeEntry("DoubleClickInterval", doubleClickInterval, KConfig::Persistent);
    group.writeEntry("StartDragTime", dragStartTime, KConfig::Persistent);
    group.writeEntry("StartDragDist", dragStartDist, KConfig::Persistent);
    group.writeEntry("WheelScrollLines", wheelScrollLines, KConfig::Persistent);
    group.writeEntry("SingleClick", singleClick, KConfig::Persistent);

    group.sync();
    config->sync();

    Kdelibs4SharedConfig::syncConfigGroup(QLatin1String("Mouse"), "kcminputrc");
    Kdelibs4SharedConfig::syncConfigGroup(QLatin1String("KDE"), "kdeglobals");

    KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_MOUSE);
}

void MouseConfig::slotScrollPolarityChanged()
{
    settings->m_handedNeedsApply = true;
}

#include "mouse.moc"
