/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "AbstractJob.h"

#ifndef PACKAGEKITJOB_H
#define PACKAGEKITJOB_H

#include "config-workspace.h"

#ifdef HAVE_PACKAGEKIT
#include <PackageKit/Transaction>
#endif

class PackageKitJob : public AbstractJob
{
Q_OBJECT

public:
    void executeOperation(const QFileInfo &fileInfo, const QString &mimeType, Operation operation) override;

#ifdef HAVE_PACKAGEKIT
private:
    QStringList supportedPackagekitMimeTypes();

private Q_SLOTS:
    void packageKitInstall(const QString &fileName, const QString &mimeType);
    void packageKitUninstall(const QString &fileName);
    void removePackage(const QString &packageId);

    void transactionError(PackageKit::Transaction::Error, const QString &details);
    void transactionFinished(PackageKit::Transaction::Exit status, uint);
#endif
};

#endif //PACKAGEKITJOB_H
