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

#include <KGlobalAccel>
#include <QAction>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QProcess>
#include <QTimer>
#include <QX11Info>

#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>

#include "input_sources.h"
#include "../keyboard_dbus.h"
#include "../xkb_rules.h"
#include "bindings.h"
#include "keyboard_hardware.h"
#include "x11_helper.h"
#include "xinput_helper.h"
#include "xkb_helper.h"

// TODO: implement switching policy

K_PLUGIN_FACTORY_WITH_JSON(KeyboardFactory,
                           "keyboard.json",
                           registerPlugin<KeyboardDaemon>();)

KeyboardDaemon::KeyboardDaemon(QObject* parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , m_layoutListModels(new LayoutListModels(this))
    , m_currentLayoutIndex(0)
    , actionCollection(nullptr)
    , xEventNotifier(nullptr)
    , m_fcitxDBusProxy("org.fcitx.Fcitx", "/inputmethod", QDBusConnection::sessionBus())
{
    if (!X11Helper::xkbSupported(nullptr))
        return; //TODO: shut down the daemon?

    FcitxQtInputMethodItem::registerMetaType();

    // Register DBus Service
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService(KEYBOARD_DBUS_SERVICE_NAME);
    dbus.registerObject(KEYBOARD_DBUS_OBJECT_PATH, this, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);
    dbus.connect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT(configureKeyboard()));
    dbus.connect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SIGNAL(configChanged()));

    // Setup timers for applying multiple events at once
    configureKeyboardTimer.setSingleShot(true);
    configureKeyboardTimer.setInterval(50);
    connect(&configureKeyboardTimer, &QTimer::timeout, this, &KeyboardDaemon::configureKeyboard);
    configureMouseTimer.setSingleShot(true);
    configureMouseTimer.setInterval(50);
    connect(&configureMouseTimer, &QTimer::timeout, this, &KeyboardDaemon::configureMouse);

    configureKeyboard();
    registerListeners();
    connect(InputSources::self(), &InputSources::currentSourceChanged, this, &KeyboardDaemon::registerListeners);
    connect(InputSources::self(), &InputSources::currentSourceChanged, [this]() { configureMouseTimer.start(); });
    connect(InputSources::self(), &InputSources::currentSourceChanged, [this]() { configureKeyboardTimer.start(); });
}

KeyboardDaemon::~KeyboardDaemon()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.disconnect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT(configureKeyboard()));
    dbus.unregisterObject(KEYBOARD_DBUS_OBJECT_PATH);
    dbus.unregisterService(KEYBOARD_DBUS_SERVICE_NAME);

    unregisterListeners();
    unregisterShortcut();

    delete xEventNotifier;
}

