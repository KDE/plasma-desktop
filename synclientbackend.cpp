#include "synclientbackend.h"

#include <QProcess>
#include <QTextStream>
#include <QStringList>
#include <qmath.h>

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

bool SynclientBackend::execSynclient(QProcess &process, const QStringList &args)
{
    static const QString synclient("synclient");
    process.start(synclient, args, QIODevice::ReadOnly);

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
    static const QStringList listParams("-l");
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

    QStringList args;
    Q_FOREACH(KConfigSkeletonItem *i, p->items()) {
        QString option(i->name());

        QVariant variantValue(i->property());
        if (variantValue.type() == QVariant::Bool) {
            variantValue.convert(QVariant::Int);
        }

        QString value(variantValue.toString());
        if (!m_currentParameters.contains(option) ||
                m_currentParameters[option].toString() == value)
        {
            continue;
        }

        option.append('=');
        option.append(value);
        args.append(option);
    }

    QProcess process;
    execSynclient(process, args);
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
        if (!value.convert(item->property().type())) {
            continue;
        }

        item->setProperty(value);

        if (supportedParameters) {
            supportedParameters->append(i.key());
        }
    }
}
