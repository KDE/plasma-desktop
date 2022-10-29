/*
    SPDX-FileCopyrightText: 2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "draghelper.h"

#include <QUrl>

#include <KWindowSystem>

#include "backend.h"

DragHelper::DragHelper(QQuickItem *parent)
    : QQuickItem(parent)
{
}

DragHelper::~DragHelper()
{
}

QVariantMap DragHelper::generateMimeData(const QString &mimeType, const QVariant &mimeData, const QUrl &url) const
{
    QVariantMap mimedata;

    const QString &taskUrlData = Backend::tryDecodeApplicationsUrl(url).toString();
    mimedata.insert(QStringLiteral("text/x-orgkdeplasmataskmanager_taskurl"), taskUrlData);

    // Workaround for https://bugreports.qt.io/browse/QTBUG-71922
    QString idString;
    if (KWindowSystem::isPlatformX11()) {
        const WId *const idData = reinterpret_cast<WId *>(mimeData.toByteArray().data());
        idString = QStringLiteral("strnum-") % QString::number(*idData);
    } else if (KWindowSystem::isPlatformWayland()) {
        idString = QString::fromLatin1(mimeData.toByteArray());
    }
    mimedata.insert(mimeType, idString);
    mimedata.insert(QStringLiteral("application/x-orgkdeplasmataskmanager_taskbuttonitem"), idString);

    return mimedata;
}