QString KeyboardDaemon::updateCurrentLayout(int newLayoutIndex)
{
    m_currentLayoutIndex = newLayoutIndex;

    QString newLayoutSaveName = m_layoutListModels->currentLayoutListModel()->data(
        m_layoutListModels->currentLayoutListModel()->index(m_currentLayoutIndex, 0),
        LayoutListModelBase::Roles::SaveNameRole).toString();
    QString newLayoutName = m_layoutListModels->currentLayoutListModel()->data(
        m_layoutListModels->currentLayoutListModel()->index(m_currentLayoutIndex, 0),
        LayoutListModelBase::Roles::NameRole).toString();
    QString newLayoutDesc = m_layoutListModels->currentLayoutListModel()->data(
        m_layoutListModels->currentLayoutListModel()->index(m_currentLayoutIndex, 0),
        LayoutListModelBase::Roles::DescriptionRole).toString();

    if (InputSources::self()->currentSource() == InputSources::Sources::FcitxSource) {
        FcitxQtInputMethodItemList list = m_fcitxDBusProxy.iMList();
        bool isLatinModeEnabled = m_layoutListModels->currentLayoutListModel()->data(
            m_layoutListModels->currentLayoutListModel()->index(m_currentLayoutIndex, 0),
            LayoutListModelBase::Roles::IsLatinModeEnabledRole).toBool();
        QString latinModeLayout = m_layoutListModels->currentLayoutListModel()->data(
            m_layoutListModels->currentLayoutListModel()->index(m_currentLayoutIndex, 0),
            LayoutListModelBase::Roles::LatinModeLayoutRole).toString();

        qDebug(KCM_KEYBOARD) << newLayoutName << isLatinModeEnabled << latinModeLayout;

        for (int i = 0; i < list.size(); ++i) {
            list[i].setEnabled(false);
            if (list[i].uniqueName() == newLayoutName) {
                list[i].setEnabled(true);
                if (isLatinModeEnabled) {
                    list.move(i, 1);
                } else {
                    list.move(i, 0);
                }
            }
            if (list[i].uniqueName() == latinModeLayout) {
                list[i].setEnabled(true);
                list.move(i, 0);
            }
        }

        m_fcitxDBusProxy.setIMList(list);
    }
    else if (InputSources::self()->currentSource() == InputSources::Sources::XkbSource) {
        // Set xkb layouts and variants
        QStringList setxkbmapCommandArguments;
        QString layout;
        QString variant = "";

        int source = m_layoutListModels->currentLayoutListModel()->data(
                            m_layoutListModels->currentLayoutListModel()->index(m_currentLayoutIndex, 0),
                            LayoutListModelBase::Roles::SourceRole).toInt();
        if (source == InputSources::Sources::XkbSource) {
            QStringList lv = newLayoutName.split('(');
            layout = lv[0];
            if (lv.size() >= 2) {
                variant = lv[1].left(lv[1].size() - 1);
            }
        }

        setxkbmapCommandArguments.append(QStringLiteral("-layout"));
        setxkbmapCommandArguments.append(layout);
        if (!variant.isEmpty()) {
            setxkbmapCommandArguments.append(QStringLiteral("-variant"));
            setxkbmapCommandArguments.append(variant);
        }
        XkbHelper::runConfigLayoutCommand(setxkbmapCommandArguments);
    }

    qDebug() << "currentLayoutChanged emitted";
    emit currentLayoutChanged(newLayoutSaveName);

    return newLayoutDesc;
}

void KeyboardDaemon::configureKeyboard()
{
    qCDebug(KCM_KEYBOARD) << "Configuring keyboard";

    KConfigGroup config(
        KSharedConfig::openConfig(QStringLiteral("kxkbrc"), KConfig::NoGlobals),
        QStringLiteral("Layout"));

    m_layoutListModels->loadConfig();
    init_keyboard_hardware();

    // Reset options
    XkbHelper::runConfigLayoutCommand(QStringList() << "-option");

    // Set keyboard model
    QStringList setxkbmapCommandArguments;
    QString keyboardModel = config.readEntry<QString>("Model", "");
    if (!keyboardModel.isEmpty()) {
        setxkbmapCommandArguments.append(QStringLiteral("-model"));
        setxkbmapCommandArguments.append(keyboardModel);
    }

    // Set advanced options
    QString options = config.readEntry<QString>("Options", "");
    setxkbmapCommandArguments.append(QStringLiteral("-option"));
    setxkbmapCommandArguments.append(options);

    XkbHelper::runConfigLayoutCommand(setxkbmapCommandArguments);

    // Reset index
    updateCurrentLayout(0);

    // Re-register shortcut keys
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

        QAction* toggleLayoutAction = actionCollection->getToggleAction();
        connect(toggleLayoutAction, &QAction::triggered, this, &KeyboardDaemon::switchToNextLayout);
        qCDebug(KCM_KEYBOARD) << KGlobalAccel::self()->shortcut(toggleLayoutAction);
        //actionCollection->loadLayoutShortcuts(keyboardConfig.layouts, rules.data());
        //connect(actionCollection, SIGNAL(actionTriggered(QAction*)), this, SLOT(setLayout(QAction*)));
    }
}

void KeyboardDaemon::unregisterShortcut()
{
    // register KDE keyboard shortcut for switching layouts
    if (actionCollection != nullptr) {
        disconnect(actionCollection, SIGNAL(actionTriggered(QAction*)), this, SLOT(setLayout(QAction*)));
        disconnect(actionCollection->getToggleAction(), &QAction::triggered, this, &KeyboardDaemon::switchToNextLayout);

        delete actionCollection;
        actionCollection = nullptr;
    }
}

