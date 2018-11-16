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

#ifndef ACTION_ITEM_H
#define ACTION_ITEM_H

#include <QObject>
#include <QMap>

#include <Solid/Predicate>

class QString;

class KDesktopFile;
class KConfigGroup;

class ActionItem: public QObject
{
    Q_OBJECT

public:
    ActionItem(const QString& pathToDesktop, const QString& action, QObject *parent = nullptr);
    ~ActionItem() override;

    bool isUserSupplied() const;

    QString icon() const;
    QString exec() const;
    QString name() const;
    Solid::Predicate predicate() const;
    QString involvedTypes() const;
    void setIcon( const QString& nameOfIcon );
    void setName( const QString& nameOfAction );
    void setExec( const QString& execUrl );
    void setPredicate( const QString& newPredicate );

    QString desktopMasterPath;
    QString desktopWritePath;
    QString actionName;

private:
    enum DesktopAction { DesktopRead = 0, DesktopWrite = 1 };
    enum GroupType { GroupDesktop = 0, GroupAction = 1 };

    QString readKey(GroupType keyGroup, const QString& keyName, const QString& defaultValue) const;
    void setKey(GroupType keyGroup, const QString& keyName, const QString& keyContents);
    bool hasKey(GroupType keyGroup, const QString& keyName) const;
    KConfigGroup * configItem(DesktopAction actionType, GroupType keyGroup, const QString& keyName = QString()) const;

    KDesktopFile * desktopFileMaster;
    KDesktopFile * desktopFileWrite;
    QMultiMap<GroupType, KConfigGroup*> actionGroups;
    QList<KConfigGroup> configGroups;
    Solid::Predicate predicateItem;
};

Q_DECLARE_METATYPE( ActionItem * )

#endif
