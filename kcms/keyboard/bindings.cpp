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

#include "bindings.h"
#include "debug.h"

#include <KLocalizedString>
#include <KGlobalAccel>
#include <KAboutData>

#include <QList>
#include <QAction>

#include "x11_helper.h"
#include "flags.h"


static const char actionName[] = I18N_NOOP("Switch to Next Keyboard Layout");
static const char COMPONENT_NAME[] = I18N_NOOP("KDE Keyboard Layout Switcher");


KeyboardLayoutActionCollection::KeyboardLayoutActionCollection(QObject* parent, bool configAction_):
		KActionCollection(parent, COMPONENT_NAME),
		configAction(configAction_)
{
    setComponentDisplayName(i18n("Keyboard Layout Switcher"));
	QAction* toggleAction = addAction( actionName );
	toggleAction->setText( i18n(actionName) );
        KGlobalAccel::self()->setShortcut(toggleAction,
                                          QList<QKeySequence>() << QKeySequence(Qt::ALT+Qt::CTRL+Qt::Key_K),
                                          KGlobalAccel::Autoloading);
	if( configAction ) {
	    toggleAction->setProperty("isConfigurationAction", true);
	}
}

KeyboardLayoutActionCollection::~KeyboardLayoutActionCollection()
{
    clear();
}

QAction* KeyboardLayoutActionCollection::getToggleAction()
{
	return action(0);
}

QAction* KeyboardLayoutActionCollection::createLayoutShortcutActon(const LayoutUnit& layoutUnit, const Rules* rules, bool autoload)
{
	QString longLayoutName = Flags::getLongText( layoutUnit, rules );
	QString actionName = QStringLiteral("Switch keyboard layout to ");
	actionName += longLayoutName;
	QAction* action = addAction( actionName );
	action->setText( i18n("Switch keyboard layout to %1", longLayoutName) );
	KGlobalAccel::GlobalShortcutLoading loading = autoload ? KGlobalAccel::Autoloading : KGlobalAccel::NoAutoloading;
        QList<QKeySequence> shortcuts;
        if (!autoload) {
            shortcuts << layoutUnit.getShortcut();
        }
        KGlobalAccel::self()->setShortcut(action, shortcuts, loading);
	action->setData(layoutUnit.toString());
	if( configAction ) {
	    action->setProperty("isConfigurationAction", true);
	}
	return action;
}

void KeyboardLayoutActionCollection::setToggleShortcut(const QKeySequence& keySequence)
{
    KGlobalAccel::self()->setShortcut(getToggleAction(), QList<QKeySequence>() << keySequence, KGlobalAccel::NoAutoloading);
}

//KAction* KeyboardLayoutActionCollection::setShortcut(LayoutUnit& layoutUnit, const QKeySequence& keySequence, const Rules* rules)
//{
//	KAction* action = getAction(layoutUnit);
//	if( action == NULL && ! keySequence.isEmpty() ) {
//		action = createLayoutShortcutActon(layoutUnit, rules, false);
//	}
//	else if( action != NULL && keySequence.isEmpty() ){
////		action->setGlobalShortcut(KShortcut(keySequence), KAction::ActiveShortcut, KAction::NoAutoloading);	// do we need this?
//		removeAction(action);
//		action = NULL;
//	}
////	if( configAction ) {
////		layoutUnit.setShortcut(keySequence);	// shortcut was restored
////	}
//	return action;
//}

void KeyboardLayoutActionCollection::setLayoutShortcuts(QList<LayoutUnit>& layoutUnits, const Rules* rules)
{
	for (QList<LayoutUnit>::iterator i = layoutUnits.begin(); i != layoutUnits.end(); ++i) {
		LayoutUnit& layoutUnit = *i;
		if( ! layoutUnit.getShortcut().isEmpty() ) {
			createLayoutShortcutActon(layoutUnit, rules, false);
		}
	}
	qCDebug(KCM_KEYBOARD) << "Cleaning component shortcuts on save" << KGlobalAccel::cleanComponent(COMPONENT_NAME);
}

void KeyboardLayoutActionCollection::loadLayoutShortcuts(QList<LayoutUnit>& layoutUnits, const Rules* rules)
{
	for (QList<LayoutUnit>::iterator i = layoutUnits.begin(); i != layoutUnits.end(); ++i) {
		LayoutUnit& layoutUnit = *i;
		QAction* action = createLayoutShortcutActon(layoutUnit, rules, true);
                const auto shortcut = KGlobalAccel::self()->shortcut(action);
		if( ! shortcut.isEmpty() ) {
			qCDebug(KCM_KEYBOARD, ) << "Restored shortcut for" << layoutUnit.toString() << shortcut.first();
			layoutUnit.setShortcut(shortcut.first());
		}
		else {
			qCDebug(KCM_KEYBOARD, ) << "Skipping empty shortcut for" << layoutUnit.toString();
			removeAction(action);
		}
	}
	qCDebug(KCM_KEYBOARD) << "Cleaning component shortcuts on load" << KGlobalAccel::cleanComponent(COMPONENT_NAME);
}

//KAction* KeyboardLayoutActionCollection::getAction(const LayoutUnit& layoutUnit)
//{
//	for(int i=1; i<actions().size(); i++) {
//		if( action(i)->data() == layoutUnit.toString() )
//			return static_cast<KAction*>(action(i));
//	}
//	return NULL;
//}

void KeyboardLayoutActionCollection::resetLayoutShortcuts()
{
	for(int i=1; i<actions().size(); i++) {
            KGlobalAccel::self()->setShortcut(action(i), QList<QKeySequence>(), KGlobalAccel::NoAutoloading);
            KGlobalAccel::self()->setDefaultShortcut(action(i), QList<QKeySequence>(), KGlobalAccel::NoAutoloading);
	}
}
