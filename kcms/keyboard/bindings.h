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


#ifndef BINDINGS_H_
#define BINDINGS_H_

#include <KActionCollection>

struct Rules;
class LayoutUnit;
template <typename T> class QList;


class KeyboardLayoutActionCollection : public KActionCollection {
public:
	KeyboardLayoutActionCollection(QObject* parent, bool configAction);
	~KeyboardLayoutActionCollection() override;

	QAction* getToggleAction();
//	KAction* getAction(const LayoutUnit& layoutUnit);
	QAction* createLayoutShortcutActon(const LayoutUnit& layoutUnit, const Rules* rules, bool autoload);
//	KAction* setShortcut(LayoutUnit& layoutUnit, const QKeySequence& keySequence, const Rules* rules);
	void setLayoutShortcuts(QList<LayoutUnit>& layoutUnits, const Rules* rules);
	void setToggleShortcut(const QKeySequence& keySequence);
	void loadLayoutShortcuts(QList<LayoutUnit>& layoutUnits, const Rules* rules);
	void resetLayoutShortcuts();

private:
	bool configAction;
};

//KActionCollection* createGlobalActionCollection(QObject *parent, KAction** mainAction);
//KAction* createLayoutShortcutActon(KActionCollection* actionCollection, const LayoutUnit& layoutUnit, const Rules* rules);

#endif /* BINDINGS_H_ */
