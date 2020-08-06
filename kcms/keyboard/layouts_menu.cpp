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

#include "layouts_menu.h"
#include "debug.h"

#include <KLocalizedString>

#include <QAction>
#include <QProcess>

#include "keyboard_config.h"
#include "x11_helper.h"
#include "xkb_helper.h"
#include "flags.h"


LayoutsMenu::LayoutsMenu(const KeyboardConfig& keyboardConfig_, const Rules& rules_, Flags& flags_):
	keyboardConfig(keyboardConfig_),
	rules(rules_),
	flags(flags_),
	actionGroup(nullptr)
{
}

LayoutsMenu::~LayoutsMenu()
{
	delete actionGroup;
}

const QIcon LayoutsMenu::getFlag(const QString& layout) const
{
	return keyboardConfig.isFlagShown() ? flags.getIcon(layout) : QIcon();
}

void LayoutsMenu::actionTriggered(QAction* action)
{
	QString data = action->data().toString();
	if( data == QLatin1String("config") ) {
		QStringList args;
		args << QStringLiteral("--args=--tab=layouts");
		args << QStringLiteral("kcm_keyboard");
		QProcess::startDetached(QStringLiteral("kcmshell5"), args);
	}
	else {
		LayoutUnit layoutUnit(LayoutUnit(action->data().toString()));
		switchToLayout(layoutUnit, keyboardConfig);
	}
}

int LayoutsMenu::switchToLayout(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig)
{
	QList<LayoutUnit> layouts = X11Helper::getCurrentLayouts().layouts;

	bool res;
	if( layouts.contains(layoutUnit) ) {
		res = X11Helper::setLayout(layoutUnit);
	}
	else if ( keyboardConfig.layouts.contains(layoutUnit) ) {
		QList<LayoutUnit> layouts(keyboardConfig.getDefaultLayouts());
		layouts.removeLast();
		layouts.append(layoutUnit);
		XkbHelper::initializeKeyboardLayouts(layouts);
		res = X11Helper::setLayout(layoutUnit);
	}
	else {
		qCWarning(KCM_KEYBOARD) << "switchToLayout with unknown layout" << layoutUnit.toString();
		res = -1;
	}
	return res;
}

QAction* LayoutsMenu::createAction(const LayoutUnit& layoutUnit) const
{
	QString menuText = Flags::getFullText(layoutUnit, keyboardConfig, &rules);
    QAction* action = new QAction(getFlag(layoutUnit.layout()), menuText, actionGroup);
	action->setData(layoutUnit.toString());
	//FIXME: tooltips don't work on dbusmenus???
//	if( ! layoutUnit.getShortcut().isEmpty() ) {
//		action->setToolTip(layoutUnit.getShortcut().toString());
//	}
	return action;
}

QList<QAction*> LayoutsMenu::contextualActions()
{
	if( actionGroup ) {
		disconnect(actionGroup, &QActionGroup::triggered, this, &LayoutsMenu::actionTriggered);
		delete actionGroup;
	}
	actionGroup = new QActionGroup(this);

	X11Helper::getLayoutsList(); //UGLY: seems to be more reliable with extra call
	QList<LayoutUnit> currentLayouts = X11Helper::getLayoutsList();
	foreach(const LayoutUnit& layoutUnit, currentLayouts) {
		QAction* action = createAction(layoutUnit);
		actionGroup->addAction(action);
	}

	if( keyboardConfig.configureLayouts ) {
		QList<LayoutUnit> extraLayouts = keyboardConfig.layouts;
		foreach(const LayoutUnit& layoutUnit, currentLayouts) {
			extraLayouts.removeOne(layoutUnit);
		}
		if( extraLayouts.size() > 0 ) {
			QAction* separator = new QAction(actionGroup);
			separator->setSeparator(true);
			actionGroup->addAction(separator);

			foreach(const LayoutUnit& layoutUnit, extraLayouts) {
				QAction* action = createAction(layoutUnit);
				actionGroup->addAction(action);
			}
		}
	}

	QAction* separator = new QAction(actionGroup);
	separator->setSeparator(true);
	actionGroup->addAction(separator);
	QAction* configAction = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("Configure Layouts..."), actionGroup);
	actionGroup->addAction(configAction);
	configAction->setData("config");
	connect(actionGroup, &QActionGroup::triggered, this, &LayoutsMenu::actionTriggered);
	return actionGroup->actions();
}
