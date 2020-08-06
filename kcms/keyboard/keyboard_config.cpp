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

#include "keyboard_config.h"
#include "debug.h"

#include <KSharedConfig>
#include <KConfigGroup>


static const char* const SWITCHING_POLICIES[] = {"Global", "Desktop", "WinClass", "Window", nullptr };
static const char LIST_SEPARATOR[] = ",";
//static const char* DEFAULT_LAYOUT = "us";
static const char DEFAULT_MODEL[] = "pc104";

static const QString CONFIG_FILENAME(QStringLiteral("kxkbrc"));
static const QString CONFIG_GROUPNAME(QStringLiteral("Layout"));

const int KeyboardConfig::NO_LOOPING = -1;

KeyboardConfig::KeyboardConfig()
{
    setDefaults();
}

QString KeyboardConfig::getSwitchingPolicyString(SwitchingPolicy switchingPolicy) {
	return SWITCHING_POLICIES[switchingPolicy];
}

static int findStringIndex(const char* const strings[], const QString& toFind, int defaultIndex)
{
	for(int i=0; strings[i] != nullptr; i++) {
		if( toFind == strings[i] ) {
			return i;
		}
	}
	return defaultIndex;
}

void KeyboardConfig::setDefaults()
{
	keyboardModel = DEFAULT_MODEL;
	resetOldXkbOptions = false;
	xkbOptions.clear();

	// init layouts options
	configureLayouts = false;
	layouts.clear();
//	layouts.append(LayoutUnit(DEFAULT_LAYOUT));
	layoutLoopCount = NO_LOOPING;

	// switch control options
	switchingPolicy = SWITCH_POLICY_GLOBAL;
//	stickySwitching = false;
//	stickySwitchingDepth = 2;

	// display options
	showIndicator = true;
	indicatorType = SHOW_LABEL;
	showSingle = false;
}

static
KeyboardConfig::IndicatorType getIndicatorType(bool showFlag, bool showLabel)
{
	if( showFlag ) {
		if( showLabel )
			return KeyboardConfig::SHOW_LABEL_ON_FLAG;
		else
			return KeyboardConfig::SHOW_FLAG;
	}
	else {
		return KeyboardConfig::SHOW_LABEL;
	}
}


void KeyboardConfig::load()
{
    KConfigGroup config(KSharedConfig::openConfig( CONFIG_FILENAME, KConfig::NoGlobals ), CONFIG_GROUPNAME);

    keyboardModel = config.readEntry("Model", "");

    resetOldXkbOptions = config.readEntry("ResetOldOptions", false);
    QString options = config.readEntry("Options", "");
    xkbOptions = options.split(LIST_SEPARATOR, Qt::SkipEmptyParts);

    configureLayouts = config.readEntry("Use", false);
    QString layoutsString = config.readEntry("LayoutList", "");
    QStringList layoutStrings = layoutsString.split(LIST_SEPARATOR, Qt::SkipEmptyParts);
//    if( layoutStrings.isEmpty() ) {
//    	layoutStrings.append(DEFAULT_LAYOUT);
//    }
    layouts.clear();
    if (layoutStrings.isEmpty()) {
        QList<LayoutUnit> x11layouts = X11Helper::getLayoutsList();
        for (const LayoutUnit& layoutUnit : x11layouts) {
            layouts.append(layoutUnit);
        }
    } else {
        for (const QString& layoutString : layoutStrings) {
            layouts.append(LayoutUnit(layoutString));
        }
    }
    configureLayouts = !layouts.isEmpty();

    layoutLoopCount = config.readEntry("LayoutLoopCount", NO_LOOPING);

	QString layoutMode = config.readEntry("SwitchMode", "Global");
	switchingPolicy = static_cast<SwitchingPolicy>(findStringIndex(SWITCHING_POLICIES, layoutMode, SWITCH_POLICY_GLOBAL));

	showIndicator = config.readEntry("ShowLayoutIndicator", true);

	bool showFlag = config.readEntry("ShowFlag", false);
	bool showLabel = config.readEntry("ShowLabel", true);
	indicatorType = getIndicatorType(showFlag, showLabel);

	showSingle = config.readEntry("ShowSingle", false);

    QString labelsStr = config.readEntry("DisplayNames", "");
    QStringList labels = labelsStr.split(LIST_SEPARATOR, Qt::KeepEmptyParts);
    for(int i=0; i<labels.count() && i<layouts.count(); i++) {
        if( !labels[i].isEmpty() && labels[i] != layouts[i].layout() ) {
    		layouts[i].setDisplayName(labels[i]);
    	}
    }

//    QString shortcutsStr = config.readEntry("LayoutShortcuts", "");
//    QStringList shortcutsList = shortcutsStr.split(LIST_SEPARATOR, QString::KeepEmptyParts);
//    for(int i=0; i<shortcutsList.count() && i<layouts.count(); i++) {
//    	if( !shortcutsList[i].isEmpty() ) {
//    		layouts[i].setShortcut(QKeySequence(shortcutsList[i]));
//    	}
//    }

	qCDebug(KCM_KEYBOARD) << "configuring layouts" << configureLayouts << "configuring options" << resetOldXkbOptions;
}

