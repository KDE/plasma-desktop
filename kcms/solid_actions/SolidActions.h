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

#ifndef SOLIDACTIONS_H
#define SOLIDACTIONS_H

#include <KCModule>

#include "ActionEditor.h"
#include "ActionModel.h"
#include "ui_AddAction.h"
#include "ui_SolidActions.h"

class ActionItem;

class SolidActions : public KCModule
{
    Q_OBJECT

public:
    SolidActions(QWidget *parent, const QVariantList &);
    ~SolidActions() override;
    void load() override;
    void save() override;
    void defaults() override;

private Q_SLOTS:
    void addAction();
    void editAction();
    void deleteAction();
    ActionItem *selectedAction() const;
    void fillActionsList();
    void acceptActionChanges();
    void toggleEditDelete();
    void slotTextChanged(const QString &);
    void slotShowAddDialog();

private:
    Ui::SolidActions mainUi;
    ActionModel *actionModel;
    ActionEditor *editUi;
    Ui::AddAction addUi;
    QDialog *addDialog;
};

#endif
