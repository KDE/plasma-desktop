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
    explicit Autostart( QWidget* parent, const QVariantList&  );
    ~Autostart() override;
    enum COL_TYPE { COL_NAME = 0, COL_COMMAND=1, COL_STATUS=2, COL_RUN=3 };
    void load() override;
    void save() override;
    void defaults() override;

public Q_SLOTS:
    void slotChangeStartup( ScriptStartItem *item, int index );

protected:
    void updateDesktopStartItem(DesktopStartItem *item, const QString &name, const QString &command, bool disabled, const QString &fileName);
    void updateScriptStartItem(ScriptStartItem *item, const QString &name, const QString &command, AutostartEntrySource type, const QString &fileName);

private Q_SLOTS:
    void slotAddProgram();
    void slotAddScript();
    void slotRemoveCMD();
    void slotEditCMD(QTreeWidgetItem*);
    void slotEditCMD();
    void slotSelectionChanged();
    void slotItemClicked( QTreeWidgetItem *, int);
    void slotAdvanced();
    void slotRowInserted(const QModelIndex &parent, int first, int last);
    void slotDatachanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

private:
    QModelIndex indexFromWidget(QTreeWidgetItem *widget) const;

    AutostartModel *m_model;
    QTreeWidgetItem *m_programItem;
    QTreeWidgetItem *m_scriptItem;

    Ui_AutostartConfig *widget;
};

#endif
