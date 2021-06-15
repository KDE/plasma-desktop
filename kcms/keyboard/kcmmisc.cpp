/*
    kcmmisc.cpp

    SPDX-FileCopyrightText: 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>

    Layout management, cleanups:
    SPDX-FileCopyrightText: 1999 Dirk A. Mueller <dmuell@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcmmisc.h"
#include "ui_kcmmiscwidget.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QWhatsThis>
#include <QX11Info>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <X11/Xlib.h>
#include <cmath>

namespace
{
bool hasAccentSupport()
{
    static bool isPlasmaIM = (qgetenv("QT_IM_MODULE") == "plasmaim");
    return isPlasmaIM;
}
}

KCMiscKeyboardWidget::KCMiscKeyboardWidget(QWidget *parent)
    : QWidget(parent)
    , ui(*new Ui_KeyboardConfigWidget)
{
    ui.setupUi(this);

    ui.delay->setRange(100, 5000);
    ui.delay->setSingleStep(50);
    ui.rate->setRange(0.2, 100);
    ui.rate->setSingleStep(5);

    sliderMax = (int)floor(0.5 + 2 * (log(5000.0L) - log(100.0L)) / (log(5000.0L) - log(4999.0L)));
    ui.delaySlider->setRange(0, sliderMax);
    ui.delaySlider->setSingleStep(sliderMax / 100);
    ui.delaySlider->setPageStep(sliderMax / 10);
    ui.delaySlider->setTickInterval(sliderMax / 10);

    ui.rateSlider->setRange(20, 10000);
    ui.rateSlider->setSingleStep(30);
    ui.rateSlider->setPageStep(500);
    ui.rateSlider->setTickInterval(498);

    connect(ui.delay, SIGNAL(valueChanged(int)), this, SLOT(delaySpinboxChanged(int)));
    connect(ui.delaySlider, &QAbstractSlider::valueChanged, this, &KCMiscKeyboardWidget::delaySliderChanged);
    connect(ui.rate, SIGNAL(valueChanged(double)), this, SLOT(rateSpinboxChanged(double)));
    connect(ui.rateSlider, &QAbstractSlider::valueChanged, this, &KCMiscKeyboardWidget::rateSliderChanged);

    _numlockButtonGroup = new QButtonGroup(ui.numlockButtonGroup);
    _numlockButtonGroup->addButton(ui.radioButton1, 0);
    _numlockButtonGroup->addButton(ui.radioButton2, 1);
    _numlockButtonGroup->addButton(ui.radioButton3, 2);

    connect(_numlockButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(changed()));

    _keyboardRepeatButtonGroup = new QButtonGroup(ui.repeatFormLayout);
    if (hasAccentSupport()) {
        _keyboardRepeatButtonGroup->addButton(ui.accentMenuRadioButton, 0);
    } else {
        ui.accentMenuRadioButton->setVisible(false);
    }
    _keyboardRepeatButtonGroup->addButton(ui.repeatRadioButton, 1);
    _keyboardRepeatButtonGroup->addButton(ui.nothingRadioButton, 2);

    connect(_keyboardRepeatButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(changed()));
    connect(_keyboardRepeatButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(keyboardRepeatStateChanged(int)));
}

KCMiscKeyboardWidget::~KCMiscKeyboardWidget()
{
    delete &ui;
}

// set the slider and LCD values
void KCMiscKeyboardWidget::setRepeat(KeyBehaviour keyboardRepeat, int delay_, double rate_)
{
    if (keyboardRepeat == KeyBehaviour::AccentMenu && !hasAccentSupport()) {
        keyboardRepeat = KeyBehaviour::RepeatKey;
    }
    _keyboardRepeatButtonGroup->button(keyboardRepeat)->click();
    ui.delay->setValue(delay_);
    ui.rate->setValue(rate_);
    delaySpinboxChanged(delay_);
    rateSpinboxChanged(rate_);
}

TriState TriStateHelper::getTriState(const QButtonGroup *group)
{
    int selected = group->checkedId();
    return selected < 0 ? STATE_UNCHANGED : getTriState(selected);
}

void TriStateHelper::setTriState(QButtonGroup *group, TriState state)
{
    group->button(getInt(state))->click();
}

void KCMiscKeyboardWidget::load()
{
    KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("kcminputrc"), KConfig::NoGlobals), "Keyboard");

    ui.delay->blockSignals(true);
    ui.rate->blockSignals(true);

    // need to read as string to support old "true/false" parameter
    QString key = config.readEntry("KeyRepeat", TriStateHelper::getString(STATE_ON));
    if (key == QLatin1String("true") || key == TriStateHelper::getString(STATE_ON) || key == QLatin1String("accent")) {
        keyboardRepeat = KeyBehaviour::AccentMenu;
    } else if (key == QLatin1String("false") || key == TriStateHelper::getString(STATE_OFF) || key == QLatin1String("nothing")) {
        keyboardRepeat = KeyBehaviour::DoNothing;
    } else if (key == QLatin1String("repeat")) {
        keyboardRepeat = KeyBehaviour::RepeatKey;
    }

    //  keyboardRepeat = (key ? AutoRepeatModeOn : AutoRepeatModeOff);
    int delay = config.readEntry("RepeatDelay", DEFAULT_REPEAT_DELAY);
    double rate = config.readEntry("RepeatRate", DEFAULT_REPEAT_RATE);
    setRepeat(keyboardRepeat, delay, rate);

    //  setRepeat(kbd.global_auto_repeat, ui.delay->value(), ui.rate->value());

    numlockState = TriStateHelper::getTriState(config.readEntry("NumLock", TriStateHelper::getInt(STATE_UNCHANGED)));
    TriStateHelper::setTriState(_numlockButtonGroup, numlockState);

    ui.delay->blockSignals(false);
    ui.rate->blockSignals(false);
}

void KCMiscKeyboardWidget::save()
{
    KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("kcminputrc"), KConfig::NoGlobals), "Keyboard");

    keyboardRepeat = KeyBehaviour(_keyboardRepeatButtonGroup->checkedId());
    numlockState = TriStateHelper::getTriState(_numlockButtonGroup);

    config.writeEntry("KeyRepeat", keybehaviourNames[KeyBehaviour(_keyboardRepeatButtonGroup->checkedId())], KConfig::Notify);
    config.writeEntry("RepeatRate", ui.rate->value(), KConfig::Notify);
    config.writeEntry("RepeatDelay", ui.delay->value(), KConfig::Notify);
    config.writeEntry("NumLock", TriStateHelper::getInt(numlockState));
    config.sync();
}

void KCMiscKeyboardWidget::defaults()
{
    setRepeat(KeyBehaviour::AccentMenu, DEFAULT_REPEAT_DELAY, DEFAULT_REPEAT_RATE);
    TriStateHelper::setTriState(_numlockButtonGroup, STATE_UNCHANGED);
    Q_EMIT changed(true);
}

QString KCMiscKeyboardWidget::quickHelp() const
{
    return QString();

    /* "<h1>Keyboard</h1> This module allows you to choose options"
       " for the way in which your keyboard works. The actual effect of"
       " setting these options depends upon the features provided by your"
       " keyboard hardware and the X server on which Plasma is running.<p>"
       " For example, you may find that changing the key click volume"
       " has no effect because this feature is not available on your system." */
}

