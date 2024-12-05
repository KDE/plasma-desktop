/*
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "applauncher.h"

#include <KIO/OpenUrlJob>
#include <KNotificationJobUiDelegate>

void AppLauncher::openUrl(const QUrl &url)
{
    auto *job = new KIO::OpenUrlJob(url);
    job->setUiDelegate(new KNotificationJobUiDelegate(KJobUiDelegate::AutoErrorHandlingEnabled));
    job->start();
}

#include "moc_applauncher.cpp"
