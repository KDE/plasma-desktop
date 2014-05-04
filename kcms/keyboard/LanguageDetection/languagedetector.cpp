#include "languagedetector.h"
#include "training_text_map.h"

#include <kstandarddirs.h>
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QMessageBox>


LanguageDetector::LanguageDetector()
{
    KStandardDirs kdir;
    QStringList langdataDir = kdir.findDirs("data", "kcmkeyboard/langdata");
    qDebug()<<"here"<<langdataDir;

    if(!langdataDir.isEmpty()){

        langdataPath = langdataDir[0];
        createTrainingData();
    }
    else
        qDebug()<<"no langdata found";
}

void LanguageDetector::createTrainingData(){

    langDir.setPath(langdataPath);

    langList = langDir.entryList();

    qDebug()<< langList;
    QString content;

    for (int i = 2; i < langList.size(); i++){

        QFile langFile(langDir.filePath(langList[i]));

        if(langFile.open(QIODevice::ReadOnly | QIODevice::Text)){

            content = langFile.readAll();

            TrainingTextMap temp;
            temp.createTTMap(content);
            temp.setLanguageName(langList[i]);
            trainingData << temp;

        }
        else{

            qDebug()<<"unable to open File: "<<langList[i];

        }

    }
}

QString LanguageDetector::detectLanguage(QString input){

    InputMap inputMap;

    inputMap.createTTMap(input);

    for(int i = 0; i < trainingData.size(); i++)
        trainingData[i].matchedNGram = 0;

    return detectLanguage(input, inputMap, trainingData, 3);
    //return detectLanguageIterative(inputMap, trainingData);
}

QString LanguageDetector::detectLanguage(QString input, InputMap inputMap, QList<TrainingTextMap> tempTrainingData, int margin){

    if (tempTrainingData.size() == 1){

        qDebug()<<"detected Language: "<<tempTrainingData[0].getLanguageName();
        return tempTrainingData[0].getLanguageName();
    }
    else{
        int sz = tempTrainingData.size();
        for(int i = 0 ; i < sz; i++){
            qDebug()<<tempTrainingData[i].getLanguageName();
        }
        qDebug()<< sz;

        if (inputMap.nGramCount.isEmpty() || tempTrainingData.isEmpty()){

            QMessageBox q;
            q.setText("language not detected");
            q.exec();
            return "unknown";

        }
        else{

            int max = 0;
            QPair <QString, int> maxTupple = inputMap.extractMaxNGram();
            QString key = maxTupple.first;
            int value = maxTupple.second;
            qDebug()<<"maxPair: "<<maxTupple;

            for(int i = 0; i < sz; i++){
                int val = tempTrainingData[i].getValue(key);
                if(val != -1){
                    qDebug()<<"found in: "<<tempTrainingData[i].getLanguageName();
                    tempTrainingData[i].matchedNGram ++;
                    if(max < tempTrainingData[i].matchedNGram){
                        max = tempTrainingData[i].matchedNGram;
                        qDebug()<<"max : "<<max;
                    }
                }
                else
                    qDebug()<<"notfound in: "<<tempTrainingData[i].getLanguageName();
            }

            for ( int i = 0 ; i < tempTrainingData.size(); i++){
                if(tempTrainingData[i].matchedNGram + margin < max){
                    qDebug()<<"Removing: "<<tempTrainingData[i].getLanguageName();
                    tempTrainingData.removeAt(i);
                    --i;
                }
            }

            qDebug()<<"After removing";
            for(int i = 0 ; i < tempTrainingData.size(); i++){
                qDebug()<<tempTrainingData[i].getLanguageName();
            }

            if( sz == tempTrainingData.size() ){
                if( margin > 0 ){
                    qDebug()<<"margin: "<<margin;
                    margin --;
                    inputMap.add(key, value);
                }
            }

            return detectLanguage(input, inputMap, tempTrainingData, margin);

        }

    }
}


QString LanguageDetector::detectLanguageIterative(InputMap inputMap, QList<TrainingTextMap> tempTrainingData){
    QList<int> matches;
    int max = 0;
    for(int i = 0; i < inputMap.size(); i++){

        QPair<QString, int> current = inputMap.extractMaxNGram();
        QString key = current.first;
        int value = current.second;

        for(int j = 0 ; j < tempTrainingData.size(); j++){

            int found = tempTrainingData[j].getValue(key);
            if( found != -1 ){
                tempTrainingData[j].matchedNGram += value;
                if(max <= tempTrainingData[j].matchedNGram){
                    if(max == tempTrainingData[j].matchedNGram){
                        qDebug()<<"another match found"<<tempTrainingData[j].getLanguageName();
                        matches << j;
                    }
                    else{
                        qDebug()<<"match cleared by"<<tempTrainingData[j].getLanguageName();
                        max = tempTrainingData[j].matchedNGram;
                        matches.clear();
                        matches << j;
                    }
                }
            }
        }
    }
    if(matches.size() == 1){
        qDebug()<<"Detected Language itr: "<<tempTrainingData[matches[0]].getLanguageName();
        return tempTrainingData[matches[0]].getLanguageName();
    }
    else{
        qDebug()<<"Detected Language itr: unknown";
        return "unknown";
    }
}
