/*
    SPDX-FileCopyrightText: 2009 Wang Hoi <zealot.hoi@gmail.com>
    SPDX-FileCopyrightText: 2011 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KIMPANEL_AGENTTYPE_H
#define KIMPANEL_AGENTTYPE_H

// Qt
#include <QList>
#include <QString>
#include <QVariant>

struct TextAttribute {
    enum Type {
        None,
        Decorate,
        Foreground,
        Background,
    };
    Type type;
    int start;
    int length;
    int value;
};

struct KimpanelProperty {
    KimpanelProperty() = default;

    KimpanelProperty(QString key, QString label, QString icon, QString tip, QString hint)
    {
        this->key = key;
        this->label = label;
        this->tip = tip;
        this->icon = icon;
        this->hint = hint;
    }

    QString key;
    QString label;
    QString icon;
    QString tip;
    QString hint;

    QVariantMap toMap() const
    {
        QVariantMap map;
        map[QStringLiteral("key")] = key;
        map[QStringLiteral("label")] = label;
        map[QStringLiteral("icon")] = icon;
        map[QStringLiteral("tip")] = tip;
        map[QStringLiteral("hint")] = hint;
        return map;
    }
};

struct KimpanelLookupTable {
    struct Entry {
        QString label;
        QString text;
        QList<TextAttribute> attr;
    };

    QList<Entry> entries;
    bool has_prev;
    bool has_next;
};

#endif // KIMPANEL_AGENTTYPE_H
