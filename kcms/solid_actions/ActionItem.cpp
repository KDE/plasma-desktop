/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ActionItem.h"
#include "SolidActionData.h"

#include <QString>

#include <KConfigGroup>
#include <KDesktopFile>
#include <KDesktopFileActions>

#include <Solid/DeviceInterface>

ActionItem::ActionItem(const QString &pathToDesktop, const QString &action, QObject *parent)
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
    actionGroups.insert(ActionItem::GroupDesktop, &configGroups.last());
    configGroups.append(desktopFileMaster->actionGroup(actionName));
    actionGroups.insert(ActionItem::GroupAction, &configGroups.last());
    configGroups.append(desktopFileWrite->desktopGroup());
    actionGroups.insert(ActionItem::GroupDesktop, &configGroups.last());
    configGroups.append(desktopFileWrite->actionGroup(actionName));
    actionGroups.insert(ActionItem::GroupAction, &configGroups.last());

    const QString predicateString = readKey(ActionItem::GroupDesktop, QStringLiteral("X-KDE-Solid-Predicate"), QLatin1String(""));
    predicateItem = Solid::Predicate::fromString(predicateString);
}

ActionItem::~ActionItem()
{
    delete desktopFileWrite;
    delete desktopFileMaster;
}

/// Public functions below

bool ActionItem::isUserSupplied() const
{
    return hasKey(ActionItem::GroupDesktop, QStringLiteral("X-KDE-Action-Custom"));
}

QString ActionItem::icon() const
{
    return readKey(ActionItem::GroupAction, QStringLiteral("Icon"), QLatin1String(""));
}

QString ActionItem::exec() const
{
    return readKey(ActionItem::GroupAction, QStringLiteral("Exec"), QLatin1String(""));
}

QString ActionItem::name() const
{
    return readKey(ActionItem::GroupAction, QStringLiteral("Name"), QLatin1String(""));
}

Solid::Predicate ActionItem::predicate() const
{
    return predicateItem;
}

QString ActionItem::involvedTypes() const
{
    SolidActionData *actData = SolidActionData::instance();
    const QSet<Solid::DeviceInterface::Type> devTypeList = predicateItem.usedTypes();
    QStringList deviceTypes;
    for (Solid::DeviceInterface::Type devType : devTypeList) {
        deviceTypes << actData->nameFromInterface(devType);
    }

    return deviceTypes.join(QLatin1String(", "));
}

void ActionItem::setIcon(const QString &nameOfIcon)
{
    setKey(ActionItem::GroupAction, QStringLiteral("Icon"), nameOfIcon);
}

void ActionItem::setName(const QString &nameOfAction)
{
    setKey(ActionItem::GroupAction, QStringLiteral("Name"), nameOfAction);
}

void ActionItem::setExec(const QString &execUrl)
{
    setKey(ActionItem::GroupAction, QStringLiteral("Exec"), execUrl);
}

void ActionItem::setPredicate(const QString &newPredicate)
{
    setKey(ActionItem::GroupDesktop, QStringLiteral("X-KDE-Solid-Predicate"), newPredicate);
    predicateItem = Solid::Predicate::fromString(newPredicate);
}

/// Private functions below

QString ActionItem::readKey(GroupType keyGroup, const QString &keyName, const QString &defaultValue) const
{
    return configItem(ActionItem::DesktopRead, keyGroup, keyName)->readEntry(keyName, defaultValue);
}

void ActionItem::setKey(GroupType keyGroup, const QString &keyName, const QString &keyContents)
{
    configItem(ActionItem::DesktopWrite, keyGroup)->writeEntry(keyName, keyContents);
}

bool ActionItem::hasKey(GroupType keyGroup, const QString &keyName) const
{
    return configItem(ActionItem::DesktopRead, keyGroup, keyName)->hasKey(keyName);
}

KConfigGroup *ActionItem::configItem(DesktopAction actionType, GroupType keyGroup, const QString &keyName) const
{
    int countAccess = 0;

    if (actionType == ActionItem::DesktopRead) {
        const auto values = actionGroups.values(keyGroup);
        for (KConfigGroup *possibleGroup : values) {
            if (possibleGroup->hasKey(keyName)) {
                return possibleGroup;
            }
        }
    } else if (actionType == ActionItem::DesktopWrite) {
        if (isUserSupplied()) {
            countAccess = 1;
        }
        return actionGroups.values(keyGroup).at(countAccess);
    }
    return actionGroups.values(keyGroup).at(0); // Implement a backstop so a valid value is always returned
}
