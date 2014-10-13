/*
 *  Copyright (C) 2012 Shivam Makkar (amourphious1992@gmail.com)
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


#ifndef GEOMETRY_PARSER_H
#define GEOMETRY_PARSER_H

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/home/support/char_encoding/iso8859_1.hpp>


#include <iostream>
#include <QDebug>
#include <QStringList>

#include "geometry_components.h"

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;
namespace iso = boost::spirit::iso8859_1;


namespace grammar
{

struct keywords : qi::symbols<char, int> {
    keywords();
};

template<typename Iterator>
struct GeometryParser : qi::grammar<Iterator, iso::space_type> {

    //comments
    qi::rule<Iterator, std::string(), iso::space_type>comments, ignore;
    qi::rule<Iterator, double(), iso::space_type>localDimension, priority;

    //general non-temrminals
    qi::rule<Iterator, std::string(), iso::space_type>name;
    qi::rule<Iterator, std::string(), iso::space_type>description;
    qi::rule<Iterator, std::string(), iso::space_type>input;

    //non-teminals for shape
    qi::rule<Iterator, int(), iso::space_type>shape;
    qi::rule<Iterator, int(), iso::space_type>shapeDef;
    qi::rule<Iterator, int(), iso::space_type>shapeC;
    qi::rule<Iterator, int(), iso::space_type>set;
    qi::rule<Iterator, int(), iso::space_type>setap;
    qi::rule<Iterator, int(), iso::space_type>seta;
    qi::rule<Iterator, int(), iso::space_type>cornerRadius;
    qi::rule<Iterator, int(), iso::space_type>cordinatea;
    qi::rule<Iterator, int(), iso::space_type>cordinates;

    //non-teminals for key
    qi::rule<Iterator, double(), iso::space_type>keygap;
    qi::rule<Iterator, std::string(), iso::space_type>keyName;
    qi::rule<Iterator, std::string(), iso::space_type>keyShape;
    qi::rule<Iterator, std::string(), iso::space_type>keyColor;
    qi::rule<Iterator, std::string(), iso::space_type>keyDesc;
    qi::rule<Iterator, std::string(), iso::space_type>keys;

    qi::rule<Iterator, std::string(), iso::space_type>row;

    qi::rule<Iterator, std::string(), iso::space_type>section;

    //non-teminals related to local data
    qi::rule<Iterator, std::string(), iso::space_type>localShape;
    qi::rule<Iterator, std::string(), iso::space_type>localColor;

    //Geometry non-terminals
    qi::rule<Iterator, std::string(), iso::space_type>geomShape;
    qi::rule<Iterator, int(), iso::space_type>geomTop, geomVertical;
    qi::rule<Iterator, int(), iso::space_type>geomLeft;
    qi::rule<Iterator, int(), iso::space_type>geomRowTop;
    qi::rule<Iterator, int(), iso::space_type>geomRowLeft;
    qi::rule<Iterator, int(), iso::space_type>geomGap;
    qi::rule<Iterator, int(), iso::space_type>geomAtt;
    qi::rule<Iterator, int(), iso::space_type>angle;
    qi::rule<Iterator, int(), iso::space_type>top;
    qi::rule<Iterator, int(), iso::space_type>left;
    qi::rule<Iterator, int(), iso::space_type>width;
    qi::rule<Iterator, int(), iso::space_type>height;

    qi::rule<Iterator, iso::space_type>start;
    Geometry geom;
    keywords keyword;
    double shapeLenX, shapeLenY, approxLenX, approxLenY, keyCordiX, keyCordiY, KeyOffset;
    GeometryParser();

    //functions for shape
    void getShapeName(std::string n);
    void setCord();
    void setApprox();

    //functions for section
    void sectionName(std::string n);
    void setSectionShape(std::string n);
    void setSectionTop(double a);
    void setSectionLeft(double a);
    void setSectionAngle(double a);
    void sectioninit();

    //functions for row
    void setRowShape(std::string n);
    void setRowTop(double a);
    void setRowLeft(double a);
    void rowinit();
    void addRow();

    //functions for key
    void setKeyName(std::string n);
    void setKeyShape(std::string n);
    void setKeyNameandShape(std::string n);
    void setKeyOffset();
    void setKeyCordi();

    //functionsfor geometry
    void setGeomShape(std::string n);
    void getName(std::string n);
    void getDescription(std::string n);

    //functions for alingment
    void setVerticalRow();
    void setVerticalSection();
    void setVerticalGeometry();
};



Geometry parseGeometry(const QString &model);
QString getGeometry(QString geometryFile, QString geometryName);
QString includeGeometry(QString geometry);
QString getGeometryStrContent(QString geometryStr);
QString findGeometryBaseDir();
}

#endif //geometry_parser
