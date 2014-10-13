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

#include "geometry_parser.h"
#include "geometry_components.h"
#include "xkb_rules.h"

#include <QString>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QPair>


#include <fixx11h.h>
#include <config-workspace.h>

namespace grammar
{
keywords::keywords()
{
    add
    ("shape", 1)
    ("height", 2)
    ("width", 3)
    ("description", 4)
    ("keys", 5)
    ("row", 6)
    ("section", 7)
    ("key", 8)
    ("//", 9)
    ("/*", 10)
    ;
}


template<typename Iterator>
GeometryParser<Iterator>::GeometryParser(): GeometryParser::base_type(start)
{

    using qi::lexeme;
    using qi::char_;
    using qi::lit;
    using qi::_1;
    using qi::_val;
    using qi::int_;
    using qi::double_;
    using qi::eol;


    name = '"' >> +(char_ - '"') >> '"';

    ignore = (lit("outline") || lit("overlay") || lit("text")) >> *(char_ - lit("};")) >> lit("};")
             || lit("solid") >> *(char_ - lit("};")) >> lit("};")
             || lit("indicator") >> *(char_ - ';' - '{') >> ';' || '{' >> *(char_ - lit("};")) >> lit("};")
             || lit("indicator") >> '.' >> lit("shape") >> '=' >> name >> ';';

    comments = lexeme[ lit("//") >> *(char_ - eol || keyword - eol) >> eol || lit("/*") >> *(char_ - lit("*/") || keyword - lit("*/")) >> lit("*/") ];

    cordinates = ('['
                  >> double_[phx::ref(shapeLenX) = _1]
                  >> ','
                  >> double_[phx::ref(shapeLenY) = _1]
                  >> ']')
                 || '[' >> double_ >> "," >> double_ >> ']'
                 ;

    cordinatea = '[' >> double_[phx::ref(approxLenX) = _1] >> "," >> double_[phx::ref(approxLenY) = _1] >> ']';

    set = '{' >> cordinates >> *(',' >> cordinates) >> '}';

    setap = '{' >> cordinatea >> *(',' >> cordinatea) >> '}';

    seta = '{'
           >> cordinates[phx::bind(&GeometryParser::setCord, this)]
           >> *(',' >> cordinates[phx::bind(&GeometryParser::setCord, this)])
           >> '}'
           ;

    description = lit("description") >> '=' >> name[phx::bind(&GeometryParser::getDescription, this, _1)] >> ';';

    cornerRadius = (lit("cornerRadius") || lit("corner")) >> '=' >> double_;

    shapeDef = lit("shape")
               >> name[phx::bind(&GeometryParser::getShapeName, this, _1)]
               >> '{'
               >> *(lit("approx") >> '=' >> setap[phx::bind(&GeometryParser::setApprox, this)] >> ',' || cornerRadius >> ',' || comments)
               >> seta
               >> *((',' >> (set || lit("approx") >> '=' >> setap[phx::bind(&GeometryParser::setApprox, this)] || cornerRadius) || comments))
               >> lit("};")
               ;

    keyName = '<' >> +(char_ - '>') >> '>';

    keyShape = *(lit("key.")) >> lit("shape") >> '=' >> name[phx::bind(&GeometryParser::setKeyShape, this, _1)]
               || name[phx::bind(&GeometryParser::setKeyShape, this, _1)];

    keyColor = lit("color") >> '=' >> name;

    keygap = lit("gap") >> '=' >> double_[phx::ref(KeyOffset) = _1] || double_[phx::ref(KeyOffset) = _1];

    keyDesc = keyName[phx::bind(&GeometryParser::setKeyNameandShape, this, _1)]
              || '{' >> (keyName[phx::bind(&GeometryParser::setKeyNameandShape, this, _1)] || keyShape
                         || keygap[phx::bind(&GeometryParser::setKeyOffset, this)]
                         || keyColor)
              >> *((','
                    >> (keyName
                        || keyShape
                        || keygap[phx::bind(&GeometryParser::setKeyOffset, this)]
                        || keyColor))
                   || comments)
              >> '}';

    keys = lit("keys")
           >> '{'
           >> keyDesc[phx::bind(&GeometryParser::setKeyCordi, this)]
           >> *((*lit(',') >> keyDesc[phx::bind(&GeometryParser::setKeyCordi, this)] >> *lit(',')) || comments)
           >> lit("};");

    geomShape = ((lit("key.shape") >> '=' >> name[phx::bind(&GeometryParser::setGeomShape, this, _1)]) || (lit("key.color") >> '=' >> name)) >> ';';
    geomLeft = lit("section.left") >> '=' >> double_[phx::ref(geom.sectionLeft) = _1] >> ';';
    geomTop = lit("section.top") >> '=' >> double_[phx::ref(geom.sectionTop) = _1] >> ';';
    geomRowTop = lit("row.top") >> '=' >> double_[phx::ref(geom.rowTop) = _1] >> ';';
    geomRowLeft = lit("row.left") >> '=' >> double_[phx::ref(geom.rowLeft) = _1] >> ';';
    geomGap = lit("key.gap") >> '=' >> double_[phx::ref(geom.keyGap) = _1] >> ';';
    geomVertical = *lit("row.") >> lit("vertical") >> '=' >> (lit("True") || lit("true")) >> ';';
    geomAtt = geomLeft || geomTop || geomRowTop || geomRowLeft || geomGap;

    top = lit("top") >> '=' >> double_ >> ';';
    left = lit("left") >> '=' >> double_ >> ';';

    row = lit("row")[phx::bind(&GeometryParser::rowinit, this)]
          >> '{'
          >> *(top[phx::bind(&GeometryParser::setRowTop, this, _1)]
               || left[phx::bind(&GeometryParser::setRowLeft, this, _1)]
               || localShape[phx::bind(&GeometryParser::setRowShape, this, _1)]
               || localColor
               || comments
               || geomVertical[phx::bind(&GeometryParser::setVerticalRow, this)]
               || keys
              )
          >> lit("};") || ignore || geomVertical[phx::bind(&GeometryParser::setVerticalSection, this)];

    angle = lit("angle") >> '=' >> double_ >> ';';

    localShape = lit("key.shape") >> '=' >> name[_val = _1] >> ';';
    localColor = lit("key.color") >> '=' >> name >> ';';
    localDimension = (lit("height") || lit("width")) >> '=' >> double_ >> ';';
    priority = lit("priority") >> '=' >> double_ >> ';';

    section = lit("section")[phx::bind(&GeometryParser::sectioninit, this)]
              >> name[phx::bind(&GeometryParser::sectionName, this, _1)]
              >> '{'
              >> *(top[phx::bind(&GeometryParser::setSectionTop, this, _1)]
                   || left[phx::bind(&GeometryParser::setSectionLeft, this, _1)]
                   || angle[phx::bind(&GeometryParser::setSectionAngle, this, _1)]
                   || row[phx::bind(&GeometryParser::addRow, this)]
                   || localShape[phx::bind(&GeometryParser::setSectionShape, this, _1)]
                   || geomAtt
                   || localColor
                   || localDimension
                   || priority
                   || comments)
              >> lit("};") || geomVertical[phx::bind(&GeometryParser::setVerticalGeometry, this)];

    shapeC = lit("shape") >> '.' >> cornerRadius >> ';';

    shape = shapeDef || shapeC;


    input = '{'
            >> +(width
                 || height
                 || comments
                 || ignore
                 || description
                 || (char_ - keyword - '}'
                     || shape[phx::bind(&Geometry::addShape, &geom)]
                     || section[phx::bind(&Geometry::addSection, &geom)]
                     || geomAtt
                     || geomShape
                    ))
            >> '}';

    width = lit("width") >> '=' >> double_[phx::bind(&Geometry::setWidth, &geom, _1)] >> ";";
    height = lit("height") >> '=' >> double_[phx::bind(&Geometry::setHeight, &geom, _1)] >> ";";

    start %= *(lit("default"))
             >> lit("xkb_geometry")
             >> name[phx::bind(&GeometryParser::getName, this, _1)]
             >> input
             >> ';' >> *(comments || char_ - lit("xkb_geometry"));
}

template<typename Iterator>
void GeometryParser<Iterator>::setCord()
{
    geom.setShapeCord(shapeLenX, shapeLenY);
}


template<typename Iterator>
void GeometryParser<Iterator>::setSectionShape(std::string n)
{
    geom.sectionList[geom.getSectionCount()].setShapeName(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
void GeometryParser<Iterator>::getName(std::string n)
{
    geom.setName(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
void GeometryParser<Iterator>::getDescription(std::string n)
{
    geom.setDescription(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
void GeometryParser<Iterator>::getShapeName(std::string n)
{
    geom.setShapeName(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
void GeometryParser<Iterator>::setGeomShape(std::string n)
{
    geom.setKeyShape(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
void GeometryParser<Iterator>::setRowShape(std::string n)
{
    int secn = geom.getSectionCount();
    int rown = geom.sectionList[secn].getRowCount();
    geom.sectionList[secn].rowList[rown].setShapeName(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
void GeometryParser<Iterator>::setApprox()
{
    geom.setShapeApprox(approxLenX, approxLenY);
}


template<typename Iterator>
void GeometryParser<Iterator>::addRow()
{
    geom.sectionList[geom.getSectionCount()].addRow();
}


template<typename Iterator>
void GeometryParser<Iterator>::sectionName(std::string n)
{
    geom.sectionList[geom.getSectionCount()].setName(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
void GeometryParser<Iterator>::rowinit()
{
    int secn = geom.getSectionCount();
    int rown = geom.sectionList[secn].getRowCount();
    double tempTop = geom.sectionList[secn].getTop();
    QString tempShape = geom.sectionList[secn].getShapeName();
    geom.sectionList[secn].rowList[rown].setTop(tempTop);
    geom.sectionList[secn].rowList[rown].setLeft(geom.sectionList[secn].getLeft());
    geom.sectionList[secn].rowList[rown].setShapeName(tempShape);
    keyCordiX = geom.sectionList[secn].rowList[rown].getLeft();
    keyCordiY = geom.sectionList[secn].rowList[rown].getTop();
    tempTop = geom.sectionList[secn].getVertical();
    geom.sectionList[secn].rowList[rown].setVertical(tempTop);
}


template<typename Iterator>
void GeometryParser<Iterator>::sectioninit()
{
    int secn = geom.getSectionCount();
    geom.sectionList[secn].setTop(geom.sectionTop);
    geom.sectionList[secn].setLeft(geom.sectionLeft);
    keyCordiX = geom.sectionList[secn].getLeft();
    keyCordiY = geom.sectionList[secn].getTop();
    geom.sectionList[secn].setShapeName(geom.getKeyShape());
    geom.sectionList[secn].setVertical(geom.getVertical());
}


template<typename Iterator>
void GeometryParser<Iterator>::setRowTop(double a)
{
    int secn = geom.getSectionCount();
    int rown = geom.sectionList[secn].getRowCount();
    double tempTop = geom.sectionList[secn].getTop();
    geom.sectionList[secn].rowList[rown].setTop(a + tempTop);
    keyCordiY = geom.sectionList[secn].rowList[rown].getTop();
}


template<typename Iterator>
void GeometryParser<Iterator>::setRowLeft(double a)
{
    int secn = geom.getSectionCount();
    int rown = geom.sectionList[secn].getRowCount();
    double tempLeft = geom.sectionList[secn].getLeft();
    geom.sectionList[secn].rowList[rown].setLeft(a + tempLeft);
    keyCordiX = geom.sectionList[secn].rowList[rown].getLeft();
}


template<typename Iterator>
void GeometryParser<Iterator>::setSectionTop(double a)
{
    //qCDebug(KEYBOARD_PREVIEW) << "\nsectionCount" << geom.sectionCount;
    int secn = geom.getSectionCount();
    geom.sectionList[secn].setTop(a + geom.sectionTop);
    keyCordiY = geom.sectionList[secn].getTop();
}


template<typename Iterator>
void GeometryParser<Iterator>::setSectionLeft(double a)
{
    //qCDebug(KEYBOARD_PREVIEW) << "\nsectionCount" << geom.sectionCount;
    int secn = geom.getSectionCount();
    geom.sectionList[secn].setLeft(a + geom.sectionLeft);
    keyCordiX = geom.sectionList[secn].getLeft();

}


template<typename Iterator>
void GeometryParser<Iterator>::setSectionAngle(double a)
{
    //qCDebug(KEYBOARD_PREVIEW) << "\nsectionCount" << geom.sectionCount;
    int secn = geom.getSectionCount();
    geom.sectionList[secn].setAngle(a);
}


template<typename Iterator>
void GeometryParser<Iterator>::setVerticalRow()
{
    int secn = geom.getSectionCount();
    int rown = geom.sectionList[secn].getRowCount();
    geom.sectionList[secn].rowList[rown].setVertical(1);
}


template<typename Iterator>
void GeometryParser<Iterator>::setVerticalSection()
{
    int secn = geom.getSectionCount();
    geom.sectionList[secn].setVertical(1);
}


template<typename Iterator>
void GeometryParser<Iterator>::setVerticalGeometry()
{
    geom.setVertical(1);
}


template<typename Iterator>
void GeometryParser<Iterator>::setKeyName(std::string n)
{
    int secn = geom.getSectionCount();
    int rown = geom.sectionList[secn].getRowCount();
    int keyn = geom.sectionList[secn].rowList[rown].getKeyCount();
    //qCDebug(KEYBOARD_PREVIEW) << "\nsC: " << secn << "\trC: " << rown << "\tkn: " << keyn;
    geom.sectionList[secn].rowList[rown].keyList[keyn].setKeyName(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
void GeometryParser<Iterator>::setKeyShape(std::string n)
{
    int secn = geom.getSectionCount();
    int rown = geom.sectionList[secn].getRowCount();
    int keyn = geom.sectionList[secn].rowList[rown].getKeyCount();
    //qCDebug(KEYBOARD_PREVIEW) << "\nsC: " << secn << "\trC: " << rown << "\tkn: " << keyn;
    geom.sectionList[secn].rowList[rown].keyList[keyn].setShapeName(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
void GeometryParser<Iterator>::setKeyNameandShape(std::string n)
{
    int secn = geom.getSectionCount();
    int rown = geom.sectionList[secn].getRowCount();
    setKeyName(n);
    setKeyShape(geom.sectionList[secn].rowList[rown].getShapeName().toUtf8().constData());
}


template<typename Iterator>
void GeometryParser<Iterator>::setKeyOffset()
{
    //qCDebug(KEYBOARD_PREVIEW) << "\nhere\n";
    int secn = geom.getSectionCount();
    int rown = geom.sectionList[secn].getRowCount();
    int keyn = geom.sectionList[secn].rowList[rown].getKeyCount();
    //qCDebug(KEYBOARD_PREVIEW) << "\nsC: " << secn << "\trC: " << rown << "\tkn: " << keyn;
    geom.sectionList[secn].rowList[rown].keyList[keyn].setOffset(KeyOffset);
}


template<typename Iterator>
void GeometryParser<Iterator>::setKeyCordi()
{
    int secn = geom.getSectionCount();
    int rown = geom.sectionList[secn].getRowCount();
    int keyn = geom.sectionList[secn].rowList[rown].getKeyCount();
    int vertical = geom.sectionList[secn].rowList[rown].getVertical();

    Key key = geom.sectionList[secn].rowList[rown].keyList[keyn];

    if (vertical == 0) {
        keyCordiX += key.getOffset();
    } else {
        keyCordiY += key.getOffset();
    }

    geom.sectionList[secn].rowList[rown].keyList[keyn].setKeyPosition(keyCordiX, keyCordiY);

    QString shapeStr = key.getShapeName();
    if (shapeStr.isEmpty()) {
        shapeStr = geom.getKeyShape();
    }

    GShape shapeObj = geom.findShape(shapeStr);
    int a = shapeObj.size(vertical);

    if (vertical == 0) {
        keyCordiX += a + geom.keyGap;
    } else {
        keyCordiY += a + geom.keyGap;
    }

    geom.sectionList[secn].rowList[rown].addKey();
}


Geometry parseGeometry(const QString &model)
{
    using boost::spirit::iso8859_1::space;
    typedef std::string::const_iterator iterator_type;
    typedef grammar::GeometryParser<iterator_type> GeometryParser;
    GeometryParser geometryParser;

    Rules::GeometryId geoId = Rules::getGeometryId(model);
    QString geometryFile = geoId.fileName;
    QString geometryName = geoId.geoName;

    qCDebug(KEYBOARD_PREVIEW) << "looking for model" << model << "geometryName" << geometryName << "in" << geometryFile;

    QString input = getGeometry(geometryFile, geometryName);
    if (! input.isEmpty()) {
        geometryParser.geom = Geometry();
        input = includeGeometry(input);
        std::string parserInput = input.toUtf8().constData();

        std::string::const_iterator iter = parserInput.begin();
        std::string::const_iterator end = parserInput.end();

        bool success = phrase_parse(iter, end, geometryParser, space);

        if (success && iter == end) {
//                qCDebug(KEYBOARD_PREVIEW) << "Geometry parsing succeeded for" << input.left(20);
            geometryParser.geom.setParsing(true);
            return geometryParser.geom;
        } else {
            qCritical() << "Geometry parsing failed for\n\t" << input.left(30);
            geometryParser.geom.setParsing(false);
        }
    }

    if (geometryParser.geom.getParsing()) {
        return geometryParser.geom;
    }

    qCritical() << "Failed to get geometry" << geometryParser.geom.getName() << "falling back to pc104";
    return parseGeometry("pc104");
}

QString includeGeometry(QString geometry)
{
    QStringList lines = geometry.split("\n");
    int includeLine = -1;
    QString includeLineStr;
    QString startLine = lines[0];
    for (int i = 0; i < lines.size(); i++) {
        includeLineStr = lines[i];
        lines[i] = lines[i].remove(" ");
        lines[i] = lines[i].remove("\r");
        if (lines[i].startsWith("include")) {
            includeLine = i;
            break;
        }
    }
    if (includeLine == -1) {
        return geometry;
    }
    geometry = geometry.remove(includeLineStr);
    lines[includeLine] = lines[includeLine].remove("include");
    lines[includeLine] = lines[includeLine].remove("\"");
    lines[includeLine] = lines[includeLine].remove(")");
    if (lines[includeLine].contains("(")) {
        QString includeFile = lines[includeLine].split("(")[0];
        QString includeGeom = lines[includeLine].split("(")[1];
        qCDebug(KEYBOARD_PREVIEW) << "looking to include " << "geometryName" << includeGeom << "in" << includeFile;
        QString includeStr = getGeometry(includeFile, includeGeom);
        includeStr = getGeometryStrContent(includeStr);
        geometry = geometry.remove(startLine);
        geometry = geometry.prepend(includeStr);
        geometry = geometry.prepend(startLine);
        includeGeometry(geometry);

    }
    return geometry;
}

QString getGeometryStrContent(QString geometryStr)
{
    int k = geometryStr.indexOf("{");
    int k2 = geometryStr.lastIndexOf("};");
    geometryStr = geometryStr.mid(k + 1, k2 - k - 2);
    return geometryStr;
}

QString getGeometry(QString geometryFile, QString geometryName)
{

    QString xkbParentDir = findGeometryBaseDir();
    geometryFile.prepend(xkbParentDir);
    QFile gfile(geometryFile);

    if (!gfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Unable to open the file" << geometryFile;
        return QString();
    }

    QString gcontent = gfile.readAll();
    gfile.close();

    QStringList gcontentList = gcontent.split("xkb_geometry ");

    int current = 0;
    for (int i = 1; i < gcontentList.size(); i++) {
        if (gcontentList[i].startsWith("\"" + geometryName + "\"")) {
            current = i;
            break;
        }
    }
    if (current != 0) {
        return gcontentList[current].prepend("xkb_geometry ");
    } else {
        return QString();
    }
}


QString findGeometryBaseDir()
{
    QString xkbDir = Rules::findXkbDir();
    return QString("%1/geometry/").arg(xkbDir);
}

}
