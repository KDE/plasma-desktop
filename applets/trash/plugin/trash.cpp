/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "trash.h"

#include <QApplication>
#include <QFileInfo>

#include <KNotificationJobUiDelegate>

#include <KIO/DeleteOrTrashJob>
#include <KIO/OpenUrlJob>

Trash::Trash(QObject *parent)
    : QObject(parent)
{
}

bool Trash::emptying() const
{
    return m_emptying;
}

void Trash::openTrash()
{
    auto *job = new KIO::OpenUrlJob(QUrl(QStringLiteral("trash:/")));
    job->setUiDelegate(new KNotificationJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled));
    job->start();
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
    connect(job, &KIO::DeleteOrTrashJob::started, this, [this] {
        m_emptying = true;
        Q_EMIT emptyingChanged(true);
    });
    connect(job, &KIO::DeleteOrTrashJob::finished, this, [this] {
        m_emptying = false;
        Q_EMIT emptyingChanged(false);
    });
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
