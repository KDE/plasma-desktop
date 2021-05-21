/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "tastenbrett.h"

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>

QString Tastenbrett::path()
{
    static QString path;
    if (!path.isNull()) {
        return path;
    }

    // Find relative to KCM (when run from build dir)
    path = QStandardPaths::findExecutable("tastenbrett", {qEnvironmentVariable("QT_PLUGIN_PATH"), qApp->applicationDirPath()});
    if (!path.isNull()) {
        return path;
    }

    return QStandardPaths::findExecutable("tastenbrett");
}

bool Tastenbrett::exists()
{
    return !path().isNull();
}

void Tastenbrett::launch(const QString &model, const QString &layout, const QString &variant, const QString &options, const QString &title)
{
    if (!exists()) {
        return;
    }

    QProcess p;
    p.setProgram(path());
    QStringList args{"--model", model, "--layout", layout, "--variant", variant, "--options", options};
    if (!title.isEmpty()) {
        args << "-title" << title;
    }
    qDebug() << args;
    p.setArguments(args);
    p.setProcessChannelMode(QProcess::ForwardedChannels);
    p.startDetached();
}