void KeyboardDaemon::registerListeners()
{
    if (InputSources::self()->currentSource() == InputSources::Sources::XkbSource) {
        if (!isXEventRegistered) {
            if (xEventNotifier == nullptr) {
                xEventNotifier = new XInputEventNotifier();
            }
            connect(xEventNotifier, &XInputEventNotifier::newPointerDevice, [this]() { configureMouseTimer.start(); });
            connect(xEventNotifier, &XInputEventNotifier::newKeyboardDevice, [this]() { configureKeyboardTimer.start(); });
            connect(xEventNotifier, &XEventNotifier::layoutMapChanged, this, &KeyboardDaemon::layoutMapChanged);
            connect(xEventNotifier, &XEventNotifier::layoutChanged, this, &KeyboardDaemon::layoutChanged);
            xEventNotifier->start();
            isXEventRegistered = true;
        }
    }
    else {
        unregisterListeners();
    }
}

void KeyboardDaemon::unregisterListeners()
{
    if (xEventNotifier != nullptr && isXEventRegistered) {
        xEventNotifier->stop();
        disconnect(xEventNotifier, &XEventNotifier::layoutChanged, this, &KeyboardDaemon::layoutChanged);
        disconnect(xEventNotifier, &XEventNotifier::layoutMapChanged, this, &KeyboardDaemon::layoutMapChanged);
    }
}

void KeyboardDaemon::layoutChanged()
{
    // TODO
}

void KeyboardDaemon::layoutMapChanged()
{
    //configureKeyboard();
}

void KeyboardDaemon::switchToNextLayout()
{
    const int origIdx = m_currentLayoutIndex;
    const int layoutCount = m_layoutListModels->currentLayoutListModel()->rowCount();
    if (layoutCount <= 1) {
        qWarning(KCM_KEYBOARD) << "Cannot find next layout";
        return;
    }
    int newIdx = (origIdx + 1) % layoutCount;

    QString layoutName = updateCurrentLayout(newIdx);

    qCDebug(KCM_KEYBOARD) << "Toggling layout " << layoutName;

    // Show a box on the screen indicating the layout change
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.kde.plasmashell"),
        QStringLiteral("/org/kde/osdService"),
        QStringLiteral("org.kde.osdService"),
        QStringLiteral("kbdLayoutChanged"));

    msg << layoutName;

    QDBusConnection::sessionBus().asyncCall(msg);
}

bool KeyboardDaemon::setLayout(QAction* action)
{
    if (action == actionCollection->getToggleAction())
        return false;

    QString layoutName = action->data().toString();
    return setLayout(layoutName);
}

bool KeyboardDaemon::setLayout(const QString& layout)
{
    qDebug() << layout;
    const int layoutCount = m_layoutListModels->currentLayoutListModel()->rowCount();
    for (int i = 0; i < layoutCount; ++i) {
        QString saveName = m_layoutListModels->currentLayoutListModel()->data(
           m_layoutListModels->currentLayoutListModel()->index(i, 0),
           LayoutListModelBase::Roles::SaveNameRole).toString();

        if (saveName == layout) {
            updateCurrentLayout(i);
            return true;
        }
    }
    return false;
}

QString KeyboardDaemon::getCurrentLayout()
{
    return m_layoutListModels->currentLayoutListModel()->data(
        m_layoutListModels->currentLayoutListModel()->index(m_currentLayoutIndex, 0),
        LayoutListModelBase::Roles::SaveNameRole).toString();
}

QStringList KeyboardDaemon::getLayoutsList()
{
    QStringList list;

    for (int i = 0; i < m_layoutListModels->currentLayoutListModel()->rowCount(); ++i) {
        QString layout = m_layoutListModels->currentLayoutListModel()->data(
                    m_layoutListModels->currentLayoutListModel()->index(i, 0),
                    LayoutListModelBase::Roles::SaveNameRole).toString();
        list << layout;
    }
    return list;
}

QString KeyboardDaemon::getLayoutDisplayName(const QString &layout)
{
    const int layoutCount = m_layoutListModels->currentLayoutListModel()->rowCount();
    for (int i = 0; i < layoutCount; ++i) {
        QString saveName = m_layoutListModels->currentLayoutListModel()->data(
           m_layoutListModels->currentLayoutListModel()->index(i, 0),
           LayoutListModelBase::Roles::SaveNameRole).toString();

        if (saveName == layout) {
            return m_layoutListModels->currentLayoutListModel()->data(
               m_layoutListModels->currentLayoutListModel()->index(i, 0),
               LayoutListModelBase::Roles::DescriptionRole).toString();
        }
    }
    return "";
}

#include "keyboard_daemon.moc"
