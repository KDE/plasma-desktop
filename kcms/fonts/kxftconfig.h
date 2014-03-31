#ifndef __KXFTCONFIG_H__
#define __KXFTCONFIG_H__

/*
   Copyright (c) 2002 Craig Drummond <craig@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config-workspace.h>

#ifdef HAVE_FONTCONFIG

#include <time.h>

#include <QStringList>
#include <QtXml/QtXml>

class KXftConfig
{
    public:

    struct Item
    {
        Item(QDomNode &n) : node(n), toBeRemoved(false) {}
        Item()            : toBeRemoved(false)          {}
        virtual void reset()                            { node.clear(); toBeRemoved=false; }
        bool         added()                            { return node.isNull(); }

        QDomNode node;

        virtual ~Item() {}
        bool toBeRemoved;
    };

    struct SubPixel : public Item
    {
        enum Type
        {
            None,
            Rgb,
            Bgr,
            Vrgb,
            Vbgr
        };

        SubPixel(Type t, QDomNode &n) : Item(n), type(t) {}
        SubPixel(Type t=None)         : type(t)          {}

        void reset() { Item::reset(); type=None; }

        Type type;
    };

    struct Exclude : public Item
    {
        Exclude(double f, double t, QDomNode &n) : Item(n), from(f), to(t) {}
        Exclude(double f=0, double t=0)          : from(f), to(t)          {}

        void reset() { Item::reset(); from=to=0; }

        double from,
               to;
    };

    struct Hint : public Item
    {
        enum Style
        {
            NotSet,
            None,
            Slight,
            Medium,
            Full
        };

        Hint(Style s, QDomNode &n) : Item(n), style(s) {}
        Hint(Style s=NotSet)       : style(s)          {}

        void reset() { Item::reset(); style=NotSet; }

        Style style;
    };

    struct Hinting : public Item
    {
        Hinting(bool s, QDomNode &n) : Item(n), set(s) {}
        Hinting(bool s=true)         : set(s)          {}

        void reset() { Item::reset(); set=true; }

        bool set;
    };

    struct AntiAliasing : public Item
    {
        AntiAliasing(bool s, QDomNode &n) : Item(n), set(s) {}
        AntiAliasing(bool s=true)         : set(s)          {}

        void reset() { Item::reset(); set=true; }

        bool set;
    };

    public:

    explicit KXftConfig();

    virtual ~KXftConfig();

    bool        reset();
    bool        apply();
    bool        getSubPixelType(SubPixel::Type &type);
    void        setSubPixelType(SubPixel::Type type);  // SubPixel::None => turn off sub-pixel rendering
    bool        getExcludeRange(double &from, double &to);
    void        setExcludeRange(double from, double to); // from:0, to:0 => turn off exclude range
    bool        getHintStyle(Hint::Style &style);
    void        setHintStyle(Hint::Style style);
    void        setAntiAliasing(bool set);
    bool        getAntiAliasing() const;
    bool        changed()                            { return m_madeChanges; }
    static QString description(SubPixel::Type t);
    static const char * toStr(SubPixel::Type t);
    static QString description(Hint::Style s);
    static const char * toStr(Hint::Style s);

    private:

    void        readContents();
    void        applySubPixelType();
    void        applyHintStyle();
    void        applyAntiAliasing();
    void        setHinting(bool set);
    void        applyHinting();
    void        applyExcludeRange(bool pixel);
    bool        aliasingEnabled();

    private:

    SubPixel           m_subPixel;
    Exclude            m_excludeRange,
                       m_excludePixelRange;
    Hint               m_hint;
    Hinting            m_hinting;
    AntiAliasing       m_antiAliasing;
    QDomDocument       m_doc;
    QString            m_file;
    bool               m_madeChanges;
    time_t             m_time;
};

#endif

#endif
