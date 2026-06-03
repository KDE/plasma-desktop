/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "x11_helper.h"

#include <QDebug>

static const char LAYOUT_VARIANT_SEPARATOR_PREFIX[] = "(";
static const char LAYOUT_VARIANT_SEPARATOR_SUFFIX[] = ")";

static QString &stripVariantName(QString &variant)
{
    if (variant.endsWith(LAYOUT_VARIANT_SEPARATOR_SUFFIX)) {
        int suffixLen = strlen(LAYOUT_VARIANT_SEPARATOR_SUFFIX);
        return variant.remove(variant.length() - suffixLen, suffixLen);
    }
    return variant;
}

LayoutUnit::LayoutUnit(const QString &fullLayoutName)
{
    QStringList lv = fullLayoutName.split(LAYOUT_VARIANT_SEPARATOR_PREFIX);
    m_layout = lv[0];
    m_variant = lv.size() > 1 ? stripVariantName(lv[1]) : QString();
}

QString LayoutUnit::toString() const
{
    if (m_variant.isEmpty())
        return m_layout;

    return m_layout + LAYOUT_VARIANT_SEPARATOR_PREFIX + m_variant + LAYOUT_VARIANT_SEPARATOR_SUFFIX;
}

const int LayoutUnit::MAX_LABEL_LENGTH = 3;

#include "moc_x11_helper.cpp"
