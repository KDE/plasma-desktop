#ifndef TOUCHPADBACKEND_H
#define TOUCHPADBACKEND_H

#include <QObject>
#include <QVariantMap>

class QProcess;
class TouchpadParameters;

class TouchpadBackend : public QObject
{
    Q_OBJECT

public:
    static TouchpadBackend *self();

    void applyConfig(const TouchpadParameters *);
    void getConfig(TouchpadParameters *);

    bool test();

Q_SIGNALS:
    void error(const QString &);

private:
    bool execSynclient(QProcess &process, const QString &arg);

    bool getParameters();

    bool m_stop;
    QVariantMap m_currentParameters;
    void setParameter(const char *param, const QString &value);
    void setParameter(const char *param, int value);

    TouchpadBackend();
    Q_DISABLE_COPY(TouchpadBackend)
};

#endif // TOUCHPADBACKEND_H
