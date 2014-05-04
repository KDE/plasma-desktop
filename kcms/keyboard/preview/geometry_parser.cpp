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
#include "x11_helper.h"
#include "model_to_geometry.h"

#include <QString>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QPair>


#include <fixx11h.h>
#include <config-workspace.h>

namespace grammar{
keywords::keywords(){
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
Geometry_parser<Iterator>::Geometry_parser():Geometry_parser::base_type(start){

    using qi::lexeme;
    using qi::char_;
    using qi::lit;
    using qi::_1;
    using qi::_val;
    using qi::int_;
    using qi::double_;
    using qi::eol;


    name = '"'>>+(char_-'"')>>'"';

    ignore =(lit("outline")||lit("overlay")||lit("text"))>>*(char_-lit("};"))>>lit("};")
            ||lit("solid")>>*(char_-lit("};"))>>lit("};")
            ||lit("indicator")>>*(char_-';'-'{')>>';'||'{'>>*(char_-lit("};"))>>lit("};")
            ||lit("indicator")>>'.'>>lit("shape")>>'='>>name>>';';

    comments =lexeme[ lit("//")>>*(char_-eol||keyword-eol)>>eol||lit("/*")>>*(char_-lit("*/")||keyword-lit("*/"))>>lit("*/") ];

    cordinates = ('['
            >>double_[phx::ref(shapeLenX)=_1]
            >>','
            >>double_[phx::ref(shapeLenY)=_1]
            >>']')
            ||'['>>double_>>",">>double_>>']'
            ;

    cordinatea = '['>>double_[phx::ref(approxLenX)=_1]>>",">>double_[phx::ref(approxLenY)=_1]>>']';

    set = '{'>>cordinates>>*(','>>cordinates)>>'}';

    setap = '{'>>cordinatea>>*(','>>cordinatea)>>'}';

    seta = '{'
            >>cordinates[phx::bind(&Geometry_parser::setCord,this)]
            >>*(','>>cordinates[phx::bind(&Geometry_parser::setCord,this)])
            >>'}'
            ;

    description = lit("description")>>'='>>name[phx::bind(&Geometry_parser::getDescription,this,_1)]>>';';

    cornerRadius = (lit("cornerRadius")||lit("corner"))>>'='>>double_;

    shapeDef = lit("shape")
            >>name[phx::bind(&Geometry_parser::getShapeName,this,_1)]
            >>'{'
            >>*(lit("approx")>>'='>>setap[phx::bind(&Geometry_parser::setApprox,this)]>>','||cornerRadius>>','||comments)
            >>seta
            >>*((','>>(set||lit("approx")>>'='>>setap[phx::bind(&Geometry_parser::setApprox,this)]||cornerRadius)||comments))
            >>lit("};")
            ;

    keyName = '<'>>+(char_-'>')>>'>';

    keyShape = *(lit("key."))>>lit("shape")>>'='>>name[phx::bind(&Geometry_parser::setKeyShape,this,_1)]
            ||name[phx::bind(&Geometry_parser::setKeyShape,this,_1)];

    keyColor = lit("color")>>'='>>name;

    keygap = lit("gap")>>'='>>double_[phx::ref(KeyOffset)=_1]||double_[phx::ref(KeyOffset)=_1];

    keyDesc = keyName[phx::bind(&Geometry_parser::setKeyNameandShape,this,_1)]
            ||'{'>>(keyName[phx::bind(&Geometry_parser::setKeyNameandShape,this,_1)]||keyShape
                   ||keygap[phx::bind(&Geometry_parser::setKeyOffset,this)]
                   ||keyColor)
            >>*((','
            >>(keyName
            ||keyShape
            ||keygap[phx::bind(&Geometry_parser::setKeyOffset,this)]
            ||keyColor))
            ||comments)
            >>'}';

    keys = lit("keys")
            >>'{'
            >>keyDesc[phx::bind(&Geometry_parser::setKeyCordi,this)]
            >>*((*lit(',')>>keyDesc[phx::bind(&Geometry_parser::setKeyCordi,this)]>>*lit(','))||comments)
            >>lit("};");

    geomShape = ((lit("key.shape")>>'='>>name[phx::bind(&Geometry_parser::setGeomShape,this,_1)])||(lit("key.color")>>'='>>name))>>';';
    geomLeft = lit("section.left")>>'='>>double_[phx::ref(geom.sectionLeft)=_1]>>';';
    geomTop = lit("section.top")>>'='>>double_[phx::ref(geom.sectionTop)=_1]>>';';
    geomRowTop = lit("row.top")>>'='>>double_[phx::ref(geom.rowTop)=_1]>>';';
    geomRowLeft = lit("row.left")>>'='>>double_[phx::ref(geom.rowLeft)=_1]>>';';
    geomGap = lit("key.gap")>>'='>>double_[phx::ref(geom.keyGap)=_1]>>';';
    geomVertical = *lit("row.")>>lit("vertical")>>'='>>(lit("True")||lit("true"))>>';';
    geomAtt = geomLeft||geomTop||geomRowTop||geomRowLeft||geomGap;

    top = lit("top")>>'='>>double_>>';';
    left = lit("left")>>'='>>double_>>';';

    row = lit("row")[phx::bind(&Geometry_parser::rowinit,this)]
            >>'{'
            >>*(top[phx::bind(&Geometry_parser::setRowTop,this,_1)]
            ||left[phx::bind(&Geometry_parser::setRowLeft,this,_1)]
            ||localShape[phx::bind(&Geometry_parser::setRowShape,this,_1)]
            ||localColor
            ||comments
            ||geomVertical[phx::bind(&Geometry_parser::setVerticalRow,this)]
            ||keys
            )
            >>lit("};")||ignore||geomVertical[phx::bind(&Geometry_parser::setVerticalSection,this)];

    angle = lit("angle")>>'='>>double_>>';';

    localShape = lit("key.shape")>>'='>>name[_val=_1]>>';';
    localColor = lit("key.color")>>'='>>name>>';';
    localDimension = (lit("height")||lit("width"))>>'='>>double_>>';';
    priority = lit("priority")>>'='>>double_>>';';

    section = lit("section")[phx::bind(&Geometry_parser::sectioninit,this)]
            >>name[phx::bind(&Geometry_parser::sectionName,this,_1)]
            >>'{'
            >>*(top[phx::bind(&Geometry_parser::setSectionTop,this,_1)]
            ||left[phx::bind(&Geometry_parser::setSectionLeft,this,_1)]
            ||angle[phx::bind(&Geometry_parser::setSectionAngle,this,_1)]
            ||row[phx::bind(&Geometry_parser::addRow,this)]
            ||localShape[phx::bind(&Geometry_parser::setSectionShape,this,_1)]
            ||geomAtt
            ||localColor
            ||localDimension
            ||priority
            ||comments)
            >>lit("};")||geomVertical[phx::bind(&Geometry_parser::setVerticalGeometry,this)];

    shapeC = lit("shape")>>'.'>>cornerRadius>>';';

    shape = shapeDef||shapeC;


    input = '{'
          >>+(width
          ||height
          ||comments
          ||ignore
          ||description
          ||(char_-keyword-'}'
          ||shape[phx::bind(&Geometry::addShape,&geom)]
          ||section[phx::bind(&Geometry::addSection,&geom)]
          ||geomAtt
          ||geomShape
          ))
          >>'}';

          width = lit("width")>>'='>>double_[phx::bind(&Geometry::setWidth,&geom,_1)]>>";";
          height = lit("height")>>'='>>double_[phx::bind(&Geometry::setHeight,&geom,_1)]>>";";

          start %= *(lit("default"))
                 >>lit("xkb_geometry")
                 >>name[phx::bind(&Geometry_parser::getName,this,_1)]
                 >>input
                 >>';'>>*(comments||char_-lit("xkb_geometry"));
}

template<typename Iterator>
    void Geometry_parser<Iterator>::setCord(){
        geom.setShapeCord(shapeLenX, shapeLenY);
    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setSectionShape(std::string n){
    geom.sectionList[geom.getSectionCount()].setShapeName( QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
    void Geometry_parser<Iterator>::getName(std::string n){
        geom.setName(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
    void Geometry_parser<Iterator>::getDescription(std::string n){
        geom.setDescription( QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
    void Geometry_parser<Iterator>::getShapeName(std::string n){
        geom.setShapeName( QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
    void Geometry_parser<Iterator>::setGeomShape(std::string n){
        geom.setKeyShape(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
void Geometry_parser<Iterator>::setRowShape(std::string n){
    int secn = geom.getSectionCount();
    int rown = geom.sectionList[secn].getRowCount();
    geom.sectionList[secn].rowList[rown].setShapeName(QString::fromUtf8(n.data(), n.size() ));
}


template<typename Iterator>
    void Geometry_parser<Iterator>::setApprox(){
        geom.setShapeApprox(approxLenX, approxLenY);
}


template<typename Iterator>
    void Geometry_parser<Iterator>::addRow(){
        geom.sectionList[geom.getSectionCount()].addRow();
}


template<typename Iterator>
    void Geometry_parser<Iterator>::sectionName(std::string n){
        geom.sectionList[geom.getSectionCount()].setName(QString::fromUtf8(n.data(), n.size()));
}


template<typename Iterator>
    void Geometry_parser<Iterator>::rowinit(){
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
    void Geometry_parser<Iterator>::sectioninit(){
        int secn = geom.getSectionCount();
        geom.sectionList[secn].setTop(geom.sectionTop);
        geom.sectionList[secn].setLeft(geom.sectionLeft);
        keyCordiX = geom.sectionList[secn].getLeft();
        keyCordiY = geom.sectionList[secn].getTop();
        geom.sectionList[secn].setShapeName(geom.getKeyShape());
        geom.sectionList[secn].setVertical(geom.getVertical());
    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setRowTop(double a){
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        double tempTop = geom.sectionList[secn].getTop();
        geom.sectionList[secn].rowList[rown].setTop(a + tempTop);
        keyCordiY = geom.sectionList[secn].rowList[rown].getTop();
    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setRowLeft(double a){
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        double tempLeft = geom.sectionList[secn].getLeft();
        geom.sectionList[secn].rowList[rown].setLeft(a + tempLeft);
        keyCordiX = geom.sectionList[secn].rowList[rown].getLeft();
    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setSectionTop(double a){
        //qDebug()<<"\nsectionCount"<<geom.sectionCount;
        int secn = geom.getSectionCount();
        geom.sectionList[secn].setTop(a + geom.sectionTop);
        keyCordiY = geom.sectionList[secn].getTop();
    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setSectionLeft(double a){
        //qDebug()<<"\nsectionCount"<<geom.sectionCount;
        int secn = geom.getSectionCount();
        geom.sectionList[secn].setLeft(a + geom.sectionLeft);
        keyCordiX = geom.sectionList[secn].getLeft();

    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setSectionAngle(double a){
        //qDebug()<<"\nsectionCount"<<geom.sectionCount;
        int secn = geom.getSectionCount();
        geom.sectionList[secn].setAngle(a);
    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setVerticalRow(){
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        geom.sectionList[secn].rowList[rown].setVertical(1);
    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setVerticalSection(){
        int secn = geom.getSectionCount();
        geom.sectionList[secn].setVertical(1);
    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setVerticalGeometry(){
        geom.setVertical(1);
    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyName(std::string n){
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        int keyn = geom.sectionList[secn].rowList[rown].getKeyCount();
        //qDebug()<<"\nsC: "<<secn<<"\trC: "<<rown<<"\tkn: "<<keyn;
        geom.sectionList[secn].rowList[rown].keyList[keyn].setKeyName(QString::fromUtf8(n.data(), n.size()));
     }


template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyShape(std::string n){
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        int keyn = geom.sectionList[secn].rowList[rown].getKeyCount();
        //qDebug()<<"\nsC: "<<secn<<"\trC: "<<rown<<"\tkn: "<<keyn;
        geom.sectionList[secn].rowList[rown].keyList[keyn].setShapeName(QString::fromUtf8(n.data(), n.size()));
    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyNameandShape(std::string n){
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        setKeyName(n);
        setKeyShape(geom.sectionList[secn].rowList[rown].getShapeName().toUtf8().constData());
    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyOffset(){
        //qDebug()<<"\nhere\n";
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        int keyn = geom.sectionList[secn].rowList[rown].getKeyCount();
        //qDebug()<<"\nsC: "<<secn<<"\trC: "<<rown<<"\tkn: "<<keyn;
        geom.sectionList[secn].rowList[rown].keyList[keyn].setOffset(KeyOffset);
    }


template<typename Iterator>
    void Geometry_parser<Iterator>::setKeyCordi(){
        int secn = geom.getSectionCount();
        int rown = geom.sectionList[secn].getRowCount();
        int keyn = geom.sectionList[secn].rowList[rown].getKeyCount();
        int vertical = geom.sectionList[secn].rowList[rown].getVertical();

        Key key = geom.sectionList[secn].rowList[rown].keyList[keyn];

        if(vertical == 0)
            keyCordiX+= key.getOffset();
        else
            keyCordiY+= key.getOffset();

        geom.sectionList[secn].rowList[rown].keyList[keyn].setKeyPosition(keyCordiX, keyCordiY);

        QString shapeStr = key.getShapeName();
        if ( shapeStr.isEmpty() )
            shapeStr = geom.getKeyShape();

        GShape shapeObj = geom.findShape(shapeStr);
        int a = shapeObj.size(vertical);

        if(vertical == 0)
            keyCordiX+=a+geom.keyGap;
        else
            keyCordiY+=a+geom.keyGap;

        geom.sectionList[secn].rowList[rown].addKey();
    }




    Geometry parseGeometry(QString model){
        using boost::spirit::iso8859_1::space;
        typedef std::string::const_iterator iterator_type;
        typedef grammar::Geometry_parser<iterator_type> Geometry_parser;
        Geometry_parser geomertyParser;
        ModelToGeometryTable m2g = ModelToGeometryTable();

        m2g.createTable();
        QString geometryFile = m2g.getGeometryFile(model);
        QString geometryName = m2g.getGeometryName(model);
        //qDebug()<< geometryFile << geometryName;

        QString xkbParentDir = findGeometryBaseDir();
        geometryFile.prepend(xkbParentDir);
        QFile gfile(geometryFile);
         if (!gfile.open(QIODevice::ReadOnly | QIODevice::Text)){
             qDebug()<<"unable to open the file";
             geomertyParser.geom.setParsing(false);
             return geomertyParser.geom;
        }

        QString gcontent = gfile.readAll();
        gfile.close();

        QStringList gcontentList = gcontent.split("xkb_geometry");

        int current = 1;
        while(geomertyParser.geom.getName()!=geometryName && current < gcontentList.size() ){
            geomertyParser.geom = Geometry();
            QString input = gcontentList.at(current);
            input.prepend("xkb_geometry");
            //qDebug()<<input;
            std::string parserInput = input.toUtf8().constData();

            std::string::const_iterator iter = parserInput.begin();
            std::string::const_iterator end = parserInput.end();

            bool success = phrase_parse(iter, end, geomertyParser, space);

            if (success && iter == end){
                qDebug() << "Geometry Parsing succeeded";
                geomertyParser.geom.setParsing(true);
            }
            else{
                qDebug() << "Geometry Parsing failed\n";
                qDebug()<<input;
                geomertyParser.geom.setParsing(false);

            }

            current++;

        }
        //g.geom.display();
        if(geomertyParser.geom.getParsing())
            return geomertyParser.geom;
        else
            return parseGeometry("pc104");
    }

    QString findGeometryBaseDir()
    {
        QString xkbDir = X11Helper::findXkbDir();
        return QString("%1/geometry/").arg(xkbDir);
    }

}
