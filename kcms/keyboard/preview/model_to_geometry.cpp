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

#include "model_to_geometry.h"
#include "../x11_helper.h"

#include <QFile>
#include <QDebug>
#include <QRegExp>

ModelGroup :: ModelGroup(){

}

void ModelGroup :: addGroupModel(QString model){
    if( !groupModels.contains(model) ){
        groupModels << model;
    }
}

int ModelGroup :: containsModel(QString model){
    for(int i = 0 ; i < groupModels.size() ; i++){
        if (model == groupModels.at(i)){
            return i;
        }
    }
    return -1;
}

void ModelGroup :: display(){
    //qDebug()<<groupName<<"\t"<<modelCount<<"\t"<<groupModels;
}

ModelToGeometry :: ModelToGeometry(){
}

ModelToGeometryTable :: ModelToGeometryTable(){

    modelGroups << ModelGroup();
    entryCount = 0;
    modelGroupCount = 0;

}

void ModelToGeometryTable :: addEntry(ModelToGeometry entry){
    table << entry;
    entryCount++;
}

QString ModelToGeometryTable :: getGeometryFile(QString model){

    if( !parsing )
        return "pc";

    for (int i = 0; i < entryCount ; i++){

        ModelToGeometry entry = table.at(i);
        QString tempModel = entry.getModel();

        if(tempModel.startsWith('$')){
            for ( int j = 0; j < modelGroupCount; j++){
                ModelGroup temp = modelGroups.at(j);
                if (temp.getGroupName() == tempModel && temp.containsModel(model) > -1){
                    return entry.getGeometryFile();
                }
            }
        }
        else{
            if (tempModel == model){
                return entry.getGeometryFile();
            }
        }

    }
    return defaultGeometryFile;
}


QString ModelToGeometryTable :: getGeometryName(QString model){

    if( !parsing )
        return "pc104";

    for (int i = 0; i < entryCount ; i++){

        ModelToGeometry entry = table.at(i);
        QString tempModel = entry.getModel();
        if(tempModel.startsWith('$')){
            for ( int j = 0; j < modelGroupCount; j++){
                ModelGroup temp = modelGroups.at(j);
                if (temp.getGroupName() == tempModel && temp.containsModel(model) > -1){
                    if(! (entry.getGeometryName() == "%m") ){
                        return entry.getGeometryName();
                    }
                    else{
                        return model;
                    }
                }
            }
        }
        else{
            if (tempModel == model){
                return entry.getGeometryName();
           }
        }
    }
    return defaultGeometryName;
}

void ModelToGeometryTable :: addModelGroup(){
    modelGroups << ModelGroup();
    modelGroupCount++;
}

void ModelToGeometryTable :: createTable(){

    QString xkbDir = X11Helper::findXkbDir();
    QString xkbRulesDir = QString("%1/rules/base").arg(xkbDir);
    QFile baseFile(xkbRulesDir);
    if (!baseFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"unable to open the file";
        parsing = false;
    }
    else{
        QString baseContent = baseFile.readAll();
        baseFile.close();

        QString tableStr = baseContent;

        QRegExp tableExp ("[!]\\s*\\b(model)\\b\\s*=\\s*\\b(geometry)\\b\\n*[^!]*");
        QRegExp tableHeader ("[!]\\s*\\b(model)\\b\\s*=\\s*\\b(geometry)\\b\\n");

        int index = tableExp.indexIn(tableStr);
        int tableLength = tableExp.matchedLength();
        tableStr = tableStr.mid(index, tableLength);
        tableStr.remove(tableHeader);
        //qDebug()<<tableStr;
        QStringList entries = tableStr.split("\n");

        bool defaultEntry = false;

        for (int i = 1 ; i < entries.size() - 1; i++){

            QString tupple = entries.at(i);

            ModelToGeometry currentEntry;
            tupple.remove(" ");
            tupple.remove("\t");
            QStringList tuppleElements = tupple.split("=");
            if(tuppleElements.size() == 2){
                QString kbModel = tuppleElements.at(0);
                currentEntry.setKbModel(kbModel);

                if( kbModel == "*"){
                    defaultEntry = true;
                }
                if(kbModel.startsWith("$")){
                    modelGroups[modelGroupCount].setGroupName(kbModel);
                    addModelGroup();
                }

                QString geomInfo = tuppleElements.at(1);
                QStringList geometryFileinfo = geomInfo.split("(");
                if(geometryFileinfo.size() == 2){
                    QString geometryFile = geometryFileinfo.at(0);
                    currentEntry.setGeometryFile(geometryFile);
                    QString geometryName = geometryFileinfo.at(1);
                    geometryName.remove(")");
                    if(geometryName == "intl")
                        geometryName = "60";
                    currentEntry.setGeometryName(geometryName);
                    addEntry(currentEntry);
                    if(defaultEntry){
                        defaultGeometryFile = geometryFile;
                        defaultGeometryName = geometryName;
                        defaultEntry = false;
                        parsing = true;
                    }
                }
            }
        }
        createModelGroups(baseContent);
        //display();
    }
}
void ModelToGeometryTable :: createModelGroups(QString content){

    QString groupStr = content;

    for(int i = 0 ; i < modelGroupCount; i++){
        QString input = groupStr;
        QString gname = modelGroups[i].getGroupName();
        gname.remove('$');
        QRegExp variable("[!]\\s*[$]\\b(" + gname + ")\\b[^\\n]*");

        int index = variable.indexIn(input);
        int len = variable.matchedLength();
        input = input.mid(index, len);

        //qDebug()<<"input: "<<input;

        QString modelList = input.split("=").at(1);
        QStringList models = modelList.split(" ");

        ModelGroup temp =  modelGroups.at(i);

        for(int j = 1; j < models.size(); j++){
            //qDebug()<<"model At"<<j<<":"<<models.at(j);
            QString model = models.at(j);
            temp.addGroupModel(model);
        }

        modelGroups.replace(i, temp);

    }
}

void ModelToGeometryTable :: display(){
    for(int i = 0 ; i < entryCount; i++){
        ModelToGeometry temp = table.at(i);
        qDebug() << temp.getModel() << "\t"<<temp.getGeometryFile()<<"\t"<<temp.getGeometryName();
    }
    for (int i = 0; i < modelGroupCount; i ++){
        ModelGroup temp = modelGroups.at(i);
        temp.display();
    }
}

