/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keyboard_config.h"
#include "debug.h"

#include <KConfigGroup>
#include <KSharedConfig>

static const QStringList SWITCHING_POLICIES = {QStringLiteral("Global"), QStringLiteral("Desktop"), QStringLiteral("WinClass"), QStringLiteral("Window")};
const int KeyboardConfig::NO_LOOPING = -1;

static KeyboardConfig::SwitchingPolicy findStringIndex(const QString &toFind, KeyboardConfig::SwitchingPolicy defaultPolicy)
{
    const int index = SWITCHING_POLICIES.indexOf(toFind);
    if (index < 0) {
        return defaultPolicy;
    }
    return static_cast<KeyboardConfig::SwitchingPolicy>(index);
}

KeyboardConfig::KeyboardConfig(QObject *parent)
    : KeyboardSettingsBase(parent)
{
    layouts.clear();
}

KeyboardConfig::SwitchingPolicy KeyboardConfig::switchingPolicy() const
{
    return findStringIndex(switchMode(), SWITCH_POLICY_GLOBAL);
}

void KeyboardConfig::setSwitchingPolicy(KeyboardConfig::SwitchingPolicy switchingPolicy)
{
    setSwitchMode(SWITCHING_POLICIES.at(switchingPolicy));
}

KeyboardConfig::SwitchingPolicy KeyboardConfig::defaultSwitchingPolicyValue() const
{
    return findStringIndex(defaultSwitchModeValue(), SWITCH_POLICY_GLOBAL);
}

bool KeyboardConfig::layoutsSaveNeeded() const
{
    if (layouts.size() != m_referenceLayouts.size()) {
        return true;
    }
    // Due to layoutUnit operator==() that does not test all properties.
    // Do not compare shortcuts, they are automatically applied
    bool isSaveNeeded = false;
    for (int i = 0; i < layouts.size(); ++i) {
        isSaveNeeded |= layouts.at(i).getDisplayName() != m_referenceLayouts.at(i).getDisplayName();
        isSaveNeeded |= layouts.at(i).layout() != m_referenceLayouts.at(i).layout();
        isSaveNeeded |= layouts.at(i).variant() != m_referenceLayouts.at(i).variant();
        if (isSaveNeeded) {
            return isSaveNeeded;
        }
    }
    return isSaveNeeded;
}

QString KeyboardConfig::getSwitchingPolicyString(SwitchingPolicy switchingPolicy)
{
    return SWITCHING_POLICIES.at(switchingPolicy);
}

void KeyboardConfig::setDefaults()
{
    layouts.clear();
}

void KeyboardConfig::load()
{
    KeyboardSettingsBase::load();

    const QStringList layoutStrings = layoutList();
    const QStringList variants = variantList();

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

    for (int i = 0; i < displayNames().count() && i < layouts.count(); i++) {
        if (!displayNames().at(i).isEmpty() && displayNames().at(i) != layouts[i].layout()) {
            layouts[i].setDisplayName(displayNames().at(i));
        }
    }

    // layouts' shortcuts are retrieved from GlobalShortcuts in KCMKeyboardWidget
    m_referenceLayouts = layouts;

    qCDebug(KCM_KEYBOARD) << "configuring layouts" << configureLayouts() << "configuring options" << xkbOptions();
}

void KeyboardConfig::save()
{
    m_referenceLayouts = layouts;

    QStringList layoutList;
    QStringList variants;
    QStringList displayNames;
    for (const LayoutUnit &layoutUnit : qAsConst(layouts)) {
        layoutList.append(layoutUnit.layout());
        variants.append(layoutUnit.variant());
        displayNames.append(layoutUnit.getRawDisplayName());
    }

    setLayoutList(layoutList);
    setVariantList(variants);
    setDisplayNames(displayNames);

    KeyboardSettingsBase::save();
}

QList<LayoutUnit> KeyboardConfig::getDefaultLayouts() const
{
    QList<LayoutUnit> defaultLayoutList;
    int i = 0;
    for (const LayoutUnit &layoutUnit : qAsConst(layouts)) {
        defaultLayoutList.append(layoutUnit);
        if (layoutLoopCount() != KeyboardConfig::NO_LOOPING && i >= layoutLoopCount() - 1) {
            break;
        }
        i++;
    }
    return defaultLayoutList;
}

QList<LayoutUnit> KeyboardConfig::getExtraLayouts() const
{
    if (layoutLoopCount() == KeyboardConfig::NO_LOOPING) {
        return QList<LayoutUnit>();
    }

    return layouts.mid(layoutLoopCount(), layouts.size());
}
