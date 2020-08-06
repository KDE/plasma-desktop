/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2011 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), Trolltech ASA 
    (or its successors, if any) and the KDE Free Qt Foundation, which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public 
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QStringList>
#include <KComponentData>
#include <KGlobal>
#include <KStandardDirs>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QSettings settings(QStringLiteral("Trolltech"));
    QString qversion = qVersion();
    if (qversion.count('.') > 1) {
        qversion.truncate(qversion.lastIndexOf('.'));
    }
    if (qversion.contains('-')) {
        qversion.truncate(qversion.lastIndexOf('-'));
    }
    const QString &libPathKey = QStringLiteral("/qt/%1/libraryPath").arg(qversion);

    QStringList kdeAdded;
    KComponentData kcd("krdb libraryPath fix");
    const QStringList &plugins = KGlobal::dirs()->resourceDirs("qtplugins");
    foreach (const QString &_path, plugins) {
        QString path = QDir(_path).canonicalPath();
        if (path.isEmpty() || kdeAdded.contains(path)) {
            continue;
        }
        kdeAdded.prepend(path);
        if (path.contains(QLatin1String("/lib64/"))) {
            path.replace(QLatin1String("/lib64/"), QLatin1String("/lib/"));
            if (!kdeAdded.contains(path)) {
                kdeAdded.prepend(path);
            }
        }
    }

    // Don't use toStringList! That's a different storage format
    QStringList libraryPath = settings.value(libPathKey, QString())
        .toString().split(QLatin1Char(':'), QString::SkipEmptyParts);

    // Remove all KDE paths, not needed anymore, done by $QT_PLUGIN_PATH
    foreach (const QString &path, const_cast<const QStringList &>(kdeAdded)) {
        libraryPath.removeAll(path);
    }

    settings.remove(QStringLiteral("/qt/KDE/kdeAddedLibraryPaths"));
    settings.setValue(libPathKey, libraryPath.join(QLatin1Char(':')));

    return 0;
}
