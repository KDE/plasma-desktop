/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QFileInfoList>
#include <QDir>
#include <QDebug>
#include "ScriptJob.h"
#include "ScriptConfirmationDialog.h"

void ScriptJob::executeOperation(const QString &archive, Operation operation)
{
    const bool install = operation == Operation::Install;
    QString installerPath;
    const QFileInfoList archiveEntries = QDir(archive).entryInfoList(QDir::Files, QDir::Name);
    const QString scriptPrefix = install ? "install" : "uninstall";
    for (const auto &file : archiveEntries) {
        if (file.baseName() == scriptPrefix) {
            installerPath = file.absoluteFilePath();
            // If the name is exactly install/uninstall we immediately take it
            break;
        } else if (file.baseName().startsWith(scriptPrefix)) {
            installerPath = file.absoluteFilePath();
        }
    }
    // We want the user to be exactly aware of whats going on
    if (install || installerPath.isEmpty()) {
        ScriptConfirmationDialog dlg(installerPath, operation, archive);
        if (dlg.exec() == QDialog::Accepted) {
            if (installerPath.isEmpty()) {
                Q_EMIT finished(); // The "Mark entry as installed" button
            } else {
                const QString bashCommand = QStringLiteral("echo %1;%1 || $SHELL && echo %2")
                    .arg(KShell::quoteArg(installerPath), KShell::quoteArg(terminalCloseMessage(operation)));
                runScriptInTerminal(QStringLiteral("bash -c %1").arg(KShell::quoteArg(bashCommand)), archive);
            }
        } else {
            Q_EMIT error(QString());
        }
    }
}
