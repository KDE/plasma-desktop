/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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

#ifndef KIM_AGENTTYPE_H
#define KIM_AGENTTYPE_H


typedef struct TextAttribute_ {
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
} TextAttribute;
//Q_DECLARE_METATYPE(TextAttribute);

typedef struct Property_ {
    enum State {
        None = 0,
        Active = 1,
        Visible = (1<<1)
    };
    Q_DECLARE_FLAGS(States,State)

    QString key;
    QString label;
    QString icon;
    QString tip;
    States state;
} Property;
Q_DECLARE_OPERATORS_FOR_FLAGS(Property::States)
//Q_DECLARE_METATYPE(Property);

typedef struct LookupTable_ {
    typedef struct Entry_{
        QString label;
        QString text;
        QList<TextAttribute> attr;
    } Entry;
    QList<Entry> entries;
    bool has_prev;
    bool has_next;
} LookupTable;
//Q_DECLARE_METATYPE(LookupTable);
//Q_DECLARE_METATYPE(LookupTable::Entry);

#endif // KIM_AGENTTYPE_H
