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

#include "keyboard_daemon.h"

#include <QX11Info>
#include <QtDBus/QtDBus>
#include <QProcess>

#include <kpluginfactory.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kglobalsettings.h>

#include "x11_helper.h"
#include "xinput_helper.h"
#include "xkb_helper.h"
#include "keyboard_dbus.h"
#include "xkb_rules.h"
#include "bindings.h"
#include "keyboard_hardware.h"
#include "layout_tray_icon.h"
#include "layout_memory_persister.h"
#include "layouts_menu.h"


K_PLUGIN_FACTORY(KeyboardFactory, registerPlugin<KeyboardDaemon>();)

KeyboardDaemon::KeyboardDaemon(QObject *parent, const QList<QVariant>&)
	: KDEDModule(parent),
	  actionCollection(NULL),
	  xEventNotifier(NULL),
	  layoutTrayIcon(NULL),
	  layoutMemory(keyboardConfig),
	  rules(Rules::readRules(Rules::READ_EXTRAS))
{
	if( ! X11Helper::xkbSupported(NULL) )
		return;		//TODO: shut down the daemon?

    QDBusConnection dbus = QDBusConnection::sessionBus();
	dbus.registerService(KEYBOARD_DBUS_SERVICE_NAME);
	dbus.registerObject(KEYBOARD_DBUS_OBJECT_PATH, this, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);
    dbus.connect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT(configureKeyboard()));

	configureKeyboard();
	registerListeners();

	LayoutMemoryPersister layoutMemoryPersister(layoutMemory);
	if( layoutMemoryPersister.restore() ) {
		if( layoutMemoryPersister.getGlobalLayout().isValid() ) {
			X11Helper::setLayout(layoutMemoryPersister.getGlobalLayout());
		}
	}
}

KeyboardDaemon::~KeyboardDaemon()
{
	LayoutMemoryPersister layoutMemoryPersister(layoutMemory);
	layoutMemoryPersister.setGlobalLayout(X11Helper::getCurrentLayout());
	layoutMemoryPersister.save();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.disconnect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT(configureKeyboard()));
	dbus.unregisterObject(KEYBOARD_DBUS_OBJECT_PATH);
	dbus.unregisterService(KEYBOARD_DBUS_SERVICE_NAME);

	unregisterListeners();
	unregisterShortcut();

	delete xEventNotifier;
	delete layoutTrayIcon;
	delete rules;
}

void KeyboardDaemon::configureKeyboard()
{
	qCDebug(KCM_KEYBOARD) << "Configuring keyboard";
	init_keyboard_hardware();

	keyboardConfig.load();
	if( keyboardConfig.configureLayouts ) {
        XkbHelper::preInitialize();
	}
	XkbHelper::initializeKeyboardLayouts(keyboardConfig);
	layoutMemory.configChanged();

	setupTrayIcon();

	unregisterShortcut();
	registerShortcut();
}

void KeyboardDaemon::configureMouse()
{
    QStringList modules;
    modules << "mouse";
    QProcess::startDetached("kcminit", modules);
}

void KeyboardDaemon::setupTrayIcon()
{
	bool show = keyboardConfig.showIndicator
			&& ( keyboardConfig.showSingle || X11Helper::getLayoutsList().size() > 1 );

	if( show && ! layoutTrayIcon ) {
		layoutTrayIcon = new LayoutTrayIcon(rules, keyboardConfig);
	}
	else if( ! show && layoutTrayIcon ) {
		delete layoutTrayIcon;
		layoutTrayIcon = NULL;
	}
}

void KeyboardDaemon::registerShortcut()
{
	if( actionCollection == NULL ) {
		actionCollection = new KeyboardLayoutActionCollection(this, false);

		QAction* toggleLayoutAction = actionCollection->getToggeAction();
		connect(toggleLayoutAction, SIGNAL(triggered()), this, SLOT(switchToNextLayout()));
		actionCollection->loadLayoutShortcuts(keyboardConfig.layouts, rules);
		connect(actionCollection, SIGNAL(actionTriggered(QAction*)), this, SLOT(setLayout(QAction*)));

		connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), this, SLOT(globalSettingsChanged(int)));
    }
}

