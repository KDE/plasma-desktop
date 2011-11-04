/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
 *   Copyright (C) 2011 by CSSlayer <wengxt@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef KIMPANEL_AGENTTYPE_H
#define KIMPANEL_AGENTTYPE_H

// Qt
#include <QString>
#include <QList>
#include <QVariant>

struct TextAttribute {
    enum Type {
        None,
        Decorate,
        Foreground,
        Background
    };
    Type type;
    int start;
    int length;
    int value;
};

struct KimpanelProperty {
    enum State {
        None = 0,
        Active = 1,
        Visible = (1 << 1)
    };
    Q_DECLARE_FLAGS(States, State)

    KimpanelProperty() { }

    KimpanelProperty(QString key, QString label, QString icon, QString tip, int state) {
        this->key = key;
        this->label = label;
        this->tip = tip;
        this->icon = icon;
        this->state = (State) state;
    }

    QString key;
    QString label;
    QString icon;
    QString tip;
    States state;

    QVariantMap toMap() const {
        QVariantMap map;
        map["key"] = key;
        map["label"] = label;
        map["icon"] = icon;
        map["tip"] = tip;
        map["state"] = (int) state;
        return map;
    }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KimpanelProperty::States)

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