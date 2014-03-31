/***************************************************************************
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
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

#ifndef AUTOSTARTITEM_H
#define AUTOSTARTITEM_H

#include <QTreeWidgetItem>
#include <QObject>
#include <QUrl>

class QComboBox;
class QTreeWidget;
class Autostart;

class AutoStartItem : public QTreeWidgetItem, public QObject
{
public:
    AutoStartItem( const QString &service, QTreeWidgetItem *parent, Autostart* );
    ~AutoStartItem();

    QUrl fileName() const;

    void setPath(const QString &path);

private:
    QUrl m_fileName;
};

class DesktopStartItem : public AutoStartItem
{
public:
    DesktopStartItem( const QString &service, QTreeWidgetItem *parent, Autostart* );
    ~DesktopStartItem();
};


class ScriptStartItem : public AutoStartItem
{
    Q_OBJECT

public:
    enum ENV { START=0, SHUTDOWN=1, PRE_START=2}; //rename
    ScriptStartItem( const QString &service, QTreeWidgetItem *parent, Autostart* );
    ~ScriptStartItem();

    void changeStartup( ScriptStartItem::ENV type );

Q_SIGNALS:
    void askChangeStartup(ScriptStartItem* item, int index);

private Q_SLOTS:

    void slotStartupChanged(int index);

private:
    QComboBox *m_comboBoxStartup;
};

#endif
