#include "training_text_map.h"

#include <QString>
#include <QStringList>


void TrainingTextMap::createTTMap(QString trainingText){
    QString punctuation = ",.?<>:;\"'!@#$%^&()-_\t\r";

    for(int i = 0; i < punctuation.size(); i++)
        trainingText.replace(punctuation.at(i)," ");
    trainingText.replace("\n"," ");
    trainingText.simplified();
    QStringList tokens = trainingText.split(' ',QString::SkipEmptyParts);

    for(int i = 0; i < tokens.size(); i++){
        QString temp = tokens.at(i);
        temp[0] = temp[0].toUpper();
        if( TTMap.value(temp, -1) == -1 ){
            //qDebug() << "adding key" << temp;
            TTMap[temp] = 1;
            //qDebug() << nGramFreqMap.value(temp);
        }
        else{
            TTMap[temp] = TTMap.value(temp)+ 1;
        }
    }

    QList <int> temp = TTMap.values();
    for(int i = 0 ;i < temp.size(); i++ ){

        if( !nGramCount.contains(temp.at(i)) )
            nGramCount << temp.at(i);
    }

    for(int i = 0; i < nGramCount.size(); i++){
        for(int j = i; j < nGramCount.size(); j++){
            if(nGramCount.at(i) < nGramCount.at(j)){
                int a = nGramCount.at(i), b = nGramCount.at(j);
                int tempn = a;
                nGramCount[i] = b;
                nGramCount[j] = tempn;
            }
        }
    }
}


QPair <QString, int> InputMap::extractMaxNGram(){

    if( !nGramCount.isEmpty()){
        int maxFq = nGramCount[0];
        QString key = TTMap.key(maxFq, "@");
        if ( key == "@" ){
            nGramCount.removeAt(0);
            return extractMaxNGram();
        }
        TTMap.remove(key);
        QPair <QString, int> max(key, maxFq);
        return max;
    }
    else{
        QPair <QString, int> max("@", -1);
        return max;
    }

}
