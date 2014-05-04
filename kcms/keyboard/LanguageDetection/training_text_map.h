#ifndef TRAINING_TEXT_MAP_H
#define TRAINING_TEXT_MAP_H

#include <QString>
#include <QMap>
#include <QList>
#include <QStringList>
#include <QDebug>
#include <QPair>

class TrainingTextMap
{
    QString languageName;


protected:
    QMap <QString, int> TTMap;

public:
    QList<int> nGramCount;
    int matchedNGram, freq;

    TrainingTextMap(){
        matchedNGram = 0;
    }

    void createTTMap(QString trainingText);

    QString getLanguageName(){
        return languageName;
    }

    QString getKey(int value){
        return TTMap.key(value, "@");
    }

    int getValue(QString key){
        return TTMap.value(key, -1);
    }

    QStringList getKeys(){
        return TTMap.keys();
    }

    void setLanguageName(QString name){
        languageName = name;
    }

};


class InputMap : public TrainingTextMap {


public :
    QPair<QString, int> extractMaxNGram();

    void add(QString key, int value){
        if(! nGramCount.contains(value))
            nGramCount<<value;

        TTMap[key] = value;
    }

    int size(){
        return TTMap.size();
    }
};

#endif // TRAINING_TEXT_MAP_H
