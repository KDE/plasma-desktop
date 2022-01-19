/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_keyboard.h"

#include <KAboutData>
#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusMessage>

#include "kcm_keyboard_widget.h"
#include "keyboard_config.h"
#include "keyboardmiscsettings.h"
#include "kcmmisc.h"
#include "keyboard_dbus.h"
#include "x11_helper.h"
#include "xkb_rules.h"
#include "keyboardsettingsdata.h"

#include "xkb_helper.h"

KCMKeyboard::KCMKeyboard(QWidget *parent, const QVariantList &args)
    : KCModule(parent)
    , m_data(new KeyboardSettingsData(this))
{
    KAboutData *about = new KAboutData(QStringLiteral("kcmkeyboard"),
                                       i18n("KDE Keyboard Control Module"),
                                       QStringLiteral("1.0"),
                                       QString(),
                                       KAboutLicense::GPL,
                                       i18n("(c) 2010 Andriy Rysin"));

    setAboutData(about);
    setQuickHelp(
        i18n("<h1>Keyboard</h1> This control module can be used to configure keyboard"
             " parameters and layouts."));

    rules = Rules::readRules(Rules::READ_EXTRAS);


    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    //  layout->setSpacing(KDialog::spacingHint());

    m_miscWidget = new KCMiscKeyboardWidget(parent, m_data->miscSettings());
    widget = new KCMKeyboardWidget(rules, m_data->keyboardConfig(), m_workspaceOptions, m_miscWidget, args, parent);
    layout->addWidget(widget);

    connect(widget, &KCMKeyboardWidget::changed, this, &KCMKeyboard::updateUnmanagedState);
    connect(m_miscWidget, qOverload<bool>(&KCMiscKeyboardWidget::changed), this, &KCMKeyboard::updateUnmanagedState);
    connect(this, &KCMKeyboard::defaultsIndicatorsVisibleChanged, this, &KCMKeyboard::updateUnmanagedState);
    connect(this, &KCMKeyboard::defaultsIndicatorsVisibleChanged, widget, &KCMKeyboardWidget::setDefaultIndicator);
    connect(this, &KCMKeyboard::defaultsIndicatorsVisibleChanged, m_miscWidget, &KCMiscKeyboardWidget::setDefaultIndicator);
    setButtons(Help | Default | Apply);

    addConfig(m_data->keyboardConfig(), widget);
    addConfig(m_data->miscSettings(), m_miscWidget);
    addConfig(&m_workspaceOptions, widget);
}

KCMKeyboard::~KCMKeyboard()
{
    delete rules;
}

void KCMKeyboard::defaults()
{
    KCModule::defaults();

    widget->defaults();
    m_miscWidget->defaults();
}

void KCMKeyboard::updateUnmanagedState()
{
    bool isNeedSave = false;
    isNeedSave |= widget->isSaveNeeded();
    isNeedSave |= m_miscWidget->isSaveNeeded();
    unmanagedWidgetChangeState(isNeedSave);

    bool isDefault = true;
    isDefault &= widget->isDefault();
    isDefault &= m_miscWidget->isDefault();
    unmanagedWidgetDefaultState(isDefault);
}

void KCMKeyboard::load()
{
    KCModule::load();
    m_data->keyboardConfig()->load();
    widget->updateUI();
    m_miscWidget->load();
}

void KCMKeyboard::save()
{
    widget->save();
    m_miscWidget->save();

    m_data->keyboardConfig()->save();
    m_data->miscSettings()->save();
    KCModule::save();

    QDBusMessage message = QDBusMessage::createSignal(KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE);
    QDBusConnection::sessionBus().send(message);
}

#include "kcm_keyboard.moc"
