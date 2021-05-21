/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QCommandLineParser>
#include <QCoreApplication>

#include <KAboutData>
#include <KLocalizedString>

#include <KConfigGroup>
#include <KDesktopFile>

#include "SolidActionData.h"

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    KLocalizedString::setApplicationDomain("kcm_solid_actions");

    // About data
    KAboutData aboutData(QStringLiteral("solid-action-desktop-gen"),
                         i18n("Solid Action Desktop File Generator"),
                         QStringLiteral("0.4"),
                         i18n("Tool to automatically generate Desktop Files from Solid DeviceInterface classes for translation"),
                         KAboutLicense::GPL,
                         i18n("(c) 2009, Ben Cooksley"));
    aboutData.addAuthor(i18n("Ben Cooksley"), i18n("Maintainer"), QStringLiteral("ben@eclipse.endoftheinternet.org"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(application);
    aboutData.processCommandLine(&parser);

    SolidActionData *availActions = SolidActionData::instance();
    const auto interfaceTypes = availActions->interfaceTypeList();
    for (Solid::DeviceInterface::Type internalType : interfaceTypes) {
        const QString typeName = Solid::DeviceInterface::typeToString(internalType);
        KDesktopFile typeFile(QStandardPaths::GenericDataLocation, "solid/devices/solid-device-" + typeName + ".desktop");
        KConfigGroup tConfig = typeFile.desktopGroup();

        tConfig.writeEntry("Name", "Solid Device");
        tConfig.writeEntry("X-KDE-ServiceTypes", "SolidDevice");
        tConfig.writeEntry("Type", "Service");

        if (!tConfig.hasKey("X-KDE-Solid-Actions-Type")) {
            tConfig.writeEntry("X-KDE-Solid-Actions-Type", typeName);
        }

        const QStringList typeValues = availActions->propertyInternalList(internalType);
        const QString actionText = typeValues.join(QLatin1Char(';')).append(";");
        tConfig.writeEntry("Actions", actionText);

        for (const QString &tValue : typeValues) {
            KConfigGroup vConfig = typeFile.actionGroup(tValue);
            if (!vConfig.hasKey("Name")) {
                vConfig.writeEntry("Name", availActions->propertyName(internalType, tValue));
            }
            vConfig.sync();
        }
        tConfig.sync();
        typeFile.sync();
    }

    return 0;
}
