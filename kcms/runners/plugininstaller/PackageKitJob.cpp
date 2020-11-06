/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "PackageKitJob.h"

#include <QFileInfo>
#include <KShell>
#include <PackageKit/Daemon>
#include <PackageKit/Details>
#include <QRegularExpression>
#include <KOSRelease>
#include <QMimeDatabase>
#include <QDBusConnection>
#include <QProcess>

#include "PackageKitConfirmationDialog.h"

void PackageKitJob::executeOperation(const QFileInfo &fileInfo, const QString &mimeType, Operation operation)
{
    if (!supportedPackagekitMimeTypes().contains(mimeType)) {
        Q_EMIT error(i18nc("@info", "The mime type %1 is not supported by the packagekit backend", mimeType));
        return;
    }

    if (operation == Operation::Install) {
        PackageKitConfirmationDialog dlg(fileInfo.absoluteFilePath());
        if (dlg.exec() == QDialog::Accepted) {
            packageKitInstall(fileInfo.absoluteFilePath(), mimeType);
        } else {
            Q_EMIT error(QString());
        }
    } else {
        packageKitUninstall(fileInfo.absoluteFilePath(), mimeType);
    }
}

void PackageKitJob::packageKitInstall(const QString &fileName, const QString &mimeType)
{
    QFileInfo fileInfo(fileName);
    if (mimeType == QLatin1String("application/x-rpm") && KOSRelease().idLike().contains(u"suse")) {
        const QString zypperCommand = QStringLiteral("sudo zypper install %1")
            .arg(KShell::quoteArg(fileInfo.absoluteFilePath()));
        const QString command = QStringLiteral("bash -c \"echo %1;%1 && echo %2\"")
            .arg(zypperCommand, KShell::quoteArg(terminalCloseMessage(Operation::Install)));
        runScriptInTerminal(command, fileInfo.absolutePath());
    } else {
        PackageKit::Transaction *transaction = PackageKit::Daemon::installFile(fileName, {});
        connect(transaction, &PackageKit::Transaction::finished, this, &PackageKitJob::transactionFinished);
        connect(transaction, &PackageKit::Transaction::errorCode, this, &PackageKitJob::transactionError);
    }
}

void PackageKitJob::packageKitUninstall(const QString &fileName, const QString &mimeType)
{
    // On OpenSUSE packagekit can't look up the package details of a file, so we have to do this manually
    if (mimeType == QLatin1String("application/x-rpm") && KOSRelease().idLike().contains(u"suse")) {
        QProcess rpmInfoProcess;
        rpmInfoProcess.start(QStringLiteral("rpm"), {"-qi", fileName});
        rpmInfoProcess.waitForFinished();
        const QString rpmInfo = rpmInfoProcess.readAll();
        const auto infoMatch = QRegularExpression(QStringLiteral("Name *: (.+)")).match(rpmInfo);
        if (!infoMatch.hasMatch()) {
            Q_EMIT error(i18nc("@info", "Could not resolve package name of %1", fileName));
        }
        const QString rpmPackageName = KShell::quoteArg(infoMatch.captured(1));
        PackageKit::Transaction *transaction = PackageKit::Daemon::resolve(rpmPackageName);
        connect(transaction, &PackageKit::Transaction::package,
                this, [this](PackageKit::Transaction::Info, const QString &packageId, const QString &) {
                    removePackage(packageId);
                });
    } else {
        PackageKit::Transaction *transaction = PackageKit::Daemon::getDetailsLocal(fileName);
        connect(transaction, &PackageKit::Transaction::details,
            this, [this](const PackageKit::Details &details) { removePackage(details.packageId()); });
        connect(transaction, &PackageKit::Transaction::errorCode, this, &PackageKitJob::transactionError);
    }
}

void PackageKitJob::removePackage(const QString &packageId)
{
    PackageKit::Transaction *transaction = PackageKit::Daemon::removePackage(packageId);
    connect(transaction, &PackageKit::Transaction::finished, this, &PackageKitJob::transactionFinished);
    connect(transaction, &PackageKit::Transaction::errorCode, this, &PackageKitJob::transactionError);
}

void PackageKitJob::transactionError(PackageKit::Transaction::Error, const QString &details)
{
    Q_EMIT error(details);
}

void PackageKitJob::transactionFinished(PackageKit::Transaction::Exit status, uint)
{
    if (status == PackageKit::Transaction::ExitSuccess) {
        Q_EMIT finished();
    }
}

QStringList PackageKitJob::supportedPackagekitMimeTypes()
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.PackageKit",
                                                          "/org/freedesktop/PackageKit",
                                                          "org.freedesktop.DBus.Properties",
                                                          "Get");
    message.setArguments({"org.freedesktop.PackageKit", "MimeTypes"});
    QDBusMessage reply = QDBusConnection::systemBus().call(message);
    return reply.arguments().at(0).value<QDBusVariant>().variant().toStringList();
}
