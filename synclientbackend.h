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

    bool m_stop;
    QVariantMap m_currentParameters;
    void setParameter(const char *param, const QString &value);
    void setParameter(const char *param, int value);
};

#endif // SYNCLIENTBACKEND_H
