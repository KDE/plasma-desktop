/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keyboard_daemon.h"
#include "debug.h"

#include <QAction>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QProcess>
#include <QX11Info>

#include <KPluginFactory>

#include "keyboard_hardware.h"
#include "layout_memory_persister.h"
#include "x11_helper.h"
#include "xinput_helper.h"
#include "xkb_helper.h"
#include "xkb_rules.h"
#include "flags.h"

K_PLUGIN_CLASS_WITH_JSON(KeyboardDaemon, "keyboard.json")

KeyboardDaemon::KeyboardDaemon(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
    , actionCollection(nullptr)
    , xEventNotifier(nullptr)
    , layoutMemory(keyboardConfig)
    , rules(Rules::readRules(Rules::READ_EXTRAS))
{
    if (!X11Helper::xkbSupported(nullptr))
        return; // TODO: shut down the daemon?

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService(KEYBOARD_DBUS_SERVICE_NAME);
    dbus.registerObject(KEYBOARD_DBUS_OBJECT_PATH, this, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);
    dbus.connect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT(configureKeyboard()));

    LayoutNames::registerMetaType();

    configureKeyboard();
    registerListeners();

    LayoutMemoryPersister layoutMemoryPersister(layoutMemory);
    if (layoutMemoryPersister.restore()) {
        if (layoutMemoryPersister.getGlobalLayout().isValid()) {
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
    delete rules;
}

void KeyboardDaemon::configureKeyboard()
{
    qCDebug(KCM_KEYBOARD) << "Configuring keyboard";
    init_keyboard_hardware();

    keyboardConfig.load();
    XkbHelper::initializeKeyboardLayouts(keyboardConfig);
    layoutMemory.configChanged();

    unregisterShortcut();
    registerShortcut();
}

void KeyboardDaemon::configureMouse()
{
    QStringList modules;
    modules << QStringLiteral("mouse");
    QProcess::startDetached(QStringLiteral("kcminit"), modules);
}

void KeyboardDaemon::registerShortcut()
{
    if (actionCollection == nullptr) {
        actionCollection = new KeyboardLayoutActionCollection(this, false);

        QAction *toggleLayoutAction = actionCollection->getToggleAction();
        connect(toggleLayoutAction, &QAction::triggered, this, [this]() {
            switchToNextLayout();

            LayoutUnit newLayout = X11Helper::getCurrentLayout();
            QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                              QStringLiteral("/org/kde/osdService"),
                                                              QStringLiteral("org.kde.osdService"),
                                                              QStringLiteral("kbdLayoutChanged"));
            msg << Flags::getLongText(newLayout, rules);
            QDBusConnection::sessionBus().asyncCall(msg);
        });

        actionCollection->loadLayoutShortcuts(keyboardConfig.layouts, rules);
        // clang-format off
		connect(actionCollection, SIGNAL(actionTriggered(QAction*)), this, SLOT(setLayout(QAction*)));
        // clang-format on
    }
}

void KeyboardDaemon::unregisterShortcut()
{
    // register KDE keyboard shortcut for switching layouts
    if (actionCollection != nullptr) {
        // clang-format off
		disconnect(actionCollection, SIGNAL(actionTriggered(QAction*)), this, SLOT(setLayout(QAction*)));
        // clang-format on
        disconnect(actionCollection->getToggleAction(), &QAction::triggered, this, &KeyboardDaemon::switchToNextLayout);

        delete actionCollection;
        actionCollection = nullptr;
    }
}

void KeyboardDaemon::registerListeners()
{
    if (xEventNotifier == nullptr) {
        xEventNotifier = new XInputEventNotifier();
    }
    connect(xEventNotifier, &XInputEventNotifier::newPointerDevice, this, &KeyboardDaemon::configureMouse);
    connect(xEventNotifier, &XInputEventNotifier::newKeyboardDevice, this, &KeyboardDaemon::configureKeyboard);
    connect(xEventNotifier, &XEventNotifier::layoutMapChanged, this, &KeyboardDaemon::layoutMapChanged);
    connect(xEventNotifier, &XEventNotifier::layoutChanged, this, &KeyboardDaemon::layoutChangedSlot);
    xEventNotifier->start();
}

void KeyboardDaemon::unregisterListeners()
{
    if (xEventNotifier != nullptr) {
        xEventNotifier->stop();
        disconnect(xEventNotifier, &XInputEventNotifier::newPointerDevice, this, &KeyboardDaemon::configureMouse);
        disconnect(xEventNotifier, &XInputEventNotifier::newKeyboardDevice, this, &KeyboardDaemon::configureKeyboard);
        disconnect(xEventNotifier, &XEventNotifier::layoutChanged, this, &KeyboardDaemon::layoutChangedSlot);
        disconnect(xEventNotifier, &XEventNotifier::layoutMapChanged, this, &KeyboardDaemon::layoutMapChanged);
    }
}

void KeyboardDaemon::layoutChangedSlot()
{
    layoutMemory.layoutChanged();

    Q_EMIT layoutChanged(getLayout());
}

void KeyboardDaemon::layoutMapChanged()
{
    keyboardConfig.load();
    layoutMemory.layoutMapChanged();
    Q_EMIT layoutListChanged();
}

void KeyboardDaemon::switchToNextLayout()
{
    X11Helper::scrollLayouts(1);
}

void KeyboardDaemon::switchToPreviousLayout()
{
    X11Helper::scrollLayouts(-1);
}

bool KeyboardDaemon::setLayout(QAction *action)
{
    if (action == actionCollection->getToggleAction())
        return false;

    return setLayout(action->data().toUInt());
}

bool KeyboardDaemon::setLayout(uint index)
{
    if (keyboardConfig.layoutLoopCount != KeyboardConfig::NO_LOOPING && index >= uint(keyboardConfig.layoutLoopCount)) {
        QList<LayoutUnit> layouts = X11Helper::getLayoutsList();
        if ( int(index) <= keyboardConfig.layouts.lastIndexOf(layouts.takeLast()) ) {
            // got to a shifted diapason due to previously selected spare layout, so adjusting the index accordingly
            --index;
        }
        // spare layout preempts last one in the loop
        layouts.append(keyboardConfig.layouts.at(index));
        XkbHelper::initializeKeyboardLayouts(layouts);
        index = layouts.size() - 1;
    }
    return X11Helper::setGroup(index);
}

uint KeyboardDaemon::getLayout() const
{
    return X11Helper::getGroup();
}

QVector<LayoutNames> KeyboardDaemon::getLayoutsList() const
{
    QVector<LayoutNames> ret;

    auto layoutsList = X11Helper::getLayoutsList();
    if (keyboardConfig.layoutLoopCount != KeyboardConfig::NO_LOOPING) {
        // extra layouts list overlaps with the main layouts loop initially by 1 position
        auto extraLayouts = keyboardConfig.layouts.mid(keyboardConfig.layoutLoopCount - 1);
        // spare layout currently placed in the loop is removed from the extra layouts
        // as it was already "moved" to the last loop position
        extraLayouts.removeOne(layoutsList.last());
        layoutsList.append(extraLayouts);
    }
    for (auto &layoutUnit : std::as_const(layoutsList)) {
        ret.append({layoutUnit.layout(), Flags::getShortText(layoutUnit, keyboardConfig), Flags::getLongText(layoutUnit, rules)});
    }
    return ret;
}

#include "keyboard_daemon.moc"
