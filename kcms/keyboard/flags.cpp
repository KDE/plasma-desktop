/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "flags.h"

#include <KLocalizedString>

#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QStandardPaths>
#include <QStringList>

#include <math.h>

#include "x11_helper.h"

// for text handling
#include "keyboard_config.h"
#include "xkb_rules.h"

static const char flagTemplate[] = "kf5/locale/countries/%1/flag.png";

int iconSize(int s)
{
    if (s < 16) {
        return 16;
    } else if (s < 22) {
        return 22;
    } else if (s < 32) {
        return 32;
    } else if (s < 48) {
        return 48;
    } else if (s < 64) {
        return 64;
    } else {
        return 128;
    }
}

const QIcon Flags::getIcon(const QString &layout)
{
    if (!iconMap.contains(layout)) {
        iconMap[layout] = createIcon(layout);
    }
    return iconMap[layout];
}

QIcon Flags::createIcon(const QString &layout)
{
    QIcon icon;
    if (!layout.isEmpty()) {
        QString file;
        if (layout == QLatin1String("epo")) {
            file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kcmkeyboard/pics/epo.png"));
        } else {
            QString countryCode = getCountryFromLayoutName(layout);
            if (!countryCode.isEmpty()) {
                file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString(flagTemplate).arg(countryCode));
                //			qCDebug(KCM_KEYBOARD, ) << "Creating icon for" << layout << "with" << file;
            }
        }

        if (!file.isEmpty()) {
            QImage flagImg;
            flagImg.load(file);
            const int size = iconSize(qMax(flagImg.width(), flagImg.height()));
            QPixmap iconPix(size, size);
            iconPix.fill(Qt::transparent);
            QRect dest(flagImg.rect());
            dest.moveCenter(iconPix.rect().center());

            QPainter painter(&iconPix);
            painter.drawImage(dest, flagImg);
            painter.end();

            icon.addPixmap(iconPix);
        }
    }
    return icon;
}

// static
// const QStringList NON_COUNTRY_LAYOUTS = QString("ara,brai,epo,latam,mao").split(",");

QString Flags::getCountryFromLayoutName(const QString &layout) const
{
    QString countryCode = layout;

    if (countryCode == QLatin1String("nec_vndr/jp"))
        return QStringLiteral("jp");

    //	if( NON_COUNTRY_LAYOUTS.contain(layout) )
    if (countryCode.length() > 2)
        return QLatin1String("");

    return countryCode;
}

// TODO: move this to some other class?

QString Flags::getShortText(const LayoutUnit &layoutUnit, const KeyboardConfig &keyboardConfig)
{
    if (layoutUnit.isEmpty())
        return QStringLiteral("--");

    QString layoutText = layoutUnit.layout();

    for (const LayoutUnit &lu : keyboardConfig.layouts) {
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
    if (rules == nullptr || rules->version == QLatin1String("1.0"))
        return i18nc("layout - variant", "%1 - %2", layout, variant);
    return variant;
}

QString Flags::getLongText(const LayoutUnit &layoutUnit, const Rules *rules)
{
    if (rules == nullptr) {
        return getDisplayText(layoutUnit.layout(), layoutUnit.variant(), rules);
    }

    QString layoutText = layoutUnit.layout();
    const LayoutInfo *layoutInfo = rules->getLayoutInfo(layoutUnit.layout());
    if (layoutInfo != nullptr) {
        layoutText = layoutInfo->description;

        if (!layoutUnit.variant().isEmpty()) {
            const VariantInfo *variantInfo = layoutInfo->getVariantInfo(layoutUnit.variant());
            QString variantText = variantInfo != nullptr ? variantInfo->description : layoutUnit.variant();

            layoutText = getDisplayText(layoutText, variantText, rules);
        }
    }

    return layoutText;
}
