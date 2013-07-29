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

    virtual bool applyConfig(const TouchpadParameters *) = 0;
    virtual bool getConfig(TouchpadParameters *) = 0;
    virtual const QStringList &supportedParameters() const = 0;
    virtual const QString &errorString() const = 0;
};

#endif // TOUCHPADBACKEND_H
