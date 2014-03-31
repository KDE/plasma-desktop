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

#include "SolidActionData.h"

#include <QList>
#include <QMetaProperty>

#include <KGlobal>
#include <kdesktopfileactions.h>
#include <KStandardDirs>
#include <KStringHandler>
#include <KDesktopFile>
#include <KConfigGroup>

#include <Solid/AcAdapter>
#include <Solid/AudioInterface>
#include <Solid/Battery>
#include <Solid/Block>
#include <Solid/Button>
#include <Solid/Camera>
#include <Solid/DvbInterface>
#include <Solid/GenericInterface>
#include <Solid/NetworkInterface>
#include <Solid/PortableMediaPlayer>
#include <Solid/Processor>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>
#include <Solid/OpticalDrive>
#include <Solid/StorageVolume>
#include <Solid/OpticalDisc>
#include <solid/video.h>
#include <solid/serialinterface.h>
#include <solid/smartcardreader.h>

static SolidActionData * actData = 0;

SolidActionData::SolidActionData(bool includeFiles)
{
    QStringList allPossibleDevices;
    int propertyOffset = Solid::DeviceInterface::staticMetaObject.propertyOffset();

    QList<QMetaObject> interfaceList = fillInterfaceList();
    foreach( const QMetaObject &interface, interfaceList ) {
        QString ifaceName = interface.className();
        ifaceName.remove(0, ifaceName.lastIndexOf(':') + 1);
        Solid::DeviceInterface::Type ifaceDev = Solid::DeviceInterface::stringToType( ifaceName );
        QString cleanName = Solid::DeviceInterface::typeDescription( ifaceDev );
        types.insert( ifaceDev, cleanName );

        QMap<QString, QString> deviceValues;
        for( int doneProps = propertyOffset; interface.propertyCount() > doneProps; doneProps = doneProps + 1 ) {
            QMetaProperty ifaceProp = interface.property(doneProps);
            deviceValues.insert( ifaceProp.name(), generateUserString(ifaceProp.name()) );
        }
        values.insert( ifaceDev, deviceValues );
    }

    if( includeFiles ) {
        // Fill the lists of possible device types / device values
        allPossibleDevices = KGlobal::dirs()->findAllResources("data", "solid/devices/");
        // List all the known device actions, then add their name and all values to the appropriate lists
        foreach( const QString &desktop, allPossibleDevices ) {
            KDesktopFile deviceFile(desktop);
            KConfigGroup deviceType = deviceFile.desktopGroup(); // Retrieve the configuration group where the user friendly name is

            QString ifaceName = deviceType.readEntry("X-KDE-Solid-Actions-Type");
            Solid::DeviceInterface::Type ifaceDev = Solid::DeviceInterface::stringToType( ifaceName );
            QString cleanName = Solid::DeviceInterface::typeDescription( ifaceDev );

            types.insert( ifaceDev, cleanName ); // Read the user friendly name

            QMap<QString,QString> deviceValues = values.value( ifaceDev );
            foreach( const QString &text, deviceFile.readActions() ) { // We want every single action
                KConfigGroup actionType = deviceFile.actionGroup( text );
                deviceValues.insert( text, actionType.readEntry("Name") ); // Add to the type - actions map
            }
            values.insert( ifaceDev, deviceValues );
        }
    }
}

QList<QString> SolidActionData::propertyList( Solid::DeviceInterface::Type devInterface )
{
    return values.value( devInterface ).values();
}

QList<QString> SolidActionData::propertyInternalList( Solid::DeviceInterface::Type devInterface )
{
    return values.value( devInterface ).keys();
}

QString SolidActionData::propertyInternal( Solid::DeviceInterface::Type devInterface, QString property )
{
    return values.value( devInterface ).key( property );
}

QString SolidActionData::propertyName( Solid::DeviceInterface::Type devInterface, QString property )
{
    return values.value( devInterface ).value( property );
}

int SolidActionData::propertyPosition( Solid::DeviceInterface::Type devInterface, QString property )
{
    return values.value( devInterface ).keys().indexOf( property );
}

QList<QString> SolidActionData::interfaceList()
{
    return types.values();
}

QList<Solid::DeviceInterface::Type> SolidActionData::interfaceTypeList()
{
    return types.keys();
}

Solid::DeviceInterface::Type SolidActionData::interfaceFromName( const QString& name )
{
    return types.key( name );
}

QString SolidActionData::nameFromInterface( Solid::DeviceInterface::Type devInterface )
{
    return types.value( devInterface );
}

int SolidActionData::interfacePosition( Solid::DeviceInterface::Type devInterface )
{
    return types.keys().indexOf( devInterface );
}

QString SolidActionData::generateUserString( QString className )
{
    QString finalString;
    QRegExp camelCase("([A-Z])"); // Create the split regexp

    finalString = className.remove(0, className.lastIndexOf(':') + 1); // Remove any Class information
    finalString = finalString.replace( camelCase, " \\1" ); // Use Camel Casing to add spaces
    finalString = KStringHandler::capwords( finalString ); // Captialise everything
    return finalString.trimmed();
}

SolidActionData * SolidActionData::instance()
{
    if( actData == 0 ) {
        actData = new SolidActionData( true );
    }
    return actData;
}

QList<QMetaObject> SolidActionData::fillInterfaceList()
{
    QList<QMetaObject> interfaces;
    interfaces.append( Solid::AcAdapter::staticMetaObject );
    interfaces.append( Solid::AudioInterface::staticMetaObject );
    interfaces.append( Solid::Battery::staticMetaObject );
    interfaces.append( Solid::Block::staticMetaObject );
    interfaces.append( Solid::Button::staticMetaObject );
    interfaces.append( Solid::Camera::staticMetaObject );
    interfaces.append( Solid::DvbInterface::staticMetaObject );
    interfaces.append( Solid::NetworkInterface::staticMetaObject );
    interfaces.append( Solid::PortableMediaPlayer::staticMetaObject );
    interfaces.append( Solid::Processor::staticMetaObject );
    interfaces.append( Solid::SerialInterface::staticMetaObject );
    interfaces.append( Solid::StorageAccess::staticMetaObject );
    interfaces.append( Solid::StorageDrive::staticMetaObject );
    interfaces.append( Solid::OpticalDrive::staticMetaObject );
    interfaces.append( Solid::StorageVolume::staticMetaObject );
    interfaces.append( Solid::OpticalDisc::staticMetaObject );
    interfaces.append( Solid::Video::staticMetaObject );
    interfaces.append( Solid::SmartCardReader::staticMetaObject );
    return interfaces;
}

#include "SolidActionData.moc"
