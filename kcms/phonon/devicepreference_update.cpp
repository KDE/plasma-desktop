/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <QtCore/QCoreApplication>
#include <QtCore/QMutableListIterator>
#include <QtCore/QList>
#include "qsettingsgroup_p.h"
#include <kcomponentdata.h>
#include <kconfig.h>
#include <kconfiggroup.h>

using Phonon::QSettingsGroup;

Q_DECLARE_METATYPE(QList<int>)

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    KComponentData cData("phonon device preference update");

    KConfig kconfig("phonondevicesrc", KConfig::NoGlobals);
    KConfigGroup globalGroup(&kconfig, "Globals");
    int newIndexForZero = 0;

    foreach (const QString &group, kconfig.groupList()) {
        KConfigGroup configGroup(&kconfig, group);
        int index = configGroup.readEntry("index", -1);
        if (index == 0) {
            newIndexForZero = globalGroup.readEntry("nextIndex", 0);
            configGroup.writeEntry("index", newIndexForZero);
            globalGroup.writeEntry("nextIndex", newIndexForZero + 1);
            break;
        }
    }

    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");

    QSettings qconfig(QLatin1String("kde.org"), QLatin1String("libphonon"));
    QSettingsGroup outputGroup(&qconfig, QLatin1String("AudioOutputDevice"));
    for (int i = -1; i < 10; ++i) {
        const QString oldKey = QLatin1String("Category") + QString::number(i);
        const QString newKey = QLatin1String("Category_") + QString::number(i);
        if (outputGroup.hasKey(oldKey) && !outputGroup.hasKey(newKey)) {
            QList<int> deviceIndexes = outputGroup.value(oldKey, QList<int>());
            QMutableListIterator<int> index(deviceIndexes);
            while (index.hasNext()) {
                index.next();
                if (index.value() < 10000 && index.value() >= 0) {
                    if (index.value() == 0) {
                        Q_ASSERT(newIndexForZero);
                        index.setValue(-newIndexForZero);
                    } else {
                        index.setValue(-index.value());
                    }
                }
            }
            outputGroup.setValue(newKey, deviceIndexes);
            outputGroup.removeEntry(oldKey);
        }
    }

    return 0;
}
