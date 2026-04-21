/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "trash.h"

#include <QFileInfo>

#include <KCoreDirLister>
#include <KIO/DeleteOrTrashJob>
#include <KIO/OpenUrlJob>
#include <KNotificationJobUiDelegate>

using namespace Qt::Literals;

Trash::Trash(QObject *parent)
    : QObject(parent)
{
    auto lister = new KCoreDirLister(this);

    connect(lister, &KCoreDirLister::itemsAdded, this, [this](const QUrl & /*directoryUrl*/, const KFileItemList &items) {
        m_count += items.size();
        Q_EMIT countChanged();
    });

    connect(lister, &KCoreDirLister::itemsDeleted, this, [this](const KFileItemList &items) {
        m_count -= items.count();
        Q_EMIT countChanged();
    });

    lister->openUrl(QUrl(u"trash:/"_s));
}

bool Trash::emptying() const
{
    return m_emptying;
}

int Trash::count() const
{
    return m_count;
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
