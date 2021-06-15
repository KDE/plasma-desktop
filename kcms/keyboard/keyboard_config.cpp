/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keyboard_config.h"
#include "debug.h"

#include <KConfigGroup>
#include <KSharedConfig>

static const char *const SWITCHING_POLICIES[] = {"Global", "Desktop", "WinClass", "Window", nullptr};
static const char LIST_SEPARATOR[] = ",";
// static const char* DEFAULT_LAYOUT = "us";
static const char DEFAULT_MODEL[] = "pc104";

static const QString CONFIG_FILENAME(QStringLiteral("kxkbrc"));
static const QString CONFIG_GROUPNAME(QStringLiteral("Layout"));

const int KeyboardConfig::NO_LOOPING = -1;

KeyboardConfig::KeyboardConfig()
{
    setDefaults();
}

QString KeyboardConfig::getSwitchingPolicyString(SwitchingPolicy switchingPolicy)
{
    return SWITCHING_POLICIES[switchingPolicy];
}

static int findStringIndex(const char *const strings[], const QString &toFind, int defaultIndex)
{
    for (int i = 0; strings[i] != nullptr; i++) {
        if (toFind == strings[i]) {
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
}

void KeyboardConfig::load()
{
    KConfigGroup config(KSharedConfig::openConfig(CONFIG_FILENAME, KConfig::NoGlobals), CONFIG_GROUPNAME);

    keyboardModel = config.readEntry("Model", "");

    resetOldXkbOptions = config.readEntry("ResetOldOptions", false);
    QString options = config.readEntry("Options", "");
    xkbOptions = options.split(LIST_SEPARATOR, Qt::SkipEmptyParts);

    configureLayouts = config.readEntry("Use", false);
    const QStringList layoutStrings = config.readEntry("LayoutList", QStringList());
    const QStringList variants = config.readEntry("VariantList", QStringList());
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

    layoutLoopCount = config.readEntry("LayoutLoopCount", NO_LOOPING);

    QString layoutMode = config.readEntry("SwitchMode", "Global");
    switchingPolicy = static_cast<SwitchingPolicy>(findStringIndex(SWITCHING_POLICIES, layoutMode, SWITCH_POLICY_GLOBAL));

    QString labelsStr = config.readEntry("DisplayNames", "");
    QStringList labels = labelsStr.split(LIST_SEPARATOR, Qt::KeepEmptyParts);
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
    KConfigGroup config(KSharedConfig::openConfig(CONFIG_FILENAME, KConfig::NoGlobals), CONFIG_GROUPNAME);

    config.writeEntry("Model", keyboardModel);

    config.writeEntry("ResetOldOptions", resetOldXkbOptions);
    if (resetOldXkbOptions) {
        config.writeEntry("Options", xkbOptions.join(LIST_SEPARATOR));
    } else {
        config.deleteEntry("Options");
    }

    config.writeEntry("Use", configureLayouts);

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

    auto cleanTail = [](QStringList &list) {
        // we need trailing comma in case of multiple layouts but only one variant,
        // see https://github.com/xkbcommon/libxkbcommon/issues/208
        while (list.size() > 2 && list.constLast().isEmpty()) {
            list.removeLast();
        }
    };
    cleanTail(variants);
    cleanTail(displayNames);

    config.writeEntry("LayoutList", layoutStrings.join(LIST_SEPARATOR));
    config.writeEntry("VariantList", variants.join(LIST_SEPARATOR));
    config.writeEntry("DisplayNames", displayNames.join(LIST_SEPARATOR));

    config.writeEntry("LayoutLoopCount", layoutLoopCount);

    config.writeEntry("SwitchMode", SWITCHING_POLICIES[switchingPolicy]);

    config.sync();
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
