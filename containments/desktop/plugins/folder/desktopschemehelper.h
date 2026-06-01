/*
    SPDX-FileCopyrightText: 2022 Alexander Wilms <f.alexander.wilms@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DESKTOPSCHEMEHELPER_H
#define DESKTOPSCHEMEHELPER_H

#include <QObject>
#include <QStandardPaths>
#include <QString>
#include <qqmlregistration.h>

class DesktopSchemeHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    enum Options {
        TrailingSlash = 0,
        NoTrailingSlash,
    };

    explicit DesktopSchemeHelper(QObject *parent = nullptr);
    ~DesktopSchemeHelper() override;

    Q_INVOKABLE static QString getDesktopUrl(const QString &absoluteUrlString, const Options &opt = Options::TrailingSlash);
    Q_INVOKABLE static QString getFileUrl(const QString &UrlRelativeToDesktopString);
};

#endif