void KeyboardConfig::save()
{
    KConfigGroup config(KSharedConfig::openConfig( CONFIG_FILENAME, KConfig::NoGlobals ), CONFIG_GROUPNAME);

    config.writeEntry("Model", keyboardModel);

    config.writeEntry("ResetOldOptions", resetOldXkbOptions);
    if( resetOldXkbOptions ) {
    	config.writeEntry("Options", xkbOptions.join(LIST_SEPARATOR));
    }
    else {
        config.deleteEntry("Options");
    }

    config.writeEntry("Use", configureLayouts);

    QStringList layoutStrings;
    QStringList displayNames;
//    QStringList shortcuts;
    foreach(const LayoutUnit& layoutUnit, layouts) {
    	layoutStrings.append(layoutUnit.toString());
    	displayNames.append(layoutUnit.getRawDisplayName());
//    	shortcuts.append(layoutUnit.getShortcut().toString());
    }
    config.writeEntry("LayoutList", layoutStrings.join(LIST_SEPARATOR));
    config.writeEntry("DisplayNames", displayNames.join(LIST_SEPARATOR));
//    config.writeEntry("LayoutShortcuts", shortcuts.join(LIST_SEPARATOR));

    config.writeEntry("LayoutLoopCount", layoutLoopCount);

	config.writeEntry("SwitchMode", SWITCHING_POLICIES[switchingPolicy]);

	config.writeEntry("ShowLayoutIndicator", showIndicator);
	config.writeEntry("ShowFlag", indicatorType == SHOW_FLAG || indicatorType == SHOW_LABEL_ON_FLAG);
	config.writeEntry("ShowLabel", indicatorType == SHOW_LABEL || indicatorType == SHOW_LABEL_ON_FLAG);
	config.writeEntry("ShowSingle", showSingle);

	config.sync();
}

QList<LayoutUnit> KeyboardConfig::getDefaultLayouts() const
{
	QList<LayoutUnit> defaultLayoutList;
	int i = 0;
	foreach(const LayoutUnit& layoutUnit, layouts) {
		defaultLayoutList.append(layoutUnit);
		if( layoutLoopCount != KeyboardConfig::NO_LOOPING && i >= layoutLoopCount-1 )
			break;
		i++;
	}
	return defaultLayoutList;
}

QList<LayoutUnit> KeyboardConfig::getExtraLayouts() const
{
	if( layoutLoopCount == KeyboardConfig::NO_LOOPING )
		return QList<LayoutUnit>();

	return layouts.mid(layoutLoopCount, layouts.size());
}
