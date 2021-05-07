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

static const char *const SWITCHING_POLICIES[] = {"Global", "Desktop", "WinClass", "Window", nullptr};

const int KeyboardConfig::NO_LOOPING = -1;

KeyboardConfig::KeyboardConfig()
{
    setDefaults();
}

QString KeyboardConfig::getSwitchingPolicyString(KeyboardSettings::EnumSwitchMode::type switchingPolicy)
{
    return SWITCHING_POLICIES[switchingPolicy];
}

void KeyboardConfig::setDefaults()
{
    KeyboardSettings config;

    keyboardModel = config.defaultModelValue();
    resetOldXkbOptions = config.defaultResetOldOptionsValue();
    xkbOptions.clear();

    // init layouts options
    configureLayouts = config.defaultUseValue();
    layouts.clear();
    //	layouts.append(LayoutUnit(DEFAULT_LAYOUT));
    layoutLoopCount = config.defaultLayoutLoopCountValue();

    // switch control options
    switchingPolicy = config.defaultSwitchModeValue();
    //	stickySwitching = false;
    //	stickySwitchingDepth = 2;

    // display options
    showIndicator = config.defaultShowLayoutIndicatorValue();
    indicatorType = SHOW_LABEL;
    showSingle = config.defaultShowSingleValue();
}

static KeyboardConfig::IndicatorType getIndicatorType(bool showFlag, bool showLabel)
{
    if (showFlag) {
        if (showLabel)
            return KeyboardConfig::SHOW_LABEL_ON_FLAG;
        else
            return KeyboardConfig::SHOW_FLAG;
    } else {
        return KeyboardConfig::SHOW_LABEL;
    }
}

void KeyboardConfig::load()
{
    KeyboardSettings config;

    keyboardModel = config.model();

    resetOldXkbOptions = config.resetOldOptions();
    xkbOptions = config.options();

    configureLayouts = config.use();
    const QStringList layoutStrings = config.layoutList();
    const QStringList variants = config.variantList();
    layouts.clear();
    if (layoutStrings.isEmpty()) {
        QList<LayoutUnit> x11layouts = X11Helper::getLayoutsList();
        for (const LayoutUnit &layoutUnit : x11layouts) {
            layouts.append(layoutUnit);
        }
    } else {
        QStringList::ConstIterator layout = layoutStrings.begin();
        const int range = qMin(layoutStrings.size(), variants.size());
        for (int i = 0; i < range; ++i) {
            layouts.append({*layout++, variants.at(i)});
        }
        while (layout != layoutStrings.end()) {
            layouts.append(LayoutUnit(*layout++));
        }
    }
    configureLayouts = !layouts.isEmpty();

    layoutLoopCount = config.layoutLoopCount();

    auto layoutMode = config.switchMode();
    switchingPolicy = layoutMode;

    showIndicator = config.showLayoutIndicator();

    bool showFlag = config.showFlag();
    bool showLabel = config.showLabel();
    indicatorType = getIndicatorType(showFlag, showLabel);

    showSingle = config.showSingle();

    QStringList labels = config.displayNames();
    for (int i = 0; i < labels.count() && i < layouts.count(); i++) {
        if (!labels[i].isEmpty() && labels[i] != layouts[i].layout()) {
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
    KeyboardSettings config;

    config.setModel(keyboardModel);
    config.setResetOldOptions(resetOldXkbOptions);
    if (resetOldXkbOptions) {
        config.setOptions(xkbOptions);
    } else {
        config.setOptions({});
    }

    config.setUse(configureLayouts);

    QStringList layoutStrings;
    QStringList variants;
    QStringList displayNames;
    //    QStringList shortcuts;
    for (const LayoutUnit &layoutUnit : qAsConst(layouts)) {
        layoutStrings.append(layoutUnit.layout());
        variants.append(layoutUnit.variant());
        displayNames.append(layoutUnit.getRawDisplayName());
        //    	shortcuts.append(layoutUnit.getShortcut().toString());
    }

    config.setLayoutList(layoutStrings);
    config.setVariantList(variants);
    config.setDisplayNames(displayNames);
    config.setLayoutLoopCount(layoutLoopCount);
    config.setSwitchMode(switchingPolicy);
    config.setShowLayoutIndicator(showIndicator);
    config.setShowFlag(indicatorType == SHOW_FLAG || indicatorType == SHOW_LABEL_ON_FLAG);
    config.setShowLabel(indicatorType == SHOW_LABEL || indicatorType == SHOW_LABEL_ON_FLAG);
    config.setShowSingle(showSingle);

    config.save();
}

QList<LayoutUnit> KeyboardConfig::getDefaultLayouts() const
{
    QList<LayoutUnit> defaultLayoutList;
    int i = 0;
    for (const LayoutUnit &layoutUnit : qAsConst(layouts)) {
        defaultLayoutList.append(layoutUnit);
        if (layoutLoopCount != KeyboardConfig::NO_LOOPING && i >= layoutLoopCount - 1)
            break;
        i++;
    }
    return defaultLayoutList;
}

QList<LayoutUnit> KeyboardConfig::getExtraLayouts() const
{
    if (layoutLoopCount == KeyboardConfig::NO_LOOPING)
        return QList<LayoutUnit>();

    return layouts.mid(layoutLoopCount, layouts.size());
}
