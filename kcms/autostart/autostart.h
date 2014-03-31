/***************************************************************************
 *   Copyright (C) 2006-2007 by Stephen Leaf                               *
 *   smileaf@gmail.com                                                     *
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


#ifndef AUTOSTART_H
#define AUTOSTART_H

#include <QPushButton>
#include <QTreeWidget>

#include <KCModule>
#include <KFileItem>

#include "ui_autostartconfig.h"
#include "autostartitem.h"

class Autostart: public KCModule
{
    Q_OBJECT

public:
    Autostart( QWidget* parent, const QVariantList&  );
    ~Autostart();
    enum COL_TYPE { COL_NAME = 0, COL_COMMAND=1, COL_STATUS=2,COL_RUN=3 };
    void load();
    void save();
    void defaults();

    QStringList listPathName() const { return m_pathName;}

public Q_SLOTS:
    void slotChangeStartup( ScriptStartItem* item, int index );

protected:
    void addItem(DesktopStartItem *item, const QString& name, const QString& run, const QString& command, bool disabled );
    void addItem(ScriptStartItem *item, const QString& name, const QString& command, ScriptStartItem::ENV type );

private Q_SLOTS:
    void slotAddProgram();
    void slotAddScript();
    void slotRemoveCMD();
    void slotEditCMD(QTreeWidgetItem*);
    bool slotEditCMD(const KFileItem&);
    void slotEditCMD();
    void slotSelectionChanged();
    void slotItemClicked( QTreeWidgetItem *, int);
    void slotAdvanced();

private:
    QTreeWidgetItem *m_programItem, *m_scriptItem;
    QStringList m_paths;
    QStringList m_pathName;

    Ui_AutostartConfig *widget;
};

#endif
