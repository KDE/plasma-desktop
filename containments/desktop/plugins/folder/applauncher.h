/*
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QUrl>

class AppLauncher : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE void openUrl(const QUrl &url);

private:
};
