/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ZypperRPMJob.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QProcess>
#include <KShell>
#include <KLocalizedString>

void ZypperRPMJob::executeOperation(const QFileInfo &fileInfo, const QString &mimeType, Operation operation)
{
    Q_UNUSED(mimeType)

    QStringList args;
    if (operation == Operation::Install) {
        args = QStringList{"zypper", "install", fileInfo.absoluteFilePath()};
    } else {
        QProcess rpmInfoProcess;
        rpmInfoProcess.start(QStringLiteral("rpm"), {"-qi", fileInfo.absoluteFilePath()});
        rpmInfoProcess.waitForFinished();
        const QString rpmInfo = rpmInfoProcess.readAll();
        const auto infoMatch = QRegularExpression(QStringLiteral("Name *: (.+)")).match(rpmInfo);
        if (!infoMatch.hasMatch()) {
            Q_EMIT error(i18nc("@info", "Could not resolve package name of %1", fileInfo.baseName()));
        }
        const QString rpmPackageName = KShell::quoteArg(infoMatch.captured(1));
        args = QStringList{"zypper", "remove", rpmPackageName};
    }
    QProcess *process = new QProcess(this);
    process->start(QStringLiteral("pkexec"), args);
    connectSignals(process);
}
