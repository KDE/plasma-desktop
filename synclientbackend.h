#ifndef SYNCLIENTBACKEND_H
#define SYNCLIENTBACKEND_H

#include <QVariantMap>

#include "touchpadbackend.h"

class QProcess;

class SynclientBackend : public TouchpadBackend
{
    Q_OBJECT
public:
    explicit SynclientBackend(QObject *parent, const QVariantList &);

    void applyConfig(const TouchpadParameters *);
    void getConfig(TouchpadParameters *, QStringList *supportedParameters = 0);
    bool test();

private:
    bool execSynclient(QProcess &process, const QString &arg);
    bool getParameters();

    QVariantMap m_currentParameters;
    QMap<QString, void (*)(QVariant &)> m_inputConv;
    QMap<QString, void (*)(QVariant &)> m_outputConv;
};

#endif // SYNCLIENTBACKEND_H
