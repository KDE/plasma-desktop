#include "touchpadbackend.h"

#include <QProcess>
#include <QTextStream>
#include <QStringList>

#include "touchpadparameters.h"

TouchpadBackend::TouchpadBackend()
{
}

TouchpadBackend *TouchpadBackend::self()
{
    static TouchpadBackend instance;
    return &instance;
}

bool TouchpadBackend::execSynclient(QProcess &process, const QString &arg)
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

void TouchpadBackend::setParameter(const char *param, const QString &value)
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

void TouchpadBackend::setParameter(const char *param, int value)
{
    setParameter(param, QString::number(value));
}

bool TouchpadBackend::getParameters()
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

void TouchpadBackend::applyConfig(const TouchpadParameters *p)
{
    if (!getParameters()) {
        return;
    }

    setParameter("TapButton1", p->oneFingerTap());
    setParameter("TapButton2", p->twoFingerTap());
    setParameter("TapButton3", p->threeFingerTap());

    setParameter("VertEdgeScroll", p->vertEdgeScroll());
    setParameter("VertTwoFingerScroll", p->vertTwoFingerScroll());

    setParameter("HorizEdgeScroll", p->horizEdgeScroll());
    setParameter("HorizTwoFingerScroll", p->horizTwoFingerScroll());
}

void TouchpadBackend::getConfig(TouchpadParameters *p,
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

    TOUCHPAD_PARAM(int, OneFingerTap, "TapButton1");
    TOUCHPAD_PARAM(int, TwoFingerTap, "TapButton2");
    TOUCHPAD_PARAM(int, ThreeFingerTap, "TapButton3");

#define TOUCHPAD_PARAM_SAME(type, name) TOUCHPAD_PARAM(type, name, #name)
    TOUCHPAD_PARAM_SAME(int, VertEdgeScroll);
    TOUCHPAD_PARAM_SAME(int, VertTwoFingerScroll);
    TOUCHPAD_PARAM_SAME(int, HorizEdgeScroll);
    TOUCHPAD_PARAM_SAME(int, HorizTwoFingerScroll);

#undef TOUCHPAD_PARAM_SAME
#undef TOUCHPAD_PARAM
}

bool TouchpadBackend::test()
{
    return getParameters();
}
