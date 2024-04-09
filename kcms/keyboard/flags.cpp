/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "flags.h"

#include <KCountryFlagEmojiIconEngine>
#include <KLocalizedString>

#include <QPainter>
#include <QPixmap>
#include <QStandardPaths>
#include <QStringList>

// for text handling
#include "keyboard_config.h"
#include "x11_helper.h"
#include "xkb_rules.h"

QIcon Flags::getIcon(const QString &layout)
{
    if (!iconMap.contains(layout)) {
        iconMap[layout] = createIcon(layout);
    }
    return iconMap[layout];
}

QIcon Flags::createIcon(const QString &layout)
{
    return QIcon(new KCountryFlagEmojiIconEngine(getCountryFromLayoutName(layout)));
}

// static
// const QStringList NON_COUNTRY_LAYOUTS = QString("ara,brai,epo,latam,mao").split(",");

QString Flags::getCountryFromLayoutName(const QString &layout) const
{
    QString countryCode = layout;

    if (countryCode == QLatin1String("nec_vndr/jp"))
        return QStringLiteral("jp");

    return countryCode;
}

// TODO: move this to some other class?

QString Flags::getShortText(const LayoutUnit &layoutUnit, const KeyboardConfig &keyboardConfig)
{
    if (layoutUnit.isEmpty())
        return QStringLiteral("--");

    QString layoutText = layoutUnit.layout();

    for (const auto layouts = keyboardConfig.layouts(); const LayoutUnit &lu : layouts) {
        if (layoutUnit.layout() == lu.layout() && layoutUnit.variant() == lu.variant()) {
            layoutText = lu.getDisplayName();
            break;
        }
    }

    // TODO: good autolabel
    //	if( layoutText == layoutUnit.layout && layoutUnit.getDisplayName() != layoutUnit.layout ) {
    //		layoutText = layoutUnit.getDisplayName();
    //	}

    return layoutText;
}

static QString getDisplayText(const QString &layout, const QString &variant, const Rules *rules)
{
    if (variant.isEmpty())
        return layout;
    if (rules == nullptr)
        return i18nc("layout - variant", "%1 - %2", layout, variant);
    return variant;
}

QString Flags::getLongText(const LayoutUnit &layoutUnit, const Rules *rules)
{
    if (rules == nullptr) {
        return getDisplayText(layoutUnit.layout(), layoutUnit.variant(), rules);
    }

    QString layoutText = layoutUnit.layout();
    const std::optional<LayoutInfo> layoutInfo = rules->getLayoutInfo(layoutUnit.layout());
    if (layoutInfo) {
        layoutText = layoutInfo->description;

        if (!layoutUnit.variant().isEmpty()) {
            const std::optional<VariantInfo> variantInfo = layoutInfo->getVariantInfo(layoutUnit.variant());
            QString variantText = variantInfo ? variantInfo->description : layoutUnit.variant();

            layoutText = getDisplayText(layoutText, variantText, rules);
        }
    }

    return layoutText;
}
