/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SolidActionData.h"

#include <QDirIterator>
#include <QList>
#include <QMetaProperty>

#include <KConfigGroup>
#include <KDesktopFile>
#include <KDesktopFileActions>
#include <KStringHandler>

#include <Solid/Battery>
#include <Solid/Block>
#include <Solid/Camera>
#include <Solid/GenericInterface>
#include <Solid/OpticalDisc>
#include <Solid/OpticalDrive>
#include <Solid/PortableMediaPlayer>
#include <Solid/Processor>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>
#include <Solid/StorageVolume>

static SolidActionData *actData = nullptr;

SolidActionData::SolidActionData(bool includeFiles)
{
    const int propertyOffset = Solid::DeviceInterface::staticMetaObject.propertyOffset();

    const QList<QMetaObject> interfaceList = fillInterfaceList();
    for (const QMetaObject &interface : interfaceList) {
        QString ifaceName = interface.className();
        ifaceName.remove(0, ifaceName.lastIndexOf(':') + 1);
        Solid::DeviceInterface::Type ifaceDev = Solid::DeviceInterface::stringToType(ifaceName);
        const QString cleanName = Solid::DeviceInterface::typeDescription(ifaceDev);
        types.insert(ifaceDev, cleanName);

        QMap<QString, QString> deviceValues;
        for (int doneProps = propertyOffset; interface.propertyCount() > doneProps; doneProps = doneProps + 1) {
            QMetaProperty ifaceProp = interface.property(doneProps);
            deviceValues.insert(ifaceProp.name(), generateUserString(ifaceProp.name()));
        }
        values.insert(ifaceDev, deviceValues);
    }

    if (includeFiles) {
        // Fill the lists of possible device types / device values
        const QString deviceDir = QStandardPaths::locate(QStandardPaths::GenericDataLocation, //
                                                         QStringLiteral("/solid/devices/"),
                                                         QStandardPaths::LocateDirectory);
        // List all the known device actions, then add their name and all values to the appropriate lists
        QDirIterator it(deviceDir, QStringList() << QStringLiteral("*.desktop"));
        while (it.hasNext()) {
            it.next();
            const QString desktop = it.filePath();
            KDesktopFile deviceFile(desktop);
            KConfigGroup deviceType = deviceFile.desktopGroup(); // Retrieve the configuration group where the user friendly name is

            const QString ifaceName = deviceType.readEntry("X-KDE-Solid-Actions-Type");
            Solid::DeviceInterface::Type ifaceDev = Solid::DeviceInterface::stringToType(ifaceName);
            const QString cleanName = Solid::DeviceInterface::typeDescription(ifaceDev);

            types.insert(ifaceDev, cleanName); // Read the user friendly name

            QMap<QString, QString> deviceValues = values.value(ifaceDev);
            const auto actions = deviceFile.readActions();
            for (const QString &text : actions) { // We want every single action
                KConfigGroup actionType = deviceFile.actionGroup(text);
                deviceValues.insert(text, actionType.readEntry("Name")); // Add to the type - actions map
            }
            values.insert(ifaceDev, deviceValues);
        }
    }
}

QList<QString> SolidActionData::propertyList(Solid::DeviceInterface::Type devInterface)
{
    return values.value(devInterface).values();
}

QList<QString> SolidActionData::propertyInternalList(Solid::DeviceInterface::Type devInterface)
{
    return values.value(devInterface).keys();
}

QString SolidActionData::propertyInternal(Solid::DeviceInterface::Type devInterface, QString property)
{
    return values.value(devInterface).key(property);
}

QString SolidActionData::propertyName(Solid::DeviceInterface::Type devInterface, QString property)
{
    return values.value(devInterface).value(property);
}

int SolidActionData::propertyPosition(Solid::DeviceInterface::Type devInterface, QString property)
{
    return values.value(devInterface).keys().indexOf(property);
}

QList<QString> SolidActionData::interfaceList()
{
    return types.values();
}

QList<Solid::DeviceInterface::Type> SolidActionData::interfaceTypeList()
{
    return types.keys();
}

Solid::DeviceInterface::Type SolidActionData::interfaceFromName(const QString &name)
{
    return types.key(name);
}

QString SolidActionData::nameFromInterface(Solid::DeviceInterface::Type devInterface)
{
    return types.value(devInterface);
}

int SolidActionData::interfacePosition(Solid::DeviceInterface::Type devInterface)
{
    return types.keys().indexOf(devInterface);
}

QString SolidActionData::generateUserString(QString className)
{
    QString finalString;
    QRegExp camelCase(QStringLiteral("([A-Z])")); // Create the split regexp

    finalString = className.remove(0, className.lastIndexOf(':') + 1); // Remove any Class information
    finalString.replace(camelCase, QStringLiteral(" \\1")); // Use Camel Casing to add spaces
    finalString = KStringHandler::capwords(finalString); // Capitalize everything
    return finalString.trimmed();
}

SolidActionData *SolidActionData::instance()
{
    if (actData == nullptr) {
        actData = new SolidActionData(true);
    }
    return actData;
}

QList<QMetaObject> SolidActionData::fillInterfaceList()
{
    QList<QMetaObject> interfaces;
    interfaces.append(Solid::Battery::staticMetaObject);
    interfaces.append(Solid::Block::staticMetaObject);
    interfaces.append(Solid::Camera::staticMetaObject);
    interfaces.append(Solid::PortableMediaPlayer::staticMetaObject);
    interfaces.append(Solid::Processor::staticMetaObject);
    interfaces.append(Solid::StorageAccess::staticMetaObject);
    interfaces.append(Solid::StorageDrive::staticMetaObject);
    interfaces.append(Solid::OpticalDrive::staticMetaObject);
    interfaces.append(Solid::StorageVolume::staticMetaObject);
    interfaces.append(Solid::OpticalDisc::staticMetaObject);
    return interfaces;
}
