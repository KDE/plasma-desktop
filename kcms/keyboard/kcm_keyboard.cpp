/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
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


#include "kcm_keyboard.h"

#include <KAboutData>
#include <KPluginFactory>
//#include <KPluginLoader>
#include <KLocalizedString>

#include <QDBusMessage>
#include <QDBusConnection>

#include "kcm_keyboard_widget.h"
#include "x11_helper.h"
#include "keyboard_config.h"
#include "xkb_rules.h"
#include "keyboard_dbus.h"

#include "xkb_helper.h"

//temp hack
#include "kcmmisc.h"


K_PLUGIN_FACTORY(KeyboardModuleFactory, registerPlugin<KCMKeyboard>();)

KCMKeyboard::KCMKeyboard(QWidget *parent, const QVariantList &args)
  : KCModule(parent)
{
  KAboutData *about =
          new KAboutData(QStringLiteral("kcmkeyboard"), i18n("KDE Keyboard Control Module"),
                  QStringLiteral("1.0"), QString(), KAboutLicense::GPL,
                  i18n("(c) 2010 Andriy Rysin"));

  setAboutData( about );
  setQuickHelp( i18n("<h1>Keyboard</h1> This control module can be used to configure keyboard"
    " parameters and layouts."));


  rules = Rules::readRules(Rules::READ_EXTRAS);

  keyboardConfig = new KeyboardConfig();

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
//  layout->setSpacing(KDialog::spacingHint());

  widget = new KCMKeyboardWidget(rules, keyboardConfig, args, parent);
  layout->addWidget(widget);

  connect(widget, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));

  setButtons(Help|Default|Apply);
}

KCMKeyboard::~KCMKeyboard()
{
	delete keyboardConfig;
	delete rules;
}

void KCMKeyboard::defaults()
{
	keyboardConfig->setDefaults();
	widget->updateUI();
	widget->getKcmMiscWidget()->defaults();
	emit changed(true);
}

void KCMKeyboard::load()
{
	keyboardConfig->load();
	widget->updateUI();
	widget->getKcmMiscWidget()->load();
}

//static void initializeKeyboardSettings();
void KCMKeyboard::save()
{
	keyboardConfig->save();
	widget->save();
	widget->getKcmMiscWidget()->save();

	QDBusMessage message = QDBusMessage::createSignal(KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE);
    QDBusConnection::sessionBus().send(message);
}

#include "kcm_keyboard.moc"
