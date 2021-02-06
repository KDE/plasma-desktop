#include "layoutnames.h"
#include <QDBusMetaType>

void LayoutNames::registerMetaType()
{
    qDBusRegisterMetaType<LayoutNames>();
    qDBusRegisterMetaType<QVector<LayoutNames>>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const LayoutNames &layoutNames)
{
    argument.beginStructure();
    argument << layoutNames.shortName << layoutNames.displayName << layoutNames.longName;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, LayoutNames &layoutNames)
{
    argument.beginStructure();
    argument >> layoutNames.shortName >> layoutNames.displayName >> layoutNames.longName;
    argument.endStructure();
    return argument;
}
