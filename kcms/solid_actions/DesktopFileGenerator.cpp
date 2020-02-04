/***************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <ben@eclipse.endoftheinternet.org> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include <QCoreApplication>
#include <QCommandLineParser>

#include <KAboutData>
#include <KLocalizedString>

#include <KDesktopFile>
#include <KConfigGroup>

#include "SolidActionData.h"

int main( int argc, char *argv[] )
{
    QCoreApplication application(argc, argv);
    KLocalizedString::setApplicationDomain("kcm_solid_actions");

    // About data
    KAboutData aboutData(QStringLiteral("solid-action-desktop-gen"), i18n("Solid Action Desktop File Generator"), QStringLiteral("0.4"),
                         i18n("Tool to automatically generate Desktop Files from Solid DeviceInterface classes for translation"),
                         KAboutLicense::GPL, i18n("(c) 2009, Ben Cooksley"));
    aboutData.addAuthor(i18n("Ben Cooksley"), i18n("Maintainer"), QStringLiteral("ben@eclipse.endoftheinternet.org"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(application);
    aboutData.processCommandLine(&parser);

    SolidActionData *availActions = SolidActionData::instance();
    const auto interfaceTypes = availActions->interfaceTypeList();
    for (Solid::DeviceInterface::Type internalType : interfaceTypes) {
        const QString typeName = Solid::DeviceInterface::typeToString( internalType );
        KDesktopFile typeFile(QStandardPaths::GenericDataLocation, "solid/devices/solid-device-" + typeName + ".desktop" );
        KConfigGroup tConfig = typeFile.desktopGroup();

        tConfig.writeEntry( "Name", "Solid Device" );
        tConfig.writeEntry( "X-KDE-ServiceTypes", "SolidDevice" );
        tConfig.writeEntry( "Type", "Service" );

        if( !tConfig.hasKey("X-KDE-Solid-Actions-Type") ) {
            tConfig.writeEntry( "X-KDE-Solid-Actions-Type", typeName );
        }

        const QStringList typeValues = availActions->propertyInternalList( internalType );
        const QString actionText = typeValues.join(QLatin1Char(';')).append(";");
        tConfig.writeEntry( "Actions", actionText );

        for (const QString &tValue : typeValues) {
            KConfigGroup vConfig = typeFile.actionGroup( tValue );
            if( !vConfig.hasKey("Name") ) {
                vConfig.writeEntry( "Name", availActions->propertyName( internalType, tValue ) );
            }
            vConfig.sync();
        }
        tConfig.sync();
        typeFile.sync();
    }

    return 0;
}
