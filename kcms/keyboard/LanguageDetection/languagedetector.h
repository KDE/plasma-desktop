#ifndef LANGUAGEDETECTOR_H
#define LANGUAGEDETECTOR_H

#include "training_text_map.h"

#include <QString>
#include <QDir>
#include <QList>
#include <QStringList>


class LanguageDetector
{
    QString langdataPath;
    QDir langDir;
    QStringList langList;
    QList<TrainingTextMap> trainingData;

    void createTrainingData();

public:

    LanguageDetector();

    QString detectLanguage(QString input);

    QString detectLanguage(QString input, InputMap inputMap, QList <TrainingTextMap> tempTrainingData, int margin);

    QString detectLanguageIterative(InputMap inputMap, QList <TrainingTextMap> tempTrainingData);
};

#endif // LANGUAGEDETECTOR_H
