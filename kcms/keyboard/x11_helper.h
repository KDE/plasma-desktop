/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QKeySequence>
#include <QString>
#include <QStringList>

class LayoutUnit
{
public:
    static const int MAX_LABEL_LENGTH;

    explicit LayoutUnit(const QString &fullLayoutName);
    LayoutUnit(const QString &layout, const QString &variant)
        : m_layout(layout)
        , m_variant(variant)
    {
    }

    QString getRawDisplayName() const
    {
        return m_displayName;
    }
    QString getDisplayName() const
    {
        return !m_displayName.isEmpty() ? m_displayName : m_layout;
    }
    void setDisplayName(const QString &name)
    {
        m_displayName = name;
    }

    void setShortcut(const QKeySequence &shortcut)
    {
        this->m_shortcut = shortcut;
    }
    QKeySequence getShortcut() const
    {
        return m_shortcut;
    }
    QString layout() const
    {
        return m_layout;
    }
    void setLayout(const QString &layout)
    {
        m_layout = layout;
    }
    QString variant() const
    {
        return m_variant;
    }
    void setVariant(const QString &variant)
    {
        m_variant = variant;
    }

    bool isEmpty() const
    {
        return m_layout.isEmpty();
    }
    bool isValid() const
    {
        return !isEmpty();
    }
    bool operator==(const LayoutUnit &layoutItem) const
    {
        // FIXME: why not compare the other properties?
        return m_layout == layoutItem.m_layout && m_variant == layoutItem.m_variant;
    }
    bool operator!=(const LayoutUnit &layoutItem) const
    {
        return !(*this == layoutItem);
    }
    QString toString() const;

private:
    QString m_displayName;
    QKeySequence m_shortcut;
    QString m_layout;
    QString m_variant;
};

struct LayoutSet {
    QList<LayoutUnit> m_layouts;
    LayoutUnit m_currentLayout;

    bool isValid() const
    {
        return m_currentLayout.isValid() && m_layouts.contains(m_currentLayout);
    }

    bool operator==(const LayoutSet &currentLayouts) const
    {
        return this->m_layouts == currentLayouts.m_layouts && this->m_currentLayout == currentLayouts.m_currentLayout;
    }

    QString toString() const
    {
        QString str(m_currentLayout.toString());
        str += QLatin1String(": ");
        for (const auto &layoutUnit : std::as_const(m_layouts)) {
            str += layoutUnit.toString() + QLatin1Char(' ');
        }
        return str;
    }

    static QString toString(const QList<LayoutUnit> &layoutUnits)
    {
        QString str;
        for (const auto &layoutUnit : layoutUnits) {
            str += layoutUnit.toString() + QLatin1Char(',');
        }
        return str;
    }
};
