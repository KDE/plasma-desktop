/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "xkbobject.h"

XkbObject::XkbObject(XkbDescPtr xkb_, QObject *parent)
    : QObject(parent)
    , xkb(xkb_)
{
    Q_ASSERT(xkb);
}

// Converts from a named color in the xkb geometry files to a QColor:
// - Regular color names: "white", "black", "red", "blue", "green", "gray"|"grey"
// - Shades of grey/gray (both spellings): "grey20", "gray30", "gray70"
// - hex-coded #rrggbb colors: "#00ff00"
// - "green30" for some reason, as an Off Indicator color
QColor XkbObject::colorFromName(const QString &colorName) const
{
    // Check if QColor recognizes the color name
    const QColor easyColor = QColor::fromString(colorName);
    if (easyColor.isValid()) {
        return easyColor;
    }
    // Calculate any shades of grey
    if (colorName.startsWith("grey") || colorName.startsWith("gray")) {
        bool ok = false;
        const int grayPct = colorName.sliced(4).toInt(&ok);
        if (!ok) {
            return QColor("gray"); // Couldn't get percentage, but it's a gray
        }
        return QColor::fromHsvF(0.0, 0.0, 1 - grayPct / 100.0);
    }
    // green30
    if (colorName == "green30") {
        const QColor green = QColor("green");
        return QColor::fromHsvF(green.hueF(), green.saturationF(), 0.3);
    }

    // No other options here. Return invalid color
    return QColor();
}

QColor XkbObject::colorFromIndex(int index) const
{
    if (!xkb->geom) {
        return QColor();
    }
    return colorFromName(xkb->geom->colors[index].spec);
}
