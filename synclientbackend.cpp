#include "synclientbackend.h"

#include <QProcess>
#include <QTextStream>
#include <QStringList>

#include <KPluginFactory>

#include "touchpadparameters.h"

K_PLUGIN_FACTORY(TouchpadBackendFactory, registerPlugin<SynclientBackend>();)
K_EXPORT_PLUGIN(TouchpadBackendFactory("synclient_touchpad"))

SynclientBackend::SynclientBackend(QObject *parent, const QVariantList &)
    : TouchpadBackend(parent)
{
}

bool SynclientBackend::test()
{
    return getParameters();
}

bool SynclientBackend::execSynclient(QProcess &process, const QString &arg)
{
    static const QString synclient("synclient");
    process.start(synclient, QStringList(arg), QIODevice::ReadOnly);

    static const int timeout = 1000;
    if (!process.waitForFinished(timeout)) {
        process.kill();

        QString errorString(synclient);
        errorString.append(": ");
        errorString.append(process.errorString());
        Q_EMIT error(errorString);
        return false;
    }
    if (process.exitStatus() != QProcess::NormalExit ||
            process.exitCode() != EXIT_SUCCESS)
    {
        QTextStream stream(&process);
        QString errorString(synclient);
        if (!process.errorString().isEmpty()) {
            errorString.append(": ");
            errorString.append(process.errorString());
        }
        QString output(stream.readAll());
        if (!output.isEmpty()) {
            errorString.append(": ");
            errorString.append(output);
        }
        Q_EMIT error(errorString);
        return false;
    }
    return true;
}

void SynclientBackend::setParameter(const char *param, const QString &value)
{
    QString option(param);
    if (!m_currentParameters.contains(option) ||
            m_currentParameters[option].toString() == value)
    {
        return;
    }

    option.append('=');
    option.append(value);

    QProcess process;
    execSynclient(process, option);
}

void SynclientBackend::setParameter(const char *param, int value)
{
    setParameter(param, QString::number(value));
}

void SynclientBackend::setParameter(const char *param, double value)
{
    setParameter(param, QString::number(value, 'g', 10));
}

bool SynclientBackend::getParameters()
{
    m_currentParameters.clear();

    QProcess process;
    static const QString listParams("-l");
    if (!execSynclient(process, listParams))
    {
        return false;
    }

    QStringList queue;
    QTextStream input(&process);
    static const QString equals("=");
    enum {
        QueueKey, QueueEquals, QueueValue, QueueSize
    };
    Q_FOREVER {
        while (queue.size() >= QueueSize) {
            queue.pop_front();
        }
        while (queue.size() < QueueSize) {
            QString word;
            input >> word;
            if (input.status() != QTextStream::Ok) {
                return !m_currentParameters.isEmpty();
            }
            queue.push_back(word);
        }
        if (queue[QueueEquals] != equals) {
            continue;
        }
        m_currentParameters.insert(queue[QueueKey], queue[QueueValue]);
    }
}

template<typename T, typename T2>
void setParameterValue(const QVariantMap &m, const char *name,
                       TouchpadParameters *p,
                       void (TouchpadParameters::* setter)(T2),
                       const char *kConfigName, QStringList *supportedParams)
{
    QString param(name);
    if (!m.contains(param)) {
        return;
    }
    const QVariant &v = m[param];
    if (v.canConvert<T>()) {
        (p->*setter)(static_cast<T2>(qvariant_cast<T>(v)));
        if (supportedParams) {
            supportedParams->append(kConfigName);
        }
    }
}

void SynclientBackend::applyConfig(const TouchpadParameters *p)
{
    if (!getParameters()) {
        return;
    }

    setParameter("TapButton1", p->tapButton1());
    setParameter("TapButton2", p->tapButton2());
    setParameter("TapButton3", p->tapButton3());

    setParameter("VertEdgeScroll", p->vertEdgeScroll());
    setParameter("VertTwoFingerScroll", p->vertTwoFingerScroll());

    setParameter("HorizEdgeScroll", p->horizEdgeScroll());
    setParameter("HorizTwoFingerScroll", p->horizTwoFingerScroll());

    setParameter("MinSpeed", p->minSpeed());
    setParameter("MaxSpeed", p->maxSpeed());
    setParameter("AccelFactor", p->accelFactor());
}

void SynclientBackend::getConfig(TouchpadParameters *p,
                                 QStringList *supportedParameters)
{
    if (supportedParameters) {
        supportedParameters->clear();
    }

    if (!getParameters()) {
        return;
    }

#define TOUCHPAD_PARAM(type, kconfigName, xName) \
    setParameterValue<type>(m_currentParameters, xName, p, \
    &TouchpadParameters::set##kconfigName, #kconfigName, supportedParameters)
#define TOUCHPAD_PARAM_SAME(type, name) TOUCHPAD_PARAM(type, name, #name)

    TOUCHPAD_PARAM_SAME(int, TapButton1);
    TOUCHPAD_PARAM_SAME(int, TapButton2);
    TOUCHPAD_PARAM_SAME(int, TapButton3);

    TOUCHPAD_PARAM_SAME(int, VertEdgeScroll);
    TOUCHPAD_PARAM_SAME(int, VertTwoFingerScroll);
    TOUCHPAD_PARAM_SAME(int, HorizEdgeScroll);
    TOUCHPAD_PARAM_SAME(int, HorizTwoFingerScroll);

    TOUCHPAD_PARAM_SAME(double, MinSpeed);
    TOUCHPAD_PARAM_SAME(double, MaxSpeed);
    TOUCHPAD_PARAM_SAME(double, AccelFactor);

#undef TOUCHPAD_PARAM_SAME
#undef TOUCHPAD_PARAM
}
