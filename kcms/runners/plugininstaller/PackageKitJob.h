/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "AbstractJob.h"

#ifndef PACKAGEKITJOB_H
#define PACKAGEKITJOB_H

#include <PackageKit/Transaction>

class PackageKitJob : public AbstractJob
{
    Q_OBJECT

public:
    void executeOperation(const QFileInfo &fileInfo, const QString &mimeType, bool install) override;

private:
    QStringList supportedPackagekitMimeTypes();

private Q_SLOTS:
    void packageKitInstall(const QString &fileName);
    void packageKitUninstall(const QString &fileName);
    void removePackage(const QString &packageId);

    void transactionError(PackageKit::Transaction::Error, const QString &details);
    void transactionFinished(PackageKit::Transaction::Exit status, uint);
};

#endif // PACKAGEKITJOB_H
