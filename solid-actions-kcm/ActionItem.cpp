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

#include "ActionItem.h"
#include "SolidActionData.h"

#include <QString>

#include <kdesktopfileactions.h>
#include <KDesktopFile>
#include <KConfigGroup>

#include <Solid/DeviceInterface>

ActionItem::ActionItem(const QString& pathToDesktop, const QString& action, QObject *parent)
{
    Q_UNUSED(parent);

    desktopMasterPath = pathToDesktop;
    actionName = action;
    // Create the desktop file
    desktopFileMaster = new KDesktopFile(desktopMasterPath);
    desktopWritePath = desktopFileMaster->locateLocal(desktopMasterPath);
    desktopFileWrite = new KDesktopFile(desktopWritePath);
    // Now we can fill the action groups list
    configGroups.append(desktopFileMaster->desktopGroup());
    actionGroups.insertMulti(ActionItem::GroupDesktop, &configGroups.last());
    configGroups.append(desktopFileMaster->actionGroup(actionName));
    actionGroups.insertMulti(ActionItem::GroupAction, &configGroups.last());
    configGroups.append(desktopFileWrite->desktopGroup());
    actionGroups.insertMulti(ActionItem::GroupDesktop, &configGroups.last());
    configGroups.append(desktopFileWrite->actionGroup(actionName));
    actionGroups.insertMulti(ActionItem::GroupAction, &configGroups.last());

    QString predicateString = readKey(ActionItem::GroupDesktop, "X-KDE-Solid-Predicate", "");
    predicateItem = Solid::Predicate::fromString( predicateString );
}

ActionItem::~ActionItem()
{
    delete desktopFileWrite;
    delete desktopFileMaster;
}

/// Public functions below

bool ActionItem::isUserSupplied()
{
    return hasKey(ActionItem::GroupDesktop, "X-KDE-Action-Custom");
}

QString ActionItem::icon()
{
    return readKey(ActionItem::GroupAction, "Icon", "");
}

QString ActionItem::exec() 
{
    return readKey(ActionItem::GroupAction, "Exec", "");
}

QString ActionItem::name()
{
    return readKey(ActionItem::GroupAction, "Name", "");
}

Solid::Predicate ActionItem::predicate() const
{
    return predicateItem;
}

QString ActionItem::involvedTypes() const
{
    SolidActionData * actData = SolidActionData::instance();
    QSet<Solid::DeviceInterface::Type> devTypeList = predicateItem.usedTypes();
    QStringList deviceTypes;
    foreach( Solid::DeviceInterface::Type devType, devTypeList ) {
        deviceTypes << actData->nameFromInterface( devType );
    }

    return deviceTypes.join(", ");
}

void ActionItem::setIcon(const QString& nameOfIcon)
{
    setKey(ActionItem::GroupAction, "Icon", nameOfIcon);
}

void ActionItem::setName(const QString& nameOfAction)
{
    setKey(ActionItem::GroupAction, "Name", nameOfAction);
}

void ActionItem::setExec(const QString& execUrl)
{
    setKey(ActionItem::GroupAction, "Exec", execUrl);
}

void ActionItem::setPredicate( const QString& newPredicate )
{
    setKey(ActionItem::GroupDesktop, "X-KDE-Solid-Predicate", newPredicate);
    predicateItem = Solid::Predicate::fromString( newPredicate );
}

/// Private functions below

QString ActionItem::readKey(GroupType keyGroup, const QString& keyName, const QString& defaultValue)
{
    return configItem(ActionItem::DesktopRead, keyGroup, keyName)->readEntry(keyName, defaultValue);
}

void ActionItem::setKey(GroupType keyGroup, const QString& keyName, const QString& keyContents)
{
    configItem(ActionItem::DesktopWrite, keyGroup)->writeEntry(keyName, keyContents);
}

bool ActionItem::hasKey(GroupType keyGroup, const QString& keyName)
{
    return configItem(ActionItem::DesktopRead, keyGroup, keyName)->hasKey(keyName);
}

KConfigGroup * ActionItem::configItem(DesktopAction actionType, GroupType keyGroup, const QString& keyName)
{
    int countAccess = 0;

    if (actionType == ActionItem::DesktopRead) {
        foreach(KConfigGroup * possibleGroup, actionGroups.values(keyGroup)) {
            if (possibleGroup->hasKey(keyName)) {
                return possibleGroup;
                break;
            }
        }
    } else if (actionType == ActionItem::DesktopWrite) {
        if (isUserSupplied()) {
            countAccess = 1;
        }
        return actionGroups.values(keyGroup)[countAccess];
    }
    return actionGroups.values(keyGroup)[0]; // Implement a backstop so a valid value is always returned
}

#include "ActionItem.moc"
