/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keyboard_config.h"

#include "debug.h"
#include "keyboardsettings.h"
#include "layoutunit.h"

KeyboardConfig::KeyboardConfig(KeyboardSettings *settings, QObject *parent) noexcept
    : QObject(parent)
    , m_settings(settings)
{
}

const QList<LayoutUnit> &KeyboardConfig::layouts() const
{
    return m_layouts;
}

QList<LayoutUnit> &KeyboardConfig::layouts()
{
    return m_layouts;
}

void KeyboardConfig::notifyLayoutsChanged()
{
    applyLayoutsToSettings();
}

KeyboardSettings *KeyboardConfig::keyboardSettings() const
{
    return m_settings;
}

void KeyboardConfig::applyLayoutsToSettings()
{
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
}

void KeyboardConfig::resetLayouts()
{
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
}

#include "moc_keyboard_config.cpp"
