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


#ifndef LAYOUT_TRAY_ICON_H_
#define LAYOUT_TRAY_ICON_H_


#include "flags.h"
#include "x11_helper.h"

class KeyboardConfig;
class LayoutsMenu;

/**
 *  System tray icon to show layouts
 */
class KStatusNotifierItem;
struct Rules;
class Flags;
class LayoutTrayIcon : public QObject
{
    Q_OBJECT

public:
    LayoutTrayIcon(const Rules* rules, const KeyboardConfig& keyboardConfig);
    ~LayoutTrayIcon() override;

    void layoutMapChanged();

public Q_SLOTS:
    void layoutChanged();

private Q_SLOTS:
	void toggleLayout();
    void scrollRequested(int, Qt::Orientation);

private:
	void init();
	void destroy();
	const QIcon getFlag(const QString& layout) const;

	const KeyboardConfig& keyboardConfig;
	const Rules* rules;
	Flags* flags;
    KStatusNotifierItem *m_notifierItem;
	LayoutsMenu* layoutsMenu;
};


#endif /* LAYOUT_WIDGET_H_ */
