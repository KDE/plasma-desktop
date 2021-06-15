/*
    SPDX-FileCopyrightText: 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
    SPDX-FileCopyrightText: 1999 Dirk A. Mueller <dmuell@gmx.net>
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2000 Bernd Gehrmann
    SPDX-FileCopyrightText: 2000 Rik Hemsley <rik@kde.org>
    SPDX-FileCopyrightText: 2000 Brad Hughes <bhughes@trolltech.com>
    SPDX-FileCopyrightText: 2004 Brad Hards <bradh@frogmouth.net>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "xlib_config.h"

#include "../configcontainer.h"
#include "backends/x11/x11_evdev_backend.h"

#include "../../../migrationlib/kdelibs4config.h"

#include <KLocalizedString>
#include <KToolInvocation>
//#include <KConfig>
#include <KAboutData>
#include <KPluginFactory>

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTabWidget>
#include <QWhatsThis>

#include <KConfig>
#include <KConfigGroup>

XlibConfig::XlibConfig(ConfigContainer *parent, InputBackend *backend)
    : ConfigPlugin(parent)
    , m_backend(dynamic_cast<X11EvdevBackend *>(backend))
{
    setupUi(this);

    handedGroup->setId(rightHanded, static_cast<int>(Handed::Right));
    handedGroup->setId(leftHanded, static_cast<int>(Handed::Left));

    connect(handedGroup, SIGNAL(buttonClicked(int)), m_parent, SLOT(changed()));
    connect(handedGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotHandedChanged(int)));

    connect(cbScrollPolarity, SIGNAL(clicked()), m_parent, SLOT(changed()));
    connect(cbScrollPolarity, SIGNAL(clicked()), this, SLOT(slotScrollPolarityChanged()));

    connect(accel, SIGNAL(valueChanged(double)), m_parent, SLOT(changed()));
    connect(thresh, SIGNAL(valueChanged(int)), m_parent, SLOT(changed()));
    connect(thresh, SIGNAL(valueChanged(int)), this, SLOT(slotThreshChanged(int)));
    slotThreshChanged(thresh->value());

    // It would be nice if the user had a test field.
    // Selecting such values in milliseconds is not intuitive
    connect(doubleClickInterval, SIGNAL(valueChanged(int)), m_parent, SLOT(changed()));

    connect(dragStartTime, SIGNAL(valueChanged(int)), m_parent, SLOT(changed()));

    connect(dragStartDist, SIGNAL(valueChanged(int)), m_parent, SLOT(changed()));
    connect(dragStartDist, SIGNAL(valueChanged(int)), this, SLOT(slotDragStartDistChanged(int)));
    slotDragStartDistChanged(dragStartDist->value());

    connect(wheelScrollLines, SIGNAL(valueChanged(int)), m_parent, SLOT(changed()));
    connect(wheelScrollLines, SIGNAL(valueChanged(int)), SLOT(slotWheelScrollLinesChanged(int)));
    slotWheelScrollLinesChanged(wheelScrollLines->value());

    connect(mouseKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));
    connect(mouseKeys, SIGNAL(clicked()), m_parent, SLOT(changed()));
    connect(mk_delay, SIGNAL(valueChanged(int)), m_parent, SLOT(changed()));
    connect(mk_interval, SIGNAL(valueChanged(int)), m_parent, SLOT(changed()));
    connect(mk_time_to_max, SIGNAL(valueChanged(int)), m_parent, SLOT(changed()));
    connect(mk_max_speed, SIGNAL(valueChanged(int)), m_parent, SLOT(changed()));
    connect(mk_curve, SIGNAL(valueChanged(int)), m_parent, SLOT(changed()));

    KAboutData *about = new KAboutData(QStringLiteral("kcmmouse"),
                                       i18n("Mouse"),
                                       QStringLiteral("1.0"),
                                       QString(),
                                       KAboutLicense::GPL,
                                       i18n("(c) 1997 - 2018 Mouse developers"));
    about->addAuthor(i18n("Patrick Dowler"));
    about->addAuthor(i18n("Dirk A. Mueller"));
    about->addAuthor(i18n("David Faure"));
    about->addAuthor(i18n("Bernd Gehrmann"));
    about->addAuthor(i18n("Rik Hemsley"));
    about->addAuthor(i18n("Brad Hughes"));
    about->addAuthor(i18n("Brad Hards"));
    about->addAuthor(i18n("Roman Gilg"));
    m_parent->setAboutData(about);
}

void XlibConfig::checkAccess()
{
    mk_delay->setEnabled(mouseKeys->isChecked());
    mk_interval->setEnabled(mouseKeys->isChecked());
    mk_time_to_max->setEnabled(mouseKeys->isChecked());
    mk_max_speed->setEnabled(mouseKeys->isChecked());
    mk_curve->setEnabled(mouseKeys->isChecked());
}

double XlibConfig::getAccel()
{
    return accel->value();
}

void XlibConfig::setAccel(double val)
{
    accel->setValue(val);
}

int XlibConfig::getThreshold()
{
    return thresh->value();
}

void XlibConfig::setThreshold(int val)
{
    thresh->setValue(val);
}

Handed XlibConfig::getHandedness()
{
    if (rightHanded->isChecked())
        return Handed::Right;
    else
        return Handed::Left;
}

void XlibConfig::setHandedness(Handed val)
{
    rightHanded->setChecked(false);
    leftHanded->setChecked(false);
    if (val == Handed::Right) {
        rightHanded->setChecked(true);
        mousePix->setPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kcmmouse/pics/mouse_rh.png"));
    } else {
        leftHanded->setChecked(true);
        mousePix->setPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kcmmouse/pics/mouse_lh.png"));
    }
    m_backend->settings()->handedNeedsApply = true;
}

void XlibConfig::load()
{
    EvdevSettings *settings = m_backend->settings();

    m_parent->kcmLoad();
    m_backend->load();

    // Only allow setting reversing scroll polarity if we have scroll buttons
    if (m_backend) {
        if (m_backend->supportScrollPolarity()) {
            cbScrollPolarity->setEnabled(true);
            cbScrollPolarity->show();
        } else {
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

    KConfig ac("kaccessrc");

    KConfigGroup group = ac.group("Mouse");
    mouseKeys->setChecked(group.readEntry("MouseKeys", false));
    mk_delay->setValue(group.readEntry("MKDelay", 160));

    int interval = group.readEntry("MKInterval", 5);
    mk_interval->setValue(interval);

    // Default time to reach maximum speed: 5000 msec
    int time_to_max = group.readEntry("MKTimeToMax", (5000 + interval / 2) / interval);
    time_to_max = group.readEntry("MK-TimeToMax", time_to_max * interval);
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
    Q_EMIT m_parent->changed(false);
}

void XlibConfig::save()
{
    EvdevSettings *settings = m_backend->settings();

    settings->accelRate = getAccel();
    settings->thresholdMove = getThreshold();
    settings->handed = getHandedness();

    settings->doubleClickInterval = doubleClickInterval->value();
    settings->dragStartTime = dragStartTime->value();
    settings->dragStartDist = dragStartDist->value();
    settings->wheelScrollLines = wheelScrollLines->value();
    settings->reverseScrollPolarity = cbScrollPolarity->isChecked();

    m_backend->apply();
    settings->save();

    KConfig ac("kaccessrc");

    KConfigGroup group = ac.group("Mouse");

    int interval = mk_interval->value();
    group.writeEntry("MouseKeys", mouseKeys->isChecked());
    group.writeEntry("MKDelay", mk_delay->value());
    group.writeEntry("MKInterval", interval);
    group.writeEntry("MK-TimeToMax", mk_time_to_max->value());
    group.writeEntry("MKTimeToMax", (mk_time_to_max->value() + interval / 2) / interval);
    group.writeEntry("MK-MaxSpeed", mk_max_speed->value());
    group.writeEntry("MKMaxSpeed", (mk_max_speed->value() * interval + 500) / 1000);
    group.writeEntry("MKCurve", mk_curve->value());
    group.sync();

    // restart kaccess
    KToolInvocation::startServiceByDesktopName("kaccess");

    Q_EMIT m_parent->changed(false);
}

void XlibConfig::defaults()
{
    setThreshold(2);
    setAccel(2);
    setHandedness(Handed::Right);
    cbScrollPolarity->setChecked(false);
    doubleClickInterval->setValue(400);
    dragStartTime->setValue(500);
    dragStartDist->setValue(4);
    wheelScrollLines->setValue(3);

    mouseKeys->setChecked(false);
    mk_delay->setValue(160);
    mk_interval->setValue(5);
    mk_time_to_max->setValue(5000);
    mk_max_speed->setValue(1000);
    mk_curve->setValue(0);

    checkAccess();
    m_parent->kcmDefaults();

    Q_EMIT m_parent->changed(true);
}

/** No descriptions */
void XlibConfig::slotHandedChanged(int val)
{
    if (val == static_cast<int>(Handed::Right))
        mousePix->setPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kcmmouse/pics/mouse_rh.png"));
    else
        mousePix->setPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kcmmouse/pics/mouse_lh.png"));
    m_backend->settings()->handedNeedsApply = true;
}

void XlibConfig::slotThreshChanged(int value)
{
    thresh->setSuffix(i18np(" pixel", " pixels", value));
}

void XlibConfig::slotDragStartDistChanged(int value)
{
    dragStartDist->setSuffix(i18np(" pixel", " pixels", value));
}

void XlibConfig::slotWheelScrollLinesChanged(int value)
{
    wheelScrollLines->setSuffix(i18np(" line", " lines", value));
}

void XlibConfig::slotScrollPolarityChanged()
{
    m_backend->settings()->handedNeedsApply = true;
}
