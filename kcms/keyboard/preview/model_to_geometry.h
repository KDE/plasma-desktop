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

#ifndef MODEL_TO_GEOMETRY_H
#define  MODEL_TO_GEOMETRY_H

#include <QString>
#include <QStringList>


class ModelGroup {

    QStringList groupModels;
    QString groupName;


public:


    ModelGroup();

    void addGroupModel( QString model );

    void setGroupName(QString name){
        groupName = name;
    }

    QString getGroupName(){
        return groupName;
    }

    int containsModel(QString model);

    void display();
};


class ModelToGeometry{

    QString geometryName;
    QString geometryFile;
    QString kbModel;

public :

    ModelToGeometry();

    void setGeometryName(QString name){
        geometryName = name;
    }

    void setGeometryFile(QString file){
        geometryFile = file;
    }

    void setKbModel(QString model){
        kbModel = model;
    }

    QString getGeometryFile(){
        return geometryFile;
    }

    QString getGeometryName(){
        return geometryName;
    }

    QString getModel(){
        return kbModel;
    }

};


class ModelToGeometryTable{

    int entryCount, modelGroupCount;
    QList <ModelGroup> modelGroups;
    QString defaultGeometryFile, defaultGeometryName;
    bool parsing;

public:
    QList <ModelToGeometry> table;

    ModelToGeometryTable();


    QString getGeometryName(QString model);
    QString getGeometryFile(QString model);
    void createTable();
    void addEntry(ModelToGeometry currentEntry);
    void display();
    void createModelGroups(QString content);
    void addModelGroup();

};

#endif //model_to_geometry.h
