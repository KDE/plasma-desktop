/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QKeySequence>
#include <QString>
#include <QStringList>

struct XkbConfig {
    QString keyboardModel;
    QStringList layouts;
    QStringList variants;
    QStringList options;

    bool isValid()
    {
        return !layouts.empty();
    }
};

class LayoutUnit
{
public:
    static const int MAX_LABEL_LENGTH;

    LayoutUnit()
    {
    }
    explicit LayoutUnit(const QString &fullLayoutName);
    LayoutUnit(const QString &layout, const QString &variant)
        : m_layout(layout)
        , m_variant(variant)
    {
    }
    /*explicit*/ LayoutUnit(const LayoutUnit &other)
    {
        operator=(other);
    }

    LayoutUnit &operator=(const LayoutUnit &other)
    {
        if (this != &other) {
            m_layout = other.m_layout;
            m_variant = other.m_variant;
            displayName = other.displayName;
            shortcut = other.shortcut;
        }
        return *this;
    }

    QString getRawDisplayName() const
    {
        return displayName;
    }
    QString getDisplayName() const
    {
        return !displayName.isEmpty() ? displayName : m_layout;
    }
    void setDisplayName(const QString &name)
    {
        displayName = name;
    }

    void setShortcut(const QKeySequence &shortcut)
    {
        this->shortcut = shortcut;
    }
    QKeySequence getShortcut() const
    {
        return shortcut;
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
    QString displayName;
    QKeySequence shortcut;
    QString m_layout;
    QString m_variant;
};

struct LayoutSet {
    QList<LayoutUnit> layouts;
    LayoutUnit currentLayout;

    LayoutSet()
    {
    }

    LayoutSet(const LayoutSet &other)
    {
        operator=(other);
    }

    bool isValid() const
    {
        return currentLayout.isValid() && layouts.contains(currentLayout);
    }

    bool operator==(const LayoutSet &currentLayouts) const
    {
        return this->layouts == currentLayouts.layouts && this->currentLayout == currentLayouts.currentLayout;
    }

    LayoutSet &operator=(const LayoutSet &currentLayouts)
    {
        this->layouts = currentLayouts.layouts;
        this->currentLayout = currentLayouts.currentLayout;
        return *this;
    }

    QString toString() const
    {
        QString str(currentLayout.toString());
        str += QLatin1String(": ");
        for (const auto &layoutUnit : std::as_const(layouts)) {
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
