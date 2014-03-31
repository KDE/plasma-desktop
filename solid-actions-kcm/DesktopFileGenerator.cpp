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

#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KApplication>
#include <KDesktopFile>
#include <KDebug>
#include <KConfigGroup>

#include "SolidActionData.h"

#include <iostream>

int main( int argc, char *argv[] )
{
    KLocale::setMainCatalog("solid-action-desktop-gen");
    // About data
    KAboutData aboutData("solid-action-desktop-gen", 0, ki18n("Solid Action Desktop File Generator"), "0.4", ki18n("Tool to automatically generate Desktop Files from Solid DeviceInterface classes for translation"),
                         KAboutData::License_GPL, ki18n("(c) 2009, Ben Cooksley"));
    aboutData.addAuthor(ki18n("Ben Cooksley"), ki18n("Maintainer"), "ben@eclipse.endoftheinternet.org");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication application(false);
    SolidActionData * availActions = SolidActionData::instance();
    foreach( Solid::DeviceInterface::Type internalType, availActions->interfaceTypeList() ) {
        QString typeName = Solid::DeviceInterface::typeToString( internalType );
        KDesktopFile typeFile( "services", "solid-device-" + typeName + ".desktop" );
        KConfigGroup tConfig = typeFile.desktopGroup();

        tConfig.writeEntry( "Name", "Solid Device" );
        tConfig.writeEntry( "X-KDE-ServiceTypes", "SolidDevice" );
        tConfig.writeEntry( "Type", "Service" );

        if( !tConfig.hasKey("X-KDE-Solid-Actions-Type") ) {
            tConfig.writeEntry( "X-KDE-Solid-Actions-Type", typeName );
        }

        QStringList typeValues = availActions->propertyInternalList( internalType );
        QString actionText = typeValues.join(";").append(";");
        tConfig.writeEntry( "Actions", actionText );

        kWarning() << "Desktop file created: " + typeFile.fileName();
        foreach( const QString &tValue, typeValues ) {
            KConfigGroup vConfig = typeFile.actionGroup( tValue );
            if( !vConfig.hasKey("Name") ) {
                vConfig.writeEntry( "Name", availActions->propertyName( internalType, tValue ) );
            }
            vConfig.sync();
        }
        tConfig.sync();
        typeFile.sync();
    }

    kWarning() << "Generation now completed";
    return 0;
}
