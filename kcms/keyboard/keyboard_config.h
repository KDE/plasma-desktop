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


#ifndef KEYBOARD_CONFIG_H_
#define KEYBOARD_CONFIG_H_

#include "x11_helper.h"

#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QPair>

/**
 * This class provides configuration options for keyboard module
 */
class KeyboardConfig
{
public:
	static const int MAX_LABEL_LEN = 3;
	static const int NO_LOOPING; // = -1;

	enum SwitchingPolicy {
		SWITCH_POLICY_GLOBAL = 0,
		SWITCH_POLICY_DESKTOP = 1,
		SWITCH_POLICY_APPLICATION = 2,
		SWITCH_POLICY_WINDOW = 3
	};

	enum IndicatorType {
		SHOW_LABEL = 0,
		SHOW_FLAG = 1,
		SHOW_LABEL_ON_FLAG = 2
	};

	QString keyboardModel;
	// resetOldXkbOptions is now also "set xkb options"
	bool resetOldXkbOptions;
	QStringList xkbOptions;

	// init layouts options
	bool configureLayouts;
	QList<LayoutUnit> layouts;
	int layoutLoopCount;

	// switch control options
	SwitchingPolicy switchingPolicy;
//	bool stickySwitching;
//	int stickySwitchingDepth;

	// display options
	bool showIndicator;
	IndicatorType indicatorType;
	bool showSingle;

	KeyboardConfig();

	QList<LayoutUnit> getDefaultLayouts() const;
	QList<LayoutUnit> getExtraLayouts() const;
	bool isFlagShown() const {
		return indicatorType == SHOW_FLAG || indicatorType == SHOW_LABEL_ON_FLAG;
	}
	bool isLabelShown() const {
		return indicatorType == SHOW_LABEL || indicatorType == SHOW_LABEL_ON_FLAG;
	}

	void setDefaults();
	void load();
	void save();

	static QString getSwitchingPolicyString(SwitchingPolicy switchingPolicy);
};

#endif /* KEYBOARD_CONFIG_H_ */
