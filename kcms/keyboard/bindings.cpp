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

static const char actionName[] = I18N_NOOP("Switch to Next Keyboard Layout");
static const char COMPONENT_NAME[] = I18N_NOOP("KDE Keyboard Layout Switcher");

KeyboardLayoutActionCollection::KeyboardLayoutActionCollection(QObject *parent, bool configAction_)
    : KActionCollection(parent, COMPONENT_NAME)
    , configAction(configAction_)
{
    setComponentDisplayName(i18n("Keyboard Layout Switcher"));
    QAction *toggleAction = addAction(actionName);
    toggleAction->setText(i18n(actionName));
    KGlobalAccel::self()->setShortcut(toggleAction, QList<QKeySequence>() << QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_K), KGlobalAccel::Autoloading);
    if (configAction) {
        toggleAction->setProperty("isConfigurationAction", true);
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

QAction *KeyboardLayoutActionCollection::createLayoutShortcutActon(const LayoutUnit &layoutUnit, int layoutIndex, const Rules *rules, bool autoload)
{
    QString longLayoutName = Flags::getLongText(layoutUnit, rules);
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

void KeyboardLayoutActionCollection::setLayoutShortcuts(QList<LayoutUnit> &layoutUnits, const Rules *rules)
{
    for (int i = 0; i < layoutUnits.size(); ++i) {
        const LayoutUnit &layoutUnit = layoutUnits.at(i);
        if (!layoutUnit.getShortcut().isEmpty()) {
            createLayoutShortcutActon(layoutUnit, i, rules, false);
        }
    }
    qCDebug(KCM_KEYBOARD) << "Cleaning component shortcuts on save" << KGlobalAccel::cleanComponent(COMPONENT_NAME);
}

void KeyboardLayoutActionCollection::loadLayoutShortcuts(QList<LayoutUnit> &layoutUnits, const Rules *rules)
{
    for (int i = 0; i < layoutUnits.size(); ++i) {
        LayoutUnit &layoutUnit = layoutUnits[i];
        QAction *action = createLayoutShortcutActon(layoutUnit, i, rules, true);
        const auto shortcut = KGlobalAccel::self()->shortcut(action);
        if (!shortcut.isEmpty()) {
            qCDebug(KCM_KEYBOARD, ) << "Restored shortcut for" << layoutUnit.toString() << shortcut.first();
            layoutUnit.setShortcut(shortcut.first());
        } else {
            qCDebug(KCM_KEYBOARD, ) << "Skipping empty shortcut for" << layoutUnit.toString();
            removeAction(action);
        }
    }
    qCDebug(KCM_KEYBOARD) << "Cleaning component shortcuts on load" << KGlobalAccel::cleanComponent(COMPONENT_NAME);
}

void KeyboardLayoutActionCollection::resetLayoutShortcuts()
{
    for (int i = 1; i < actions().size(); i++) {
        KGlobalAccel::self()->setShortcut(action(i), QList<QKeySequence>(), KGlobalAccel::NoAutoloading);
        KGlobalAccel::self()->setDefaultShortcut(action(i), QList<QKeySequence>(), KGlobalAccel::NoAutoloading);
    }
}
