/*
 *  Copyright (C) 2013 Shivam Makkar (amourphious1992@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "symbol_parser.h"
#include "xkb_rules.h"

#include <QString>
#include <QStringList>
#include <QDebug>
#include <QFileDialog>
#include <QFile>


namespace grammar
{

symbol_keywords::symbol_keywords()
{
    add("key", 2)("include", 1)("//", 3)("*/", 4);
}

levels::levels()
{
    add("ONE", 1)("TWO", 2)("THREE", 3)("FOUR", 4)("SIX", 6)("EIGHT", 8);
}

template<typename Iterator>
SymbolParser<Iterator>::SymbolParser() :
    SymbolParser::base_type(start)
{
    using qi::lexeme;
    using qi::char_;
    using qi::lit;
    using qi::_1;
    using qi::_val;
    using qi::int_;
    using qi::double_;
    using qi::eol;

    newKey = 0;

    name %= '"' >> +(char_ - '"') >> '"';

    group = lit("Group") >> int_;

    comments = lexeme[lit("//") >> *(char_ - eol || symbolKeyword - eol) >> eol
                      || lit("/*") >> *(char_ - lit("*/") || symbolKeyword - lit("*/"))
                      >> lit("*/")];

    include = lit("include")
              >> name[phx::bind(&SymbolParser::getInclude, this, _1)];

    type = lit("type") >> '[' >> group >> lit(']') >> lit('=') >> lit("\"")
           >> *(char_ - lvl)
           >> *lvl[phx::bind(&SymbolParser::setLevel, this, _1)]
           >> *(char_ - lvl - '"') >> lit("\"");

    symbol = +(char_ - ',' - ']');

    symbols = *(lit("symbols") >> '[' >> group >> lit(']') >> lit('=')) >> '['
              >> symbol[phx::bind(&SymbolParser::getSymbol, this, _1)]
              >> *(',' >> symbol[phx::bind(&SymbolParser::getSymbol, this, _1)])
              >> ']';

    keyName = '<' >> *(char_ - '>') >> '>';

    key = (lit("key")
           >> keyName[phx::bind(&SymbolParser::addKeyName, this, _1)] >> '{'
           >> *(type >> ',') >> symbols >> *(',' >> type) >> lit("};"))
          || lit("key") >> lit(".") >> type >> lit(";");

    ee = *(char_ - symbolKeyword - '{') >> '{' >> *(char_ - '}' - ';')
         >> lit("};");

    start = *(char_ - lit("xkb_symbols") || comments) >> lit("xkb_symbols")
            >> name[phx::bind(&SymbolParser::setName, this, _1)] >> '{'
            >> *(key[phx::bind(&SymbolParser::addKey, this)] || include || ee
                 || char_ - '}' - symbolKeyword || comments) >> lit("};")
            >> *(comments || char_);
}

template<typename Iterator>
void SymbolParser<Iterator>::getSymbol(std::string n)
{
    int index = layout.keyList[keyIndex].getSymbolCount();
    layout.keyList[keyIndex].addSymbol(QString::fromUtf8(n.data(), n.size()),
                                       index);
    //qCDebug(KEYBOARD_PREVIEW) << "adding symbol: " << QString::fromUtf8(n.data(), n.size());
    //qCDebug(KEYBOARD_PREVIEW) << "added symbol: " << layout.keyList[keyIndex].getSymbol(index) << " in " << keyIndex << " at " << index;
}

template<typename Iterator>
void SymbolParser<Iterator>::addKeyName(std::string n)
{
    QString kname = QString::fromUtf8(n.data(), n.size());
    if (kname.startsWith("Lat")) {
        kname = alias.getAlias(layout.country, kname);
    }
    keyIndex = layout.findKey(kname);
    //qCDebug(KEYBOARD_PREVIEW) << layout.getKeyCount();
    if (keyIndex == -1) {
        layout.keyList[layout.getKeyCount()].keyName = kname;
        keyIndex = layout.getKeyCount();
        newKey = 1;
    }
    // qCDebug(KEYBOARD_PREVIEW) << "key at" << keyIndex;
}

template<typename Iterator>
void SymbolParser<Iterator>::addKey()
{
    if (newKey == 1) {
        layout.addKey();
        newKey = 0;
        //qCDebug(KEYBOARD_PREVIEW) << "new key";
    }
}

