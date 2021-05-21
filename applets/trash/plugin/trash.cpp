/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "trash.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFileInfo>

#include <KIO/CopyJob>
#include <KIO/EmptyTrashJob>
#include <KIO/Job>
#include <KIO/JobUiDelegate>

Trash::Trash(QObject *parent)
    : QObject(parent)
{
}

void Trash::trashUrls(const QList<QUrl> &urls)
{
    KIO::JobUiDelegate uiDelegate;
    if (uiDelegate.askDeleteConfirmation(urls, KIO::JobUiDelegate::Trash, KIO::JobUiDelegate::DefaultConfirmation)) {
        KIO::Job *job = KIO::trash(urls);
        job->uiDelegate()->setAutoErrorHandlingEnabled(true);
    }
}

void Trash::emptyTrash()
{
    KIO::JobUiDelegate uiDelegate;
    uiDelegate.setWindow(QApplication::desktop());
    if (uiDelegate.askDeleteConfirmation(QList<QUrl>(), KIO::JobUiDelegate::EmptyTrash, KIO::JobUiDelegate::DefaultConfirmation)) {
        KIO::Job *job = KIO::emptyTrash();
        job->uiDelegate()->setAutoErrorHandlingEnabled(true);
    }
}

bool Trash::canBeTrashed(const QUrl &url) const
{
    return url.isValid() && url.isLocalFile() && QFileInfo(url.toLocalFile()).isWritable();
}

QList<QUrl> Trash::trashableUrls(const QList<QUrl> &urls) const
{
    QList<QUrl> validUrls = urls;

    QMutableListIterator<QUrl> it(validUrls);

    while (it.hasNext()) {
        if (!canBeTrashed(it.next())) {
            it.remove();
        }
    }

    return validUrls;
}
