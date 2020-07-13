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
#include "debug.h"

#include <QAction>
#include <QX11Info>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QProcess>

#include <KPluginFactory>

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

K_PLUGIN_FACTORY_WITH_JSON(KeyboardFactory,
                           "keyboard.json",
                           registerPlugin<KeyboardDaemon>();)

KeyboardDaemon::KeyboardDaemon(QObject *parent, const QList<QVariant>&)
	: KDEDModule(parent),
	  actionCollection(nullptr),
	  xEventNotifier(nullptr),
	  layoutTrayIcon(nullptr),
	  layoutMemory(keyboardConfig),
	  rules(Rules::readRules(Rules::READ_EXTRAS))
{
	if( ! X11Helper::xkbSupported(nullptr) )
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
    layoutMemoryPersister.setGlobalLayout(currentLayout);
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
	XkbHelper::initializeKeyboardLayouts(keyboardConfig);
	layoutMemory.configChanged();
	keyboardConfig.save();

	setupTrayIcon();

	unregisterShortcut();
	registerShortcut();
}

void KeyboardDaemon::configureMouse()
{
    QStringList modules;
    modules << QStringLiteral("mouse");
    QProcess::startDetached(QStringLiteral("kcminit"), modules);
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
		layoutTrayIcon = nullptr;
	}
}

void KeyboardDaemon::registerShortcut()
{
	if( actionCollection == nullptr ) {
		actionCollection = new KeyboardLayoutActionCollection(this, false);

		QAction* toggleLayoutAction = actionCollection->getToggleAction();
		connect(toggleLayoutAction, &QAction::triggered, this, &KeyboardDaemon::switchToNextLayout);
		actionCollection->loadLayoutShortcuts(keyboardConfig.layouts, rules);
		connect(actionCollection, SIGNAL(actionTriggered(QAction*)), this, SLOT(setLayout(QAction*)));
    }
}

void KeyboardDaemon::unregisterShortcut()
{
	// register KDE keyboard shortcut for switching layouts
    if( actionCollection != nullptr ) {
		disconnect(actionCollection, SIGNAL(actionTriggered(QAction*)), this, SLOT(setLayout(QAction*)));
        disconnect(actionCollection->getToggleAction(), &QAction::triggered, this, &KeyboardDaemon::switchToNextLayout);

        delete actionCollection;
        actionCollection = nullptr;
    }
}

void KeyboardDaemon::registerListeners()
{
	if( xEventNotifier == nullptr ) {
		xEventNotifier = new XInputEventNotifier();
	}
	connect(xEventNotifier, &XInputEventNotifier::newPointerDevice, this, &KeyboardDaemon::configureMouse);
	connect(xEventNotifier, &XInputEventNotifier::newKeyboardDevice, this, &KeyboardDaemon::configureKeyboard);
	connect(xEventNotifier, &XEventNotifier::layoutMapChanged, this, &KeyboardDaemon::layoutMapChanged);
	connect(xEventNotifier, &XEventNotifier::layoutChanged, this, &KeyboardDaemon::layoutChanged);
	xEventNotifier->start();
}

void KeyboardDaemon::unregisterListeners()
{
	if( xEventNotifier != nullptr ) {
		xEventNotifier->stop();
		disconnect(xEventNotifier, &XInputEventNotifier::newPointerDevice, this, &KeyboardDaemon::configureMouse);
		disconnect(xEventNotifier, &XInputEventNotifier::newKeyboardDevice, this, &KeyboardDaemon::configureKeyboard);
		disconnect(xEventNotifier, &XEventNotifier::layoutChanged, this, &KeyboardDaemon::layoutChanged);
		disconnect(xEventNotifier, &XEventNotifier::layoutMapChanged, this, &KeyboardDaemon::layoutMapChanged);
	}
}

void KeyboardDaemon::layoutChanged()
{
	//TODO: pass newLayout into layoutTrayIcon?
    LayoutUnit newLayout = X11Helper::getCurrentLayout();

	layoutMemory.layoutChanged();
	if( layoutTrayIcon != nullptr ) {
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
	if( layoutTrayIcon != nullptr ) {
		layoutTrayIcon->layoutMapChanged();
	}
}

void KeyboardDaemon::switchToNextLayout()
{
	qCDebug(KCM_KEYBOARD) << "Toggling layout";
	X11Helper::switchToNextLayout();

        LayoutUnit newLayout = X11Helper::getCurrentLayout();

        QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.kde.plasmashell"),
        QStringLiteral("/org/kde/osdService"),
        QStringLiteral("org.kde.osdService"),
        QStringLiteral("kbdLayoutChanged"));

        msg << Flags::getShortText(newLayout, keyboardConfig);

        QDBusConnection::sessionBus().asyncCall(msg);
}

bool KeyboardDaemon::setLayout(QAction* action)
{
	if( action == actionCollection->getToggleAction() )
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

QString KeyboardDaemon::getLayoutDisplayName(const QString &layout)
{
	return Flags::getShortText(LayoutUnit(layout), keyboardConfig);
}

#include "keyboard_daemon.moc"
