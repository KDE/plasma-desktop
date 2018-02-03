/*
 *   Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "trash.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFileInfo>

#include <KIO/Job>
#include <KIO/CopyJob>
#include <KIO/EmptyTrashJob>
#include <KIO/JobUiDelegate>

Trash::Trash(QObject *parent) : QObject(parent)
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
