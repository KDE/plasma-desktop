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
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    KComponentData cData("phonon device uids update");

    // Things to update:
    // 1. change group names from
    //    Audio{Capture,Output}Device_<bus>:<numbers>:{capture,playback}:{alsa,oss}:<devicenum>
    //    to
    //    AudioDevice_<bus>:<numbers>:<devicenum>:{capture,playback}
    // 2. make indexes negative
    // 3. remove playbackDevice/captureDevice keys
    // 4. remove udi key
    // 5. remove driver key
    // 6. add deviceNumber key
    KConfig kconfig("phonondevicesrc", KConfig::SimpleConfig);
    KConfigGroup globalGroup(&kconfig, "Globals");
    int nextIndex = globalGroup.readEntry("nextIndex", 1);

    const QStringList &oldGroupList = kconfig.groupList();
    foreach (const QString &group, oldGroupList) {
        if (group == "Globals") {
            continue;
        }
        if (group.startsWith(QLatin1String("front:")) || group.startsWith(QLatin1String("iec958:"))) {
            kconfig.deleteGroup(group);
            continue;
        }

        KConfigGroup configGroup(&kconfig, group);

        const QString &cardName = configGroup.readEntry("cardName", QString());
        const QString &icon = configGroup.readEntry("icon", QString());
        const int initialPreference = configGroup.readEntry("initialPreference", 0);
        const bool isAdvanced = configGroup.readEntry("isAdvanced", true);

        int index = configGroup.readEntry("index", 0);
        if (index <= 0) {
            kError() << "invalid old index found";
        } else {
            index *= -1;
        }
        QString newGroup;
        int deviceNumber = -1;

        if (group.startsWith(QLatin1String("AudioOutputDevice_"))) {
            const QString &deviceNumberString = group.right(group.length() - group.lastIndexOf(QLatin1Char(':')) - 1);
            bool ok = false;
            deviceNumber = deviceNumberString.toInt(&ok);
            if (!ok) {
                // It's probably a fallback UniqueID. Let's get rid of it.
                kconfig.deleteGroup(group);
                continue;
            }
            newGroup = QLatin1String("AudioDevice_") +
                group.mid(18, group.indexOf(QLatin1String(":playback")) - 17) +
                deviceNumberString + QLatin1String(":playback");
        } else if (group.startsWith(QLatin1String("AudioCaptureDevice_"))) {
            const QString &deviceNumberString = group.right(group.length() - group.lastIndexOf(QLatin1Char(':')) - 1);
            bool ok = false;
            deviceNumber = deviceNumberString.toInt(&ok);
            if (!ok) {
                // It's probably a fallback UniqueID. Let's get rid of it.
                kconfig.deleteGroup(group);
                continue;
            }
            newGroup = QLatin1String("AudioDevice_") +
                group.mid(19, group.indexOf(QLatin1String(":capture")) - 18) +
                deviceNumberString + QLatin1String(":capture");
        } else if (group.startsWith(QLatin1String("AudioIODevice_"))) {
            /*
             * OSS Device entry, on Linux already covered by ALSA, so we skip it if an ALSA device
             * for this existed.
             */
            if (oldGroupList.indexOf(QRegExp(QLatin1String("^Audio(Capture|Output)Device_") +
                            group.mid(14, group.indexOf(QLatin1String(":both")) - 14) +
                            QLatin1String(":(capture|playback):alsa:\\d+$"))) > -1) {
                kconfig.deleteGroup(group);
                continue;
            }
            const QString &deviceNumberString = group.right(group.length() - group.lastIndexOf(QLatin1Char(':')) - 1);
            bool ok = false;
            deviceNumber = deviceNumberString.toInt(&ok);
            if (!ok) {
                // It's probably a fallback UniqueID. Let's get rid of it.
                kconfig.deleteGroup(group);
                continue;
            }
            newGroup = QLatin1String("AudioDevice_") +
                group.mid(19, group.indexOf(QLatin1String(":both")) - 18) + deviceNumberString;

            {
                KConfigGroup newConfigGroup(&kconfig, newGroup + QLatin1String(":playback"));
                newConfigGroup.writeEntry("cardName", cardName);
                newConfigGroup.writeEntry("iconName", icon);
                newConfigGroup.writeEntry("initialPreference", initialPreference);
                newConfigGroup.writeEntry("isAdvanced", isAdvanced);
                newConfigGroup.writeEntry("index", index);
                newConfigGroup.writeEntry("deviceNumber", deviceNumber);
            } {
                KConfigGroup newConfigGroup(&kconfig, newGroup + QLatin1String(":capture"));
                newConfigGroup.writeEntry("cardName", cardName);
                newConfigGroup.writeEntry("iconName", icon);
                newConfigGroup.writeEntry("initialPreference", initialPreference);
                newConfigGroup.writeEntry("isAdvanced", isAdvanced);
                newConfigGroup.writeEntry("index", -(nextIndex++));
                newConfigGroup.writeEntry("deviceNumber", deviceNumber);
            }
            kconfig.deleteGroup(group);
            continue;
        } else {
            kWarning() << "found unknown/unhandled group:" << group << ". Removing.";
            kconfig.deleteGroup(group);
            continue;
        }
        KConfigGroup newConfigGroup(&kconfig, newGroup);
        newConfigGroup.writeEntry("cardName", cardName);
        newConfigGroup.writeEntry("iconName", icon);
        newConfigGroup.writeEntry("initialPreference", initialPreference);
        newConfigGroup.writeEntry("isAdvanced", isAdvanced);
        newConfigGroup.writeEntry("index", index);
        newConfigGroup.writeEntry("deviceNumber", deviceNumber);
        kconfig.deleteGroup(group);
    }

    globalGroup.writeEntry("nextIndex", nextIndex);

    return 0;
}
