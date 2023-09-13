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
    , m_referenceLayoutLoopCount(mLayoutLoopCount)
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
    if (mLayoutLoopCount != m_referenceLayoutLoopCount) {
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
    const QStringList names = displayNames();

    layouts.clear();
    for (int i = 0; i < layoutStrings.size(); ++i) {
        if (i < variants.size()) {
            layouts.append({layoutStrings[i], variants[i]});
        } else {
            layouts.append(LayoutUnit(layoutStrings[i]));
        }

        if (i < names.size() && !names[i].isEmpty() && names[i] != layouts[i].layout()) {
            layouts[i].setDisplayName(names[i]);
        }
    }

    // layouts' shortcuts are retrieved from GlobalShortcuts in KCMKeyboardWidget
    m_referenceLayouts = layouts;
    m_referenceLayoutLoopCount = mLayoutLoopCount;

    qCDebug(KCM_KEYBOARD) << "configuring layouts" << configureLayouts() << "configuring options" << resetOldXkbOptions();
}

void KeyboardConfig::save()
{
    m_referenceLayouts = layouts;
    m_referenceLayoutLoopCount = mLayoutLoopCount;

    QStringList layoutList;
    QStringList variants;
    QStringList displayNames;
    for (const LayoutUnit &layoutUnit : std::as_const(layouts)) {
        layoutList.append(layoutUnit.layout());
        variants.append(layoutUnit.variant());
        displayNames.append(layoutUnit.getRawDisplayName());
    }

    // QStringLists with a single empty string are serialized as "\\0", avoid that
    // by saving them as an empty list instead. This way it can be passed as-is to
    // libxkbcommon/setxkbmap. Before KConfigXT it used QStringList::join(",").
    if (variants.size() == 1 && variants.constFirst().isEmpty()) {
        variants.clear();
    }
    if (displayNames.size() == 1 && displayNames.constFirst().isEmpty()) {
        displayNames.clear();
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
    for (const LayoutUnit &layoutUnit : std::as_const(layouts)) {
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
