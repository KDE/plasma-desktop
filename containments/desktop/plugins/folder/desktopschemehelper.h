/*
    SPDX-FileCopyrightText: 2022 Alexander Wilms <f.alexander.wilms@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DESKTOPSCHEMEHELPER_H
#define DESKTOPSCHEMEHELPER_H

#include "folderplugin_private_export.h"
#include <QObject>
#include <QStandardPaths>
#include <QString>

class FOLDERPLUGIN_TESTS_EXPORT DesktopSchemeHelper : public QObject
{
    Q_OBJECT

public:
    explicit DesktopSchemeHelper(QObject *parent = nullptr);
    ~DesktopSchemeHelper() override;

    Q_INVOKABLE static QString getDesktopUrl(const QString &absoluteUrlString);
    Q_INVOKABLE static QString getFileUrl(const QString &UrlRelativeToDesktopString);
};

#endif
