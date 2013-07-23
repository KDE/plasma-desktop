#ifndef TOUCHPADBACKEND_H
#define TOUCHPADBACKEND_H

#include <QObject>
#include <QStringList>

#include <KGlobal>

class TouchpadParameters;

class KDE_EXPORT TouchpadBackend : public QObject
{
    Q_OBJECT
public:
    explicit TouchpadBackend(QObject *parent);

    static TouchpadBackend *self();

    virtual void applyConfig(const TouchpadParameters *) = 0;
    virtual void getConfig(TouchpadParameters *) = 0;
    virtual QStringList supportedParameters() = 0;

Q_SIGNALS:
    void error(const QString &);
};

#endif // TOUCHPADBACKEND_H
