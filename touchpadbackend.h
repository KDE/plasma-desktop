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

    void applyConfig(const TouchpadParameters *);
    void getConfig(TouchpadParameters *);
    QStringList supportedParameters();

Q_SIGNALS:
    void error(const QString &);

protected:
    virtual bool init() = 0;
    virtual void internalApplyConfig(const TouchpadParameters *) = 0;
    virtual void internalGetConfig(TouchpadParameters *) = 0;
    virtual QStringList internalSupportedParameters() = 0;

private:
    bool tryInit();
    bool m_triedInit, m_initResult;
};

#endif // TOUCHPADBACKEND_H