void KCMiscKeyboardWidget::delaySliderChanged(int value)
{
    double alpha = sliderMax / (log(5000.0L) - log(100.0L));
    double linearValue = exp(value / alpha + log(100.0L));

    ui.delay->setValue((int)floor(0.5 + linearValue));

    Q_EMIT changed(true);
}

void KCMiscKeyboardWidget::delaySpinboxChanged(int value)
{
    double alpha = sliderMax / (log(5000.0L) - log(100.0L));
    double logVal = alpha * (log((double)value) - log(100.0L));

    ui.delaySlider->setValue((int)floor(0.5 + logVal));

    Q_EMIT changed(true);
}

void KCMiscKeyboardWidget::rateSliderChanged(int value)
{
    ui.rate->setValue(value / 100.0);

    Q_EMIT changed(true);
}

void KCMiscKeyboardWidget::rateSpinboxChanged(double value)
{
    ui.rateSlider->setValue((int)(value * 100));

    Q_EMIT changed(true);
}

void KCMiscKeyboardWidget::changed()
{
    Q_EMIT changed(true);
}

void KCMiscKeyboardWidget::keyboardRepeatStateChanged(int selection)
{
    ui.keyboardRepeatParamsGroupBox->setVisible(selection == KeyBehaviour::RepeatKey);
    changed();
}
