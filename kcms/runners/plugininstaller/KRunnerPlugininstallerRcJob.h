/*
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "AbstractJob.h"
struct InstallerInfo;

class KRunnerPluginInstallerRcJob : public AbstractJob
{
public:
    explicit KRunnerPluginInstallerRcJob(bool skipConfigmDialog)
        : m_skipConfirmDialog(skipConfigmDialog)
    {
    }
    void executeOperation(const QFileInfo &fileInfo, const QString &mimeType, bool install) override;

private:
    void doInstall(const InstallerInfo &info);
    void doUninstall(const InstallerInfo &info);
    const bool m_skipConfirmDialog;
};
