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

#ifndef LAYOUTS_MENU_H_
#define LAYOUTS_MENU_H_

#include <QtCore/QList>
#include <QtGui/QIcon>

class QAction;
class KeyboardConfig;
class Flags;
class Rules;
class QActionGroup;
class LayoutUnit;

class LayoutsMenu : public QObject
{
    Q_OBJECT

public:
	LayoutsMenu(const KeyboardConfig& keyboardConfig, const Rules& rules, Flags& flags);
	virtual ~LayoutsMenu();

	QList<QAction*> contextualActions();
	static int switchToLayout(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig);

private Q_SLOTS:
	void actionTriggered(QAction* action);

private:
	const QIcon getFlag(const QString& layout) const;
	QAction* createAction(const LayoutUnit& layoutUnit) const;

	const KeyboardConfig& keyboardConfig;
	const Rules& rules;
	Flags& flags;
    QActionGroup* actionGroup;
};

#endif /* LAYOUTS_MENU_H_ */
