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
#include "kcmmisc.h"
#include "keyboard_config.h"
#include "keyboard_dbus.h"
#include "keyboardmiscsettings.h"
#include "keyboardsettingsdata.h"
#include "x11_helper.h"
#include "xkb_rules.h"

#include "xkb_helper.h"

KCMKeyboard::KCMKeyboard(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
    , m_data(new KeyboardSettingsData(this))
{
    rules = Rules::readRules(Rules::READ_EXTRAS);

    QVBoxLayout *layout = new QVBoxLayout(widget());
    layout->setContentsMargins(0, 0, 0, 0);
    //  layout->setSpacing(KDialog::spacingHint());

    m_miscWidget = new KCMiscKeyboardWidget(widget(), m_data->miscSettings());
    m_widget = new KCMKeyboardWidget(rules, m_data->keyboardConfig(), m_workspaceOptions, m_miscWidget, args, widget());
    layout->addWidget(m_widget);

    connect(m_widget, &KCMKeyboardWidget::changed, this, &KCMKeyboard::updateUnmanagedState);
    connect(m_miscWidget, qOverload<bool>(&KCMiscKeyboardWidget::changed), this, &KCMKeyboard::updateUnmanagedState);
    connect(this, &KCMKeyboard::defaultsIndicatorsVisibleChanged, this, &KCMKeyboard::updateUnmanagedState);
    connect(this, &KCMKeyboard::defaultsIndicatorsVisibleChanged, m_widget, [this]() {
        m_widget->setDefaultIndicator(defaultsIndicatorsVisible());
    });
    connect(this, &KCMKeyboard::defaultsIndicatorsVisibleChanged, m_miscWidget, [this]() {
        m_miscWidget->setDefaultIndicator(defaultsIndicatorsVisible());
    });
    setButtons(Help | Default | Apply);

    addConfig(m_data->keyboardConfig(), m_widget);
    addConfig(m_data->miscSettings(), m_miscWidget);
    addConfig(&m_workspaceOptions, m_widget);
}

KCMKeyboard::~KCMKeyboard()
{
    delete rules;
}

void KCMKeyboard::defaults()
{
    KCModule::defaults();

    m_widget->defaults();
    m_miscWidget->defaults();
}

void KCMKeyboard::updateUnmanagedState()
{
    bool isNeedSave = false;
    isNeedSave |= m_widget->isSaveNeeded();
    isNeedSave |= m_miscWidget->isSaveNeeded();
    unmanagedWidgetChangeState(isNeedSave);

    bool isDefault = true;
    isDefault &= m_widget->isDefault();
    isDefault &= m_miscWidget->isDefault();
    unmanagedWidgetDefaultState(isDefault);
}

void KCMKeyboard::load()
{
    KCModule::load();
    m_data->keyboardConfig()->load();
    m_widget->updateUI();
    m_miscWidget->load();
}

void KCMKeyboard::save()
{
    m_widget->save();
    m_miscWidget->save();

    m_data->keyboardConfig()->save();
    m_data->miscSettings()->save();
    KCModule::save();

    QDBusMessage message = QDBusMessage::createSignal(KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE);
    QDBusConnection::sessionBus().send(message);
}

#include "kcm_keyboard.moc"
