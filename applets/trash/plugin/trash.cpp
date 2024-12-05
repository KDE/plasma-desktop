/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "trash.h"

#include <QApplication>
#include <QFileInfo>

#include <KIO/DeleteOrTrashJob>

Trash::Trash(QObject *parent)
    : QObject(parent)
{
}

void Trash::trashUrls(const QList<QUrl> &urls)
{
    using Iface = KIO::AskUserActionInterface;
    auto *job = new KIO::DeleteOrTrashJob(urls, Iface::Trash, Iface::DefaultConfirmation, this);
    job->start();
}

void Trash::emptyTrash()
{
    using Iface = KIO::AskUserActionInterface;
    auto *job = new KIO::DeleteOrTrashJob({}, Iface::EmptyTrash, Iface::DefaultConfirmation, this);
    job->start();
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

#include "moc_trash.cpp"