template<typename Iterator>
void SymbolParser<Iterator>::getInclude(std::string n)
{
    layout.addInclude(QString::fromUtf8(n.data(), n.size()));
}

template<typename Iterator>
void SymbolParser<Iterator>::setName(std::string n)
{
    layout.setName(QString::fromUtf8(n.data(), n.size()));
    //qCDebug(KEYBOARD_PREVIEW) << layout.getLayoutName();
}

template<typename Iterator>
void SymbolParser<Iterator>::setLevel(int lvl)
{
    if (lvl > layout.getLevel()) {
        layout.setLevel(lvl);
        qCDebug(KEYBOARD_PREVIEW) << lvl;
    }
}

QString findSymbolBaseDir()
{
    QString xkbDir = Rules::findXkbDir();
    return QString("%1/symbols/").arg(xkbDir);
}

QString findLayout(const QString &layout, const QString &layoutVariant)
{

    QString symbolBaseDir = findSymbolBaseDir();
    QString symbolFile = symbolBaseDir.append(layout);

    QFile sfile(symbolFile);
    if (!sfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qCDebug(KEYBOARD_PREVIEW) << "unable to open the file";
        return QString("I/O ERROR");
    }

    QString scontent = sfile.readAll();
    sfile.close();
    QStringList scontentList = scontent.split("xkb_symbols");

    QString variant;
    QString input;

    if (layoutVariant.isEmpty()) {
        input = scontentList.at(1);
        input.prepend("xkb_symbols");
    }

    else {
        int current = 1;

        while (layoutVariant != variant && current < scontentList.size()) {
            input = scontentList.at(current);

            QString symbolCont = scontentList.at(current);

            int index = symbolCont.indexOf("\"");
            symbolCont = symbolCont.mid(index);
            index = symbolCont.indexOf("{");
            symbolCont = symbolCont.left(index);
            symbolCont = symbolCont.remove(" ");
            variant = symbolCont.remove("\"");

            input.prepend("xkb_symbols");
            current++;
        }
    }

    return input;
}

KbLayout parseSymbols(const QString &layout, const QString &layoutVariant)
{

    using boost::spirit::iso8859_1::space;
    typedef std::string::const_iterator iterator_type;
    typedef grammar::SymbolParser<iterator_type> SymbolParser;

    SymbolParser symbolParser;

    symbolParser.layout.country = layout;
    QString input = findLayout(layout, layoutVariant);

    if (input == "I/O ERROR") {
        symbolParser.layout.setParsedSymbol(false);
        return symbolParser.layout;
    }

    std::string parserInput = input.toUtf8().constData();

    std::string::const_iterator iter = parserInput.begin();
    std::string::const_iterator end = parserInput.end();

    bool success = phrase_parse(iter, end, symbolParser, space);

    if (success && iter == end) {
        qCDebug(KEYBOARD_PREVIEW) << "Symbols Parsing succeeded";
        symbolParser.layout.setParsedSymbol(true);

    } else {
        qWarning() << "Symbols Parsing failed\n" << input;
        symbolParser.layout.setParsedSymbol(false);
    }

    for (int currentInclude = 0;
            currentInclude < symbolParser.layout.getIncludeCount();
            currentInclude++) {
        QString include = symbolParser.layout.getInclude(currentInclude);
        QStringList includeFile = include.split("(");
        if (includeFile.size() == 2) {
            QString file = includeFile.at(0);
            QString layout = includeFile.at(1);
            layout.remove(")");
            input = findLayout(file, layout);

        }

        else {
            QString a;
            a.clear();
            input = findLayout(includeFile.at(0), a);
        }

        parserInput = input.toUtf8().constData();

        std::string::const_iterator iter = parserInput.begin();
        std::string::const_iterator end = parserInput.end();

        success = phrase_parse(iter, end, symbolParser, space);

        if (success && iter == end) {
            qCDebug(KEYBOARD_PREVIEW) << "Symbols Parsing succeeded";
            symbolParser.layout.setParsedSymbol(true);

        } else {
            qCDebug(KEYBOARD_PREVIEW) << "Symbols Parsing failed\n";
            qCDebug(KEYBOARD_PREVIEW) << input;
            symbolParser.layout.setParsedSymbol(false);
        }

    }

    //s.layout.display();
    if (symbolParser.layout.getParsedSymbol()) {
        return symbolParser.layout;
    } else {
        return parseSymbols("us", "basic");
    }
}

}
