/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "PackageKitJob.h"

#include <KOSRelease>
#include <KShell>
#include <PackageKit/Daemon>
#include <PackageKit/Details>
#include <QDBusConnection>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QProcess>
#include <QRegularExpression>

#include "PackageKitConfirmationDialog.h"

void PackageKitJob::executeOperation(const QFileInfo &fileInfo, const QString &mimeType, bool install)
{
    if (!supportedPackagekitMimeTypes().contains(mimeType)) {
        Q_EMIT error(i18nc("@info", "The mime type %1 is not supported by the packagekit backend", mimeType));
        return;
    }

    if (install) {
        PackageKitConfirmationDialog dlg(fileInfo.absoluteFilePath());
        if (dlg.exec() == QDialog::Accepted) {
            packageKitInstall(fileInfo.absoluteFilePath());
        } else {
            Q_EMIT error(QString());
        }
    } else {
        packageKitUninstall(fileInfo.absoluteFilePath());
    }
}

void PackageKitJob::packageKitInstall(const QString &fileName)
{
    PackageKit::Transaction *transaction = PackageKit::Daemon::installFile(fileName, {});
    connect(transaction, &PackageKit::Transaction::finished, this, &PackageKitJob::transactionFinished);
    connect(transaction, &PackageKit::Transaction::errorCode, this, &PackageKitJob::transactionError);
}

void PackageKitJob::packageKitUninstall(const QString &fileName)
{
    PackageKit::Transaction *transaction = PackageKit::Daemon::getDetailsLocal(fileName);
    connect(transaction, &PackageKit::Transaction::details, this, [this](const PackageKit::Details &details) {
        removePackage(details.packageId());
    });
    connect(transaction, &PackageKit::Transaction::errorCode, this, &PackageKitJob::transactionError);
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
    QDBusMessage message =
        QDBusMessage::createMethodCall("org.freedesktop.PackageKit", "/org/freedesktop/PackageKit", "org.freedesktop.DBus.Properties", "Get");
    message.setArguments({"org.freedesktop.PackageKit", "MimeTypes"});
    QDBusMessage reply = QDBusConnection::systemBus().call(message);
    return reply.arguments().at(0).value<QDBusVariant>().variant().toStringList();
}
