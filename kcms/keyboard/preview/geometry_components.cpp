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


#include "geometry_components.h"

#include <QString>
#include <QDebug>
#include <QPoint>


Q_LOGGING_CATEGORY(KEYBOARD_PREVIEW, "keyboard_preview")


GShape::GShape()
{
    cordi_count = 0;
}

void GShape::setCordinate(double a, double b)
{
    cordii << QPoint(a, b);
    cordi_count++;
}

void GShape::setApprox(double a, double b)
{
    a -= approx.x();
    b -= approx.y();
    approx = QPoint(a, b);
}

QPoint GShape :: getCordii(int i) const
{
    if (i < cordi_count) {
        return cordii[i];
    }

    return QPoint();
}

void GShape::display()
{
    qCDebug(KEYBOARD_PREVIEW) << "shape: " << sname << "\n";
    qCDebug(KEYBOARD_PREVIEW) << "(" << approx.x() << "," << approx.y() << ");";

    for (int i = 0; i < cordi_count; i++) {
        qCDebug(KEYBOARD_PREVIEW) << cordii[i];
    }
}

double GShape::size(int vertical) const
{
    if (!cordii.isEmpty()) {
        if (vertical == 0) {
            if (approx.x() == 0 && approx.y() == 0) {
                int max = 0;
                for (int i = 0; i < cordi_count; i++) {
                    if (max < cordii[i].x()) {
                        max = cordii[i].x();
                    }
                }
                return max;
            } else {
                return approx.x();
            }
        } else {
            if (approx.x() == 0 && approx.y() == 0) {
                int max = 0;
                for (int i = 0; i < cordi_count; i++) {
                    if (max < cordii[i].y()) {
                        max = cordii[i].y();
                    }
                }
                return max;
            }

            return approx.y();
        }
    }

    return 0;
}


Key::Key()
{
    offset = 0;
}

void Key::setKeyPosition(double x, double y)
{
    position = QPoint(x, y);
}

void Key::showKey()
{
    qCDebug(KEYBOARD_PREVIEW) << "\n\tKey: " << name << "\tshape: " << shapeName << "\toffset: " << offset;
    qCDebug(KEYBOARD_PREVIEW) << "\tposition" << position;
}


Row::Row()
{
    top = 0;
    left = 0;
    keyCount = 0;
    vertical = 0;
    keyList << Key();
}

void Row::addKey()
{
    //qCDebug(KEYBOARD_PREVIEW) << "keyCount: " << keyCount;
    keyCount++;
    keyList << Key();
}

void Row::displayRow()
{
    qCDebug(KEYBOARD_PREVIEW) << "\nRow: (" << left << "," << top << ")\n";
    qCDebug(KEYBOARD_PREVIEW) << "vertical: " << vertical;
    for (int i = 0; i < keyCount; i++) {
        keyList[i].showKey();
    }
}


Section::Section()
{
    top = 0;
    left = 0;
    angle = 0;
    rowCount = 0;
    vertical = 0;
    rowList << Row();
}

void Section::addRow()
{
    //qCDebug(KEYBOARD_PREVIEW) << "\nrowCount: " << rowCount;
    rowCount++;
    rowList << Row();
}

void Section::displaySection()
{
    //qCDebug(KEYBOARD_PREVIEW) << "\nSection: " << name << "\n\tposition: (" << left << "," << top << ");" << angle << "\n";
    //qCDebug(KEYBOARD_PREVIEW) << "vertical: " << vertical;
    for (int i = 0; i < rowCount; i++) {
        qCDebug(KEYBOARD_PREVIEW) << "\n\t";
        rowList[i].displayRow();
    }
}


Geometry::Geometry()
{
    sectionTop = 0;
    sectionLeft = 0;
    rowTop = 0;
    rowLeft = 0;
    keyGap = 0;
    shape_count = 0;
    width = 0;
    height = 0;
    sectionCount = 0;
    vertical = 0;
    sectionList << Section();
    shapes << GShape();
    keyShape = QString("NORM");
    parsedGeometry = true;
}

void Geometry::setShapeName(const QString &n)
{
    shapes[shape_count].setShapeName(n);
}

void Geometry::setShapeCord(double a, double b)
{
    shapes[shape_count].setCordinate(a, b);
}

void Geometry::setShapeApprox(double a, double b)
{
    shapes[shape_count].setApprox(a, b);
}

void Geometry::addShape()
{
    shape_count++;
    shapes << GShape();
}

void Geometry::display()
{
    qCDebug(KEYBOARD_PREVIEW) << name << "\n" << description << "\nwidth:" << width
                              << "\nheight:" << height << "\n" << "sectionTop:" << sectionTop;
    qCDebug(KEYBOARD_PREVIEW) << "\nsectionLeft:" << sectionLeft << "\nrowTop:" << rowTop << "\nrowLeft:"
                              << rowLeft << "\nkeyGap: " << keyGap << "\nkeyShape:" << keyShape << "\n";
    qCDebug(KEYBOARD_PREVIEW) << "vertical:" << vertical;

    for (int i = 0; i < shape_count; i++) {
        shapes[i].display();
    }

    for (int j = 0; j < sectionCount; j++) {
        sectionList[j].displaySection();
    }
}

void Geometry::addSection()
{
    //qCDebug(KEYBOARD_PREVIEW) << "\nsectionCount: " << sectionCount;
    sectionCount++;
    sectionList << Section();
}

GShape Geometry::findShape(const QString &name)
{
    GShape l;

    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].getShapeName() == name) {
            return shapes[i];
        }
    }
    return l;
}
