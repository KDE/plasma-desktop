/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bindings.h"
#include "debug.h"

#include <KAboutData>
#include <KGlobalAccel>
#include <KLocalizedString>

#include <QAction>
#include <QList>

#include "flags.h"
#include "x11_helper.h"

KeyboardLayoutActionCollection::KeyboardLayoutActionCollection(QObject *parent, bool configAction_)
    : KActionCollection(parent, QStringLiteral("KDE Keyboard Layout Switcher"))
    , configAction(configAction_)
{
    setComponentDisplayName(i18n("Keyboard Layout Switcher"));
    QAction *toggleAction = addAction(QStringLiteral("Switch to Next Keyboard Layout"));
    toggleAction->setText(i18n("Switch to Next Keyboard Layout"));
    KGlobalAccel::self()->setShortcut(toggleAction, QList<QKeySequence>() << QKeySequence(Qt::META | Qt::ALT | Qt::Key_K), KGlobalAccel::Autoloading);

    QAction *lastUsedLayoutAction = addAction(QStringLiteral("Switch to Last-Used Keyboard Layout"));
    lastUsedLayoutAction->setText(i18n("Switch to Last-Used Keyboard Layout"));
    KGlobalAccel::self()->setShortcut(lastUsedLayoutAction, QList<QKeySequence>() << QKeySequence(Qt::META | Qt::ALT | Qt::Key_L), KGlobalAccel::Autoloading);

    if (configAction) {
        toggleAction->setProperty("isConfigurationAction", true);
        lastUsedLayoutAction->setProperty("isConfigurationAction", true);
    }
}

KeyboardLayoutActionCollection::~KeyboardLayoutActionCollection()
{
    clear();
}

QAction *KeyboardLayoutActionCollection::getToggleAction()
{
    return action(0);
}

QAction *KeyboardLayoutActionCollection::getLastUsedLayoutAction()
{
    return action(1);
}

QAction *KeyboardLayoutActionCollection::createLayoutShortcutActon(const LayoutUnit &layoutUnit, int layoutIndex, bool autoload)
{
    QString longLayoutName = Flags::getLongText(layoutUnit);
    QString actionName = QStringLiteral("Switch keyboard layout to ");
    actionName += longLayoutName;
    QAction *action = addAction(actionName);
    action->setText(i18n("Switch keyboard layout to %1", longLayoutName));
    KGlobalAccel::GlobalShortcutLoading loading = autoload ? KGlobalAccel::Autoloading : KGlobalAccel::NoAutoloading;
    QList<QKeySequence> shortcuts;
    if (!autoload) {
        shortcuts << layoutUnit.getShortcut();
    }
    KGlobalAccel::self()->setShortcut(action, shortcuts, loading);
    action->setData(layoutIndex);
    if (configAction) {
        action->setProperty("isConfigurationAction", true);
    }
    return action;
}

void KeyboardLayoutActionCollection::setToggleShortcut(const QKeySequence &keySequence)
{
    KGlobalAccel::self()->setShortcut(getToggleAction(), QList<QKeySequence>() << keySequence, KGlobalAccel::NoAutoloading);
}

void KeyboardLayoutActionCollection::setLastUsedLayoutShortcut(const QKeySequence &keySequence)
{
    KGlobalAccel::self()->setShortcut(getLastUsedLayoutAction(), QList<QKeySequence>() << keySequence, KGlobalAccel::NoAutoloading);
}

void KeyboardLayoutActionCollection::setLayoutShortcuts(QList<LayoutUnit> &layoutUnits)
{
    for (int i = 0; i < layoutUnits.size(); ++i) {
        const LayoutUnit &layoutUnit = layoutUnits.at(i);
        if (!layoutUnit.getShortcut().isEmpty()) {
            createLayoutShortcutActon(layoutUnit, i, false);
        }
    }
    qCDebug(KCM_KEYBOARD) << "Cleaning component shortcuts on save" << KGlobalAccel::cleanComponent(QStringLiteral("KDE Keyboard Layout Switcher"));
}

void KeyboardLayoutActionCollection::loadLayoutShortcuts(QList<LayoutUnit> &layoutUnits)
{
    for (int i = 0; i < layoutUnits.size(); ++i) {
        LayoutUnit &layoutUnit = layoutUnits[i];
        QAction *action = createLayoutShortcutActon(layoutUnit, i, true);
        const auto shortcut = KGlobalAccel::self()->shortcut(action);
        if (!shortcut.isEmpty()) {
            qCDebug(KCM_KEYBOARD, ) << "Restored shortcut for" << layoutUnit.toString() << shortcut.first();
            layoutUnit.setShortcut(shortcut.first());
        } else {
            qCDebug(KCM_KEYBOARD, ) << "Skipping empty shortcut for" << layoutUnit.toString();
            removeAction(action);
        }
    }
    qCDebug(KCM_KEYBOARD) << "Cleaning component shortcuts on load" << KGlobalAccel::cleanComponent(QStringLiteral("KDE Keyboard Layout Switcher"));
}

void KeyboardLayoutActionCollection::resetLayoutShortcuts()
{
    for (int i = 2; i < actions().size(); i++) {
        KGlobalAccel::self()->setShortcut(action(i), QList<QKeySequence>(), KGlobalAccel::NoAutoloading);
        KGlobalAccel::self()->setDefaultShortcut(action(i), QList<QKeySequence>(), KGlobalAccel::NoAutoloading);
    }
}
