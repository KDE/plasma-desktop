/*
    kcmmisc.cpp

    SPDX-FileCopyrightText: 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>

    Layout management, cleanups:
    SPDX-FileCopyrightText: 1999 Dirk A. Mueller <dmuell@gmx.net>

    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcmmisc.h"
#include "ui_kcmmiscwidget.h"
#include "keyboardmiscsettings.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QWhatsThis>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QX11Info>
#else
#include <QtGui/private/qtx11extras_p.h>
#endif

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

KCMiscKeyboardWidget::KCMiscKeyboardWidget(QWidget *parent, KeyboardMiscSettings *settings)
    : QWidget(parent)
    , ui(*new Ui_KeyboardConfigWidget)
    , m_settings(settings)
{
    ui.setupUi(this);

    ui.kcfg_repeatDelay->setSingleStep(50);
    ui.kcfg_repeatRate->setSingleStep(5);

    sliderMax = (int)floor(0.5 + 2 * (log(5000.0L) - log(100.0L)) / (log(5000.0L) - log(4999.0L)));
    ui.delaySlider->setRange(0, sliderMax);
    ui.delaySlider->setSingleStep(sliderMax / 100);
    ui.delaySlider->setPageStep(sliderMax / 10);
    ui.delaySlider->setTickInterval(sliderMax / 10);

    ui.rateSlider->setRange(20, 10000);
    ui.rateSlider->setSingleStep(30);
    ui.rateSlider->setPageStep(500);
    ui.rateSlider->setTickInterval(498);

    connect(ui.kcfg_repeatDelay, SIGNAL(valueChanged(int)), this, SLOT(delaySpinboxChanged(int)));
    connect(ui.delaySlider, &QAbstractSlider::valueChanged, this, &KCMiscKeyboardWidget::delaySliderChanged);
    connect(ui.kcfg_repeatRate, SIGNAL(valueChanged(double)), this, SLOT(rateSpinboxChanged(double)));
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

    connect(_numlockButtonGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked),
            this, &KCMiscKeyboardWidget::updateUiDefaultIndicator);
    connect(_keyboardRepeatButtonGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked),
            this, &KCMiscKeyboardWidget::updateUiDefaultIndicator);
}

KCMiscKeyboardWidget::~KCMiscKeyboardWidget()
{
    delete &ui;
}

// set the slider and LCD values
void KCMiscKeyboardWidget::setRepeat(KeyBehaviour keyboardRepeat, int delay_, double rate_)
{
    _keyboardRepeatButtonGroup->button(keyboardRepeat)->click();

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
    // need to read as string to support old "true/false" parameter
    QString key = m_settings->keyboardRepeat();
    if (key == QLatin1String("true") || key == TriStateHelper::getString(STATE_ON) || key == QLatin1String("accent")) {
        keyboardRepeat = KeyBehaviour::AccentMenu;
    } else if (key == QLatin1String("false") || key == TriStateHelper::getString(STATE_OFF) || key == QLatin1String("nothing")) {
        keyboardRepeat = KeyBehaviour::DoNothing;
    } else if (key == QLatin1String("repeat")) {
        keyboardRepeat = KeyBehaviour::RepeatKey;
    }
    setRepeat(keyboardRepeat, m_settings->repeatDelay(), m_settings->repeatRate());

    numlockState = TriStateHelper::getTriState(m_settings->numLock());
    TriStateHelper::setTriState(_numlockButtonGroup, numlockState);
}

void KCMiscKeyboardWidget::save()
{
    numlockState = TriStateHelper::getTriState(_numlockButtonGroup);
    keyboardRepeat = KeyBehaviour(_keyboardRepeatButtonGroup->checkedId());

    m_settings->setKeyboardRepeat(keybehaviourNames[keyboardRepeat]);
    m_settings->setNumLock(TriStateHelper::getInt(numlockState));
}

void KCMiscKeyboardWidget::defaults()
{
    setRepeat(defaultValueKeyboardRepeat(), m_settings->defaultRepeatDelayValue(), m_settings->defaultRepeatRateValue());
    TriStateHelper::setTriState(_numlockButtonGroup, static_cast<TriState>(m_settings->defaultNumLockValue()));
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

bool KCMiscKeyboardWidget::isSaveNeeded() const
{
    return m_settings->keyboardRepeat() != keybehaviourNames[KeyBehaviour(_keyboardRepeatButtonGroup->checkedId())]
            || m_settings->numLock() != TriStateHelper::getInt(TriStateHelper::getTriState(_numlockButtonGroup));
}

bool KCMiscKeyboardWidget::isDefault() const
{
    return defaultValueKeyboardRepeat() == KeyBehaviour(_keyboardRepeatButtonGroup->checkedId())
            && m_settings->defaultNumLockValue() == TriStateHelper::getInt(TriStateHelper::getTriState(_numlockButtonGroup));
}

void KCMiscKeyboardWidget::setDefaultIndicator(bool visible)
{
    m_highlightVisible = visible;
    updateUiDefaultIndicator();
}

void KCMiscKeyboardWidget::updateUiDefaultIndicator()
{
    const auto isNumLockDefault = m_settings->defaultNumLockValue() == TriStateHelper::getInt(TriStateHelper::getTriState(_numlockButtonGroup));
    for (auto button : _numlockButtonGroup->buttons()) {
        setDefaultIndicatorVisible(button, m_highlightVisible && !isNumLockDefault && _numlockButtonGroup->checkedButton() == button);
    }

    const auto isKeyboardRepeatDefault = defaultValueKeyboardRepeat() == KeyBehaviour(_keyboardRepeatButtonGroup->checkedId());
    for (auto button : _keyboardRepeatButtonGroup->buttons()) {
        setDefaultIndicatorVisible(button, m_highlightVisible && !isKeyboardRepeatDefault && _keyboardRepeatButtonGroup->checkedButton() == button);
    }

    setDefaultIndicatorVisible(ui.delaySlider, m_highlightVisible && ui.kcfg_repeatDelay->value() != m_settings->defaultRepeatDelayValue());
    setDefaultIndicatorVisible(ui.rateSlider, m_highlightVisible && ui.kcfg_repeatRate->value() != m_settings->defaultRepeatRateValue());
}

void KCMiscKeyboardWidget::delaySliderChanged(int value)
{
    double alpha = sliderMax / (log(5000.0L) - log(100.0L));
    double linearValue = exp(value / alpha + log(100.0L));

    ui.kcfg_repeatDelay->setValue((int)floor(0.5 + linearValue));
    updateUiDefaultIndicator();

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
    ui.kcfg_repeatRate->setValue(value / 100.0);
    updateUiDefaultIndicator();

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

void KCMiscKeyboardWidget::setDefaultIndicatorVisible(QWidget *widget, bool visible)
{
    widget->setProperty("_kde_highlight_neutral", visible);
    widget->update();
}

KeyBehaviour KCMiscKeyboardWidget::defaultValueKeyboardRepeat() const
{
    if (m_settings->defaultKeyboardRepeatValue() == keybehaviourNames[KeyBehaviour::AccentMenu] && !hasAccentSupport()) {
        return KeyBehaviour::RepeatKey;
    }

    const auto keys = keybehaviourNames.keys();
    auto defaultRepeat = std::find_if(keys.constBegin(), keys.constEnd(), [=](const KeyBehaviour &key) {
        return keybehaviourNames[key] == m_settings->defaultKeyboardRepeatValue();
    });

    return *defaultRepeat;
}
