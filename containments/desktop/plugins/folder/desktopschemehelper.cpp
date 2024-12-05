/*
    SPDX-FileCopyrightText: 2022 Alexander Wilms <f.alexander.wilms@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "desktopschemehelper.h"

#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>

DesktopSchemeHelper::DesktopSchemeHelper(QObject *parent)
    : QObject(parent)
{
}

DesktopSchemeHelper::~DesktopSchemeHelper()
{
}

QString DesktopSchemeHelper::getDesktopUrl(const QString &absoluteUrlString)
{
    QUrl absoluteUrl(absoluteUrlString);
    QUrl absoluteFileUrl(absoluteUrlString);
    absoluteFileUrl.setScheme(QStringLiteral("file"));
    QString absoluteDesktopString(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0));
    QUrl absoluteDesktopUrl(absoluteDesktopString);
    absoluteDesktopUrl.setScheme(QStringLiteral("file"));

    if (absoluteUrl.scheme() != QStringLiteral("desktop") && (absoluteDesktopUrl.isParentOf(absoluteUrl) || absoluteDesktopUrl == absoluteFileUrl)) {
        // Argument is a desktop: URL, so the path relative to the desktop dir has to be determined
        QDir absoluteDesktopDir(absoluteDesktopString);
        QString relativePath = absoluteDesktopDir.relativeFilePath(absoluteUrl.toString(QUrl::RemoveScheme));

        // Clean up the desktop URL
        // Remove a trailing period
        if (relativePath.endsWith(QLatin1Char('.'))) {
            relativePath.chop(1);
        }

        QString relativeUrlString = QStringLiteral("desktop:/") + relativePath + QStringLiteral("/");

        // Collapse redundant slashes
        QRegularExpression slashRegex(QStringLiteral("/+"));
        relativeUrlString.replace(slashRegex, QStringLiteral("/"));

        return relativeUrlString;
    } else {
        // If the argument is not a desktop: URL, we don't need to resolve it.
        // Redundant slashes are collapsed anyway.
        QString absoluteUrlStringCopy(absoluteUrlString);
        // Don't collapse slashes immediately following the scheme, e.g. in smb://192.168.001.002
        QRegularExpression slashRegex(QStringLiteral("(!:)/+"));
        absoluteUrlStringCopy.replace(slashRegex, QStringLiteral("/"));

        return absoluteUrlStringCopy;
    }
}

QString DesktopSchemeHelper::getFileUrl(const QString &UrlRelativeToDesktopString)
{
    QString absoluteDesktopString(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0));
    QString absoluteFileUrl(UrlRelativeToDesktopString);
    if (absoluteFileUrl.startsWith(QStringLiteral("desktop:/"))) {
        absoluteFileUrl.replace(QStringLiteral("desktop:/"), QStringLiteral("desktop:"));
    }
    if (absoluteFileUrl.endsWith(QLatin1Char('.'))) {
        absoluteFileUrl.chop(1);
    }
    return absoluteFileUrl.replace(QStringLiteral("desktop:"), absoluteDesktopString + QStringLiteral("/"));
}
#include "moc_desktopschemehelper.cpp"
