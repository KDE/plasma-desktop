/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "AbstractJob.h"

#include <QDebug>

void AbstractJob::runScriptInTerminal(const QString &script, const QString &pwd)
{
    KConfigGroup confGroup(KSharedConfig::openConfig(), "General");
    QString exec = confGroup.readPathEntry("TerminalApplication", QStringLiteral("konsole"));

    if (exec == QLatin1String("konsole")) {
        exec += QLatin1String(" --noclose --separate");
    } else if (exec == QLatin1String("xterm")) {
        exec += QLatin1String(" -hold");
    }
    exec += QLatin1String(" -e ");
    exec += KShell::quoteArg(script);

    QProcess *process = new QProcess();
    process->setWorkingDirectory(pwd);
    // We don't know if the entry read from the config contains options
    // so we just split it at the end
    QStringList split = KShell::splitArgs(exec);
    process->setProgram(split.takeFirst());
    process->setArguments(split);
    process->start();
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, process] (int, QProcess::ExitStatus exitStatus) {
        process->deleteLater();
        if (exitStatus == QProcess::NormalExit) {
            Q_EMIT finished();
        } else {
            Q_EMIT error(QString());
        }
    });
}

QString AbstractJob::terminalCloseMessage(Operation operation)
{
    return operation == Operation::Install
           ? i18nc("@info", "Installation executed successfully, you may now close this window")
           : i18nc("@info", "Uninstallation executed successfully, you may now close this window");
}

#include "AbstractJob.moc"