void KeyboardDaemon::unregisterShortcut()
{
	// register KDE keyboard shortcut for switching layouts
    if( actionCollection != NULL ) {
        disconnect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), this, SLOT(globalSettingsChanged(int)));

		disconnect(actionCollection, SIGNAL(actionTriggered(QAction*)), this, SLOT(setLayout(QAction*)));
        disconnect(actionCollection->getToggeAction(), SIGNAL(triggered()), this, SLOT(switchToNextLayout()));

        delete actionCollection;
        actionCollection = NULL;
    }
}

void KeyboardDaemon::registerListeners()
{
	if( xEventNotifier == NULL ) {
		xEventNotifier = new XInputEventNotifier();
	}
	connect(xEventNotifier, SIGNAL(newPointerDevice()), this, SLOT(configureMouse()));
	connect(xEventNotifier, SIGNAL(newKeyboardDevice()), this, SLOT(configureKeyboard()));
	connect(xEventNotifier, SIGNAL(layoutMapChanged()), this, SLOT(layoutMapChanged()));
	connect(xEventNotifier, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
	xEventNotifier->start();
}

void KeyboardDaemon::unregisterListeners()
{
	if( xEventNotifier != NULL ) {
		xEventNotifier->stop();
		disconnect(xEventNotifier, SIGNAL(newPointerDevice()), this, SLOT(configureMouse()));
		disconnect(xEventNotifier, SIGNAL(newKeyboardDevice()), this, SLOT(configureKeyboard()));
		disconnect(xEventNotifier, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
		disconnect(xEventNotifier, SIGNAL(layoutMapChanged()), this, SLOT(layoutMapChanged()));
	}
}

void KeyboardDaemon::globalSettingsChanged(int category)
{
	if ( category == KGlobalSettings::SETTINGS_SHORTCUTS) {
//TODO: optimize this? seems like we'll get configReload and globalShortcuts from kcm so we'll reconfigure twice
		unregisterShortcut();
		registerShortcut();
	}
}

void KeyboardDaemon::layoutChanged()
{
	//TODO: pass newLayout into layoutTrayIcon?
    LayoutUnit newLayout = X11Helper::getCurrentLayout();

	layoutMemory.layoutChanged();
	if( layoutTrayIcon != NULL ) {
		layoutTrayIcon->layoutChanged();
	}

	if( newLayout != currentLayout ) {
            currentLayout = newLayout;
            emit currentLayoutChanged(newLayout.toString());
        }
}

void KeyboardDaemon::layoutMapChanged()
{
	keyboardConfig.load();
	layoutMemory.layoutMapChanged();
	emit layoutListChanged();
	if( layoutTrayIcon != NULL ) {
		layoutTrayIcon->layoutMapChanged();
	}
}

void KeyboardDaemon::switchToNextLayout()
{
	qCDebug(KCM_KEYBOARD) << "Toggling layout";
	X11Helper::switchToNextLayout();

        LayoutUnit newLayout = X11Helper::getCurrentLayout();

        QDBusMessage msg = QDBusMessage::createMethodCall(
        QLatin1Literal("org.kde.plasmashell"),
        QLatin1Literal("/org/kde/osdService"),
        QLatin1Literal("org.kde.osdService"),
        QLatin1Literal("kbdLayoutChanged"));

        msg.setArguments(QList<QVariant>() << newLayout.getDisplayName());
        QDBusConnection::sessionBus().asyncCall(msg);
}

bool KeyboardDaemon::setLayout(QAction* action)
{
	if( action == actionCollection->getToggeAction() )
		return false;

	LayoutUnit layoutUnit(action->data().toString());
	return LayoutsMenu::switchToLayout(layoutUnit, keyboardConfig);	// need this to be able to switch to spare layouts
//	return X11Helper::setLayout(LayoutUnit(action->data().toString()));
}

bool KeyboardDaemon::setLayout(const QString& layout)
{
	return X11Helper::setLayout(LayoutUnit(layout));
}

QString KeyboardDaemon::getCurrentLayout()
{
	return X11Helper::getCurrentLayout().toString();
}

QStringList KeyboardDaemon::getLayoutsList()
{
	return X11Helper::getLayoutsListAsString( X11Helper::getLayoutsList() );
}

#include "keyboard_daemon.moc"
