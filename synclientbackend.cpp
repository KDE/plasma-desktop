#include "synclientbackend.h"

#include <QProcess>
#include <QTextStream>
#include <QStringList>
#include <qmath.h>

#include <KPluginFactory>

#include "touchpadparameters.h"

K_PLUGIN_FACTORY(TouchpadBackendFactory, registerPlugin<SynclientBackend>();)
K_EXPORT_PLUGIN(TouchpadBackendFactory("synclient_touchpad"))

static const double speedScale = 100.0;
static const double speedBase = 10.0;
static const double accelFactorScale = 1000000.0;
static const double accelFactorBase = 10.0;

static void speedInputConv(QVariant &v)
{
    if (!v.convert(QVariant::Double)) {
        return;
    }
    v = QVariant(qLn(1.0 + v.toDouble() * speedBase) * speedScale);
}

static void speedOutputConv(QVariant &v)
{
    if (!v.convert(QVariant::Double)) {
        return;
    }
    v = QVariant((qExp(v.toDouble() / speedScale) - 1.0) / speedBase);
}

static void accelFactorInputConv(QVariant &v)
{
    if (!v.convert(QVariant::Double)) {
        return;
    }
    v = QVariant(qLn(1.0 + v.toDouble() * accelFactorBase) * accelFactorScale);
}

static void accelFactorOutputConv(QVariant &v)
{
    if (!v.convert(QVariant::Double)) {
        return;
    }
    v = QVariant((qExp(v.toDouble() / accelFactorScale) - 1.0)
                 / accelFactorBase);
}

SynclientBackend::SynclientBackend(QObject *parent, const QVariantList &)
    : TouchpadBackend(parent)
{
    m_inputConv["MinSpeed"] = speedInputConv;
    m_outputConv["MinSpeed"] = speedOutputConv;
    m_inputConv["MaxSpeed"] = speedInputConv;
    m_outputConv["MaxSpeed"] = speedOutputConv;

    m_inputConv["AccelFactor"] = accelFactorInputConv;
    m_outputConv["AccelFactor"] = accelFactorOutputConv;
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

void SynclientBackend::applyConfig(const TouchpadParameters *p)
{
    if (!getParameters()) {
        return;
    }

    Q_FOREACH(KConfigSkeletonItem *i, p->items()) {
        QString option(i->name());

        QVariant variantValue(i->property());
        if (variantValue.type() == QVariant::Bool) {
            variantValue.convert(QVariant::Int);
        }
        if (m_outputConv.contains(option)) {
            m_outputConv[option](variantValue);
        }

        QString value(variantValue.toString());
        if (!m_currentParameters.contains(option) ||
                m_currentParameters[option].toString() == value)
        {
            continue;
        }

        option.append('=');
        option.append(value);

        QProcess process;
        execSynclient(process, option);
    }
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

    for (QVariantMap::ConstIterator i = m_currentParameters.begin();
         i != m_currentParameters.end(); i++)
    {
        KConfigSkeletonItem *item = p->findItem(i.key());
        if (!item) {
            continue;
        }

        QVariant value(i.value());
        if (m_inputConv.contains(i.key())) {
            m_inputConv[i.key()](value);
        }

        if (!value.convert(item->property().type())) {
            continue;
        }

        item->setProperty(value);

        if (supportedParameters) {
            supportedParameters->append(i.key());
        }
    }
}
