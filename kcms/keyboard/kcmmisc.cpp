/*
 * kcmmisc.cpp
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Layout management, cleanups:
 * Copyright (c) 1999 Dirk A. Mueller <dmuell@gmx.net>
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

#include "kcmmisc.h"
#include "ui_kcmmiscwidget.h"

#include <math.h>

#include <QCheckBox>
#include <QWhatsThis>
#include <QX11Info>

#include <klocale.h>
#include <kconfig.h>
#include <KConfigGroup>
#include <knuminput.h>

#include <X11/Xlib.h>


KCMiscKeyboardWidget::KCMiscKeyboardWidget(QWidget *parent)
	: QWidget(parent),
	  ui(*new Ui_KeyboardConfigWidget)
{
  ui.setupUi(this);

  ui.delay->setRange(100, 5000, 50);
  ui.delay->setSliderEnabled(false);
  ui.rate->setRange(0.2, 50, 5, false);

  sliderMax = (int)floor (0.5 + 2*(log(5000.0L)-log(100.0L)) / (log(5000.0L)-log(4999.0L)));
  ui.delaySlider->setRange(0, sliderMax);
  ui.delaySlider->setSingleStep(sliderMax/100);
  ui.delaySlider->setPageStep(sliderMax/10);
  ui.delaySlider->setTickInterval(sliderMax/10);

  ui.rateSlider->setRange(20, 5000);
  ui.rateSlider->setSingleStep(30);
  ui.rateSlider->setPageStep(500);
  ui.rateSlider->setTickInterval(498);

  connect(ui.keyboardRepeatButtonGroup, SIGNAL(clicked()), this, SLOT(changed()));
  connect(ui.keyboardRepeatButtonGroup, SIGNAL(changed(int)), this, SLOT(keyboardRepeatStateChanged(int)));
  connect(ui.delay, SIGNAL(valueChanged(int)), this, SLOT(delaySpinboxChanged(int)));
  connect(ui.delaySlider, SIGNAL(valueChanged(int)), this, SLOT(delaySliderChanged(int)));
  connect(ui.rate, SIGNAL(valueChanged(double)), this, SLOT(rateSpinboxChanged(double)));
  connect(ui.rateSlider, SIGNAL(valueChanged(int)), this, SLOT(rateSliderChanged(int)));

  connect(ui.click, SIGNAL(valueChanged(int)), this, SLOT(changed()));
  connect(ui.numlockButtonGroup, SIGNAL(released(int)), this, SLOT(changed()));

// Not sure why we need this - if XKB is not found the whole keyboard module won't be compiled
//#if !defined(HAVE_XTEST) && !defined(HAVE_XKB)
//  ui.numlockButtonGroup->setDisabled( true );
//#endif
//#if !defined(HAVE_XKB) && !defined(HAVE_XF86MISC)
//  ui.delay->setDisabled( true );
//  ui.rate->setDisabled( true );
//#endif
}

KCMiscKeyboardWidget::~KCMiscKeyboardWidget()
{
	delete &ui;
}

int  KCMiscKeyboardWidget::getClick()
{
    return ui.click->value();
}

// set the slider and LCD values
void KCMiscKeyboardWidget::setRepeat(TriState keyboardRepeat, int delay_, double rate_)
{
	TriStateHelper::setTriState( ui.keyboardRepeatButtonGroup, keyboardRepeat );
//    ui.repeatBox->setChecked(r == AutoRepeatModeOn);
    ui.delay->setValue(delay_);
    ui.rate->setValue(rate_);
    delaySpinboxChanged(delay_);
    rateSpinboxChanged(rate_);
}

void KCMiscKeyboardWidget::setClickVolume(int v)
{
    ui.click->setValue(v);
}

TriState TriStateHelper::getTriState(const KButtonGroup* group)
{
    int selected = group->selected();
    return selected < 0 ? STATE_UNCHANGED : getTriState(selected);
}

void TriStateHelper::setTriState(KButtonGroup* group, TriState state)
{
    group->setSelected( getInt(state) );
}

void KCMiscKeyboardWidget::load()
{
  KConfigGroup config(KSharedConfig::openConfig("kcminputrc", KConfig::NoGlobals), "Keyboard");

  ui.delay->blockSignals(true);
  ui.rate->blockSignals(true);
  ui.click->blockSignals(true);

  // need to read as string to support old "true/false" parameter
  QString key = config.readEntry("KeyboardRepeating", TriStateHelper::getString(STATE_ON));
  if( key == "true" || key == TriStateHelper::getString(STATE_ON)) {
	  keyboardRepeat = STATE_ON;
  }
  else if( key == "false" || key == TriStateHelper::getString(STATE_OFF)) {
	  keyboardRepeat = STATE_OFF;
  }
  else {
	  keyboardRepeat = STATE_UNCHANGED;
  }

//  keyboardRepeat = (key ? AutoRepeatModeOn : AutoRepeatModeOff);
  float delay = config.readEntry( "RepeatDelay", 660 );
  float rate = config.readEntry( "RepeatRate", 25. );
  setRepeat(keyboardRepeat, delay, rate);

  XKeyboardState kbd;
  XGetKeyboardControl(QX11Info::display(), &kbd);

  clickVolume = config.readEntry("ClickVolume", kbd.key_click_percent);
  setClickVolume(clickVolume);
  //  setRepeat(kbd.global_auto_repeat, ui.delay->value(), ui.rate->value());

  numlockState = TriStateHelper::getTriState(config.readEntry( "NumLock", TriStateHelper::getInt(STATE_UNCHANGED) ));
  TriStateHelper::setTriState( ui.numlockButtonGroup, numlockState );

  ui.delay->blockSignals(false);
  ui.rate->blockSignals(false);
  ui.click->blockSignals(false);
}

void KCMiscKeyboardWidget::save()
{
  KConfigGroup config(KSharedConfig::openConfig("kcminputrc", KConfig::NoGlobals), "Keyboard");

  clickVolume = getClick();
  keyboardRepeat = TriStateHelper::getTriState(ui.keyboardRepeatButtonGroup);
  numlockState = TriStateHelper::getTriState(ui.numlockButtonGroup);

  config.writeEntry("ClickVolume",clickVolume);
  config.writeEntry("KeyboardRepeating", TriStateHelper::getInt(keyboardRepeat));
  config.writeEntry("RepeatRate", ui.rate->value() );
  config.writeEntry("RepeatDelay", ui.delay->value() );
  config.writeEntry("NumLock", TriStateHelper::getInt(numlockState) );
  config.sync();
}

void KCMiscKeyboardWidget::defaults()
{
    setClickVolume(50);
    setRepeat(STATE_ON, 660, 25);
    TriStateHelper::setTriState( ui.numlockButtonGroup, STATE_UNCHANGED );
    emit changed(true);
}

QString KCMiscKeyboardWidget::quickHelp() const
{
  return QString();

  /* "<h1>Keyboard</h1> This module allows you to choose options"
     " for the way in which your keyboard works. The actual effect of"
     " setting these options depends upon the features provided by your"
     " keyboard hardware and the X server on which KDE is running.<p>"
     " For example, you may find that changing the key click volume"
     " has no effect because this feature is not available on your system." */
}

void KCMiscKeyboardWidget::delaySliderChanged (int value) {
	double alpha  = sliderMax / (log(5000.0L) - log(100.0L));
	double linearValue = exp (value/alpha + log(100.0L));

	ui.delay->setValue((int)floor(0.5 + linearValue));

	emit changed(true);
}

void KCMiscKeyboardWidget::delaySpinboxChanged (int value) {
	double alpha  = sliderMax / (log(5000.0L) - log(100.0L));
	double logVal = alpha * (log((double)value)-log(100.0L));

	ui.delaySlider->setValue ((int)floor (0.5 + logVal));

	emit changed(true);
}

void KCMiscKeyboardWidget::rateSliderChanged (int value) {
	ui.rate->setValue(value/100.0);

	emit changed(true);
}

void KCMiscKeyboardWidget::rateSpinboxChanged (double value) {
	ui.rateSlider->setValue ((int)(value*100));

	emit changed(true);
}

void KCMiscKeyboardWidget::changed()
{
  emit changed(true);
}

void KCMiscKeyboardWidget::keyboardRepeatStateChanged(int selection)
{
	bool enabled = selection == TriStateHelper::getInt(STATE_ON);
	ui.keyboardRepeatParamsGroupBox->setEnabled(enabled);
	changed();
}
