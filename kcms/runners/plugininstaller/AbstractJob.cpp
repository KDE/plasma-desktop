/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "AbstractJob.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KShell>

#include <QDebug>
#include <QProcess>

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

    QProcess *process = new QProcess(this);
    process->setWorkingDirectory(pwd);
    // We don't know if the entry read from the config contains options
    // so we just split it at the end
    QStringList split = KShell::splitArgs(exec);
    const QString program = split.takeFirst();
    process->setProgram(program);
    process->setArguments(split);
    process->start();
    connectSignals(process);
}

QString AbstractJob::terminalCloseMessage(bool install)
{
    if (install) {
        return i18nc("@info", "Installation executed successfully, you may now close this window");
    } else {
        return i18nc("@info", "Uninstallation executed successfully, you may now close this window");
    }
}

void AbstractJob::connectSignals(QProcess *process)
{
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this](int, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit) {
            Q_EMIT finished();
        }
    });

    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError) {
        Q_EMIT error(i18nc("@info", "Failed to run install script in terminal \"%1\"", process->program()));
    });
}

#include "AbstractJob.moc"
