/*
    Copyright 2019 Harald Sitter <sitter@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tastenbrett.h"

#include <QCoreApplication>
#include <QProcess>
#include <QStandardPaths>
#include <QDebug>

QString Tastenbrett::path()
{
    static QString path;
    if (!path.isNull()) {
        return path;
    }

    // Find relative to KCM (when run from build dir)
    path = QStandardPaths::findExecutable("tastenbrett", { qEnvironmentVariable("QT_PLUGIN_PATH"),
                                                           qApp->applicationDirPath() });
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
    QStringList args { "--model", model, "--layout", layout, "--variant", variant, "--options", options };
    if (!title.isEmpty()) {
        args << "-title" << title;
    }
    qDebug() << args;
    p.setArguments(args);
    p.setProcessChannelMode(QProcess::ForwardedChannels);
    p.startDetached();
}
