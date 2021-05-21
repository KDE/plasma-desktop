/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTION_ITEM_H
#define ACTION_ITEM_H

#include <QMap>
#include <QObject>

#include <Solid/Predicate>

class QString;

class KDesktopFile;
class KConfigGroup;

class ActionItem : public QObject
{
    Q_OBJECT

public:
    ActionItem(const QString &pathToDesktop, const QString &action, QObject *parent = nullptr);
    ~ActionItem() override;

    bool isUserSupplied() const;

    QString icon() const;
    QString exec() const;
    QString name() const;
    Solid::Predicate predicate() const;
    QString involvedTypes() const;
    void setIcon(const QString &nameOfIcon);
    void setName(const QString &nameOfAction);
    void setExec(const QString &execUrl);
    void setPredicate(const QString &newPredicate);

    QString desktopMasterPath;
    QString desktopWritePath;
    QString actionName;

private:
    enum DesktopAction { DesktopRead = 0, DesktopWrite = 1 };
    enum GroupType { GroupDesktop = 0, GroupAction = 1 };

    QString readKey(GroupType keyGroup, const QString &keyName, const QString &defaultValue) const;
    void setKey(GroupType keyGroup, const QString &keyName, const QString &keyContents);
    bool hasKey(GroupType keyGroup, const QString &keyName) const;
    KConfigGroup *configItem(DesktopAction actionType, GroupType keyGroup, const QString &keyName = QString()) const;

    KDesktopFile *desktopFileMaster;
    KDesktopFile *desktopFileWrite;
    QMultiMap<GroupType, KConfigGroup *> actionGroups;
    QList<KConfigGroup> configGroups;
    Solid::Predicate predicateItem;
};

Q_DECLARE_METATYPE(ActionItem *)

#endif
