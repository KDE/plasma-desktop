/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ActionItem.h"
#include "SolidActionData.h"

#include <QString>

#include <KConfigGroup>
#include <KDesktopFile>

#include <Solid/DeviceInterface>

ActionItem::ActionItem(const QString &pathToDesktop, const QString &action, QObject *parent)
    : desktopMasterPath(pathToDesktop)
    , actionName(action)
{
    Q_UNUSED(parent);

    // Create the desktop file
    desktopFileMaster = new KDesktopFile(desktopMasterPath);
    desktopWritePath = desktopFileMaster->locateLocal(desktopMasterPath);
    desktopFileWrite = new KDesktopFile(desktopWritePath);
    // Now we can fill the action groups list
    configGroups.append(desktopFileMaster->desktopGroup());
    actionGroupIndices.insert(ActionItem::GroupDesktop, configGroups.size() - 1);
    configGroups.append(desktopFileMaster->actionGroup(actionName));
    actionGroupIndices.insert(ActionItem::GroupAction, configGroups.size() - 1);
    configGroups.append(desktopFileWrite->desktopGroup());
    actionGroupIndices.insert(ActionItem::GroupDesktop, configGroups.size() - 1);
    configGroups.append(desktopFileWrite->actionGroup(actionName));
    actionGroupIndices.insert(ActionItem::GroupAction, configGroups.size() - 1);

    const QString predicateString = readKey(ActionItem::GroupDesktop, QStringLiteral("X-KDE-Solid-Predicate"), QString());
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
    return readKey(ActionItem::GroupAction, QStringLiteral("Icon"), QString());
}

QString ActionItem::exec() const
{
    return readKey(ActionItem::GroupAction, QStringLiteral("Exec"), QString());
}

QString ActionItem::name() const
{
    return readKey(ActionItem::GroupAction, QStringLiteral("Name"), QString());
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
    return configItem(ActionItem::DesktopRead, keyGroup, keyName).readEntry(keyName, defaultValue);
}

void ActionItem::setKey(GroupType keyGroup, const QString &keyName, const QString &keyContents)
{
    configItem(ActionItem::DesktopWrite, keyGroup).writeEntry(keyName, keyContents);
}

bool ActionItem::hasKey(GroupType keyGroup, const QString &keyName) const
{
    return configItem(ActionItem::DesktopRead, keyGroup, keyName).hasKey(keyName);
}

KConfigGroup &ActionItem::configItem(DesktopAction actionType, GroupType keyGroup, const QString &keyName)
{
    return configGroups[configItemIndex(actionType, keyGroup, keyName)];
}

const KConfigGroup &ActionItem::configItem(DesktopAction actionType, GroupType keyGroup, const QString &keyName) const
{
    return configGroups.at(configItemIndex(actionType, keyGroup, keyName));
}

qsizetype ActionItem::configItemIndex(DesktopAction actionType, GroupType keyGroup, const QString &keyName) const
{
    switch (actionType) {
    case ActionItem::DesktopRead: {
        const auto indices = actionGroupIndices.values(keyGroup);
        for (qsizetype possibleGroupIndex : indices) {
            if (configGroups[possibleGroupIndex].hasKey(keyName)) {
                return possibleGroupIndex;
            }
        }
        return indices.first(); // backstop so a valid value is always returned
    }
    case ActionItem::DesktopWrite: {
        int countAccess = isUserSupplied() ? 1 : 0;
        return actionGroupIndices.values(keyGroup).at(countAccess);
    }
    }
    Q_UNREACHABLE();
}

#include "moc_ActionItem.cpp"
