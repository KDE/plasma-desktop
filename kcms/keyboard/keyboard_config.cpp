/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keyboard_config.h"

#include "debug.h"
#include "keyboardsettings.h"
#include "x11_helper.h"
#include "xkb_rules.h"

const QMap<KeyboardConfig::SwitchingPolicy, QString> switchingPolicyNames = {
    {KeyboardConfig::SwitchingPolicy::SWITCH_POLICY_GLOBAL, QStringLiteral("Global")},
    {KeyboardConfig::SwitchingPolicy::SWITCH_POLICY_DESKTOP, QStringLiteral("Desktop")},
    {KeyboardConfig::SwitchingPolicy::SWITCH_POLICY_APPLICATION, QStringLiteral("WinClass")},
    {KeyboardConfig::SwitchingPolicy::SWITCH_POLICY_WINDOW, QStringLiteral("Window")},
};

const int KeyboardConfig::NO_LOOPING = -1;

KeyboardConfig::KeyboardConfig(KeyboardSettings *settings, QObject *parent) noexcept
    : QObject(parent)
    , m_settings(settings)
{
    QObject::connect(m_settings, &KeyboardSettings::switchModeChanged, this, &KeyboardConfig::switchingPolicyChanged);
}

KeyboardConfig::SwitchingPolicy KeyboardConfig::switchingPolicy() const
{
    return policyFromString(m_settings->switchMode());
}

void KeyboardConfig::setSwitchingPolicy(SwitchingPolicy mode)
{
    m_settings->setSwitchMode(switchingPolicyNames[mode]);
}

const QList<LayoutUnit> &KeyboardConfig::layouts() const
{
    return m_layouts;
}

QList<LayoutUnit> &KeyboardConfig::layouts()
{
    return m_layouts;
}

KeyboardSettings *KeyboardConfig::keyboardSettings() const
{
    return m_settings;
}

bool KeyboardConfig::isDefaults() const
{
    return m_settings->isDefaults() && isDefaultsLayouts();
}

bool KeyboardConfig::isSaveNeeded() const
{
    return m_settings->isSaveNeeded() || layoutsSaveNeeded();
}

QList<LayoutUnit> KeyboardConfig::getDefaultLayouts() const
{
    QList<LayoutUnit> defaultLayoutList;
    int i = 0;
    for (const LayoutUnit &layoutUnit : std::as_const(m_layouts)) {
        defaultLayoutList.append(layoutUnit);
        if (m_settings->layoutLoopCount() != KeyboardConfig::NO_LOOPING && i >= m_settings->layoutLoopCount() - 1) {
            break;
        }
        i++;
    }
    return defaultLayoutList;
}

QList<LayoutUnit> KeyboardConfig::getExtraLayouts() const
{
    if (m_settings->layoutLoopCount() == KeyboardConfig::NO_LOOPING) {
        return QList<LayoutUnit>();
    }

    return m_layouts.mid(m_settings->layoutLoopCount(), m_layouts.size());
}

QString KeyboardConfig::getSwitchingPolicyString(SwitchingPolicy switchingPolicy)
{
    return switchingPolicyNames[switchingPolicy];
}

KeyboardConfig::SwitchingPolicy KeyboardConfig::policyFromString(const QString &string) const
{
    const auto keys = switchingPolicyNames.keys();
    auto mode = std::find_if(keys.constBegin(), keys.constEnd(), [=, this](const KeyboardConfig::SwitchingPolicy &key) {
        return switchingPolicyNames[key] == string;
    });

    return *mode;
}

bool KeyboardConfig::layoutsSaveNeeded() const
{
    if (m_layouts.size() != m_referenceLayouts.size()) {
        return true;
    }
    if (m_settings->layoutLoopCount() != m_referenceLayoutLoopCount) {
        return true;
    }

    // Due to layoutUnit operator==() that does not test all properties.
    // Do not compare shortcuts, they are automatically applied
    bool isSaveNeeded = false;
    for (int i = 0; i < m_layouts.size(); ++i) {
        isSaveNeeded |= m_layouts.at(i).getDisplayName() != m_referenceLayouts.at(i).getDisplayName();
        isSaveNeeded |= m_layouts.at(i).layout() != m_referenceLayouts.at(i).layout();
        isSaveNeeded |= m_layouts.at(i).variant() != m_referenceLayouts.at(i).variant();

        if (isSaveNeeded) {
            return isSaveNeeded;
        }
    }
    return isSaveNeeded;
}

bool KeyboardConfig::isDefaultsLayouts() const
{
    return m_layouts.isEmpty();
}

void KeyboardConfig::save()
{
    m_referenceLayouts = m_layouts;
    m_referenceLayoutLoopCount = m_settings->layoutLoopCount();

    QStringList layoutList;
    QStringList variants;
    QStringList displayNames;
    for (const LayoutUnit &layoutUnit : std::as_const(m_layouts)) {
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

    m_settings->setLayoutList(layoutList);
    m_settings->setVariantList(variants);
    m_settings->setDisplayNames(displayNames);

    m_settings->save();
}

void KeyboardConfig::load()
{
    m_settings->load();

    const QStringList layoutStrings = m_settings->layoutList();
    const QStringList variants = m_settings->variantList();
    const QStringList names = m_settings->displayNames();

    m_layouts.clear();
    for (int i = 0; i < layoutStrings.size(); ++i) {
        if (i < variants.size()) {
            m_layouts.append({layoutStrings[i], variants[i]});
        } else {
            m_layouts.append(LayoutUnit(layoutStrings[i]));
        }

        if (i < names.size() && !names[i].isEmpty() && names[i] != m_layouts[i].layout()) {
            m_layouts[i].setDisplayName(names[i]);
        }
    }

    // layouts' shortcuts are retrieved from GlobalShortcuts in KCMKeyboardWidget
    m_referenceLayouts = m_layouts;
    m_referenceLayoutLoopCount = m_settings->layoutLoopCount();

    qCDebug(KCM_KEYBOARD) << "configuring layouts" << m_settings->configureLayouts() << "configuring options" << m_settings->resetOldXkbOptions();
}

void KeyboardConfig::defaults()
{
    m_layouts.clear();
    m_settings->setDefaults();
}

#include "keyboard_config.moc"
#include "moc_keyboard_config.cpp"
