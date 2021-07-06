/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "AbstractJob.h"

#include <KLocalizedString>
#include <KTerminalLauncherJob>

void AbstractJob::runScriptInTerminal(const QString &script, const QString &pwd)
{
    auto job = new KTerminalLauncherJob(script);
    job->setWorkingDirectory(pwd);
    connect(job, &KJob::result, [this, job]() {
        if (job->error()) {
            Q_EMIT error(xi18nc("@info:status", "Failed to run install script in terminal <message>%1</message>", job->errorString()));
        } else {
            Q_EMIT finished();
        }
    });
    job->start();
}

QString AbstractJob::terminalCloseMessage(bool install)
{
    if (install) {
        return i18nc("@info", "Installation executed successfully, you may now close this window");
    } else {
        return i18nc("@info", "Uninstallation executed successfully, you may now close this window");
    }
}

#include "AbstractJob.moc"
