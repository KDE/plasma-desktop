/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SOLIDACTIONDATA_H
#define SOLIDACTIONDATA_H

#include <QMap>
#include <QObject>
#include <QString>

#include <Solid/DeviceInterface>

class SolidActionData : public QObject
{
    Q_OBJECT

public:
    QList<QString> propertyList(Solid::DeviceInterface::Type devInterface);
    QList<QString> propertyInternalList(Solid::DeviceInterface::Type devInterface);
    QString propertyInternal(Solid::DeviceInterface::Type devInterface, QString property);
    QString propertyName(Solid::DeviceInterface::Type devInterface, QString property);
    int propertyPosition(Solid::DeviceInterface::Type devInterface, QString property);

    QList<QString> interfaceList();
    QList<Solid::DeviceInterface::Type> interfaceTypeList();
    Solid::DeviceInterface::Type interfaceFromName(const QString &name);
    QString nameFromInterface(Solid::DeviceInterface::Type devInterface);
    int interfacePosition(Solid::DeviceInterface::Type devInterface);

    static SolidActionData *instance();

private:
    SolidActionData(bool includeFiles);
    QString generateUserString(QString className);
    QList<QMetaObject> fillInterfaceList();

    QMap<Solid::DeviceInterface::Type, QMap<QString, QString>> values;
    QMap<Solid::DeviceInterface::Type, QString> types;
};

#endif
