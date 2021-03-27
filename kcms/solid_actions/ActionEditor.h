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

#ifndef ACTIONEDITOR_H
#define ACTIONEDITOR_H

#include <QDialog>

#include "ActionItem.h"
#include "PredicateItem.h"
#include "PredicateModel.h"
#include "SolidActionData.h"
#include "ui_ActionEditor.h"

class ActionEditor : public QDialog
{
    Q_OBJECT
public:
    explicit ActionEditor(QWidget *parent = nullptr);
    ~ActionEditor() override;

    void setActionToEdit(ActionItem *item);

public Q_SLOTS:
    void accept() override;

private:
    SolidActionData *actionData();
    QString predicateString();

    Ui::ActionEditor ui;
    ActionItem *activeItem;
    PredicateItem *topItem;
    PredicateItem *rootItem;
    PredicateModel *rootModel;

private Q_SLOTS:
    void updatePropertyList();
    void manageControlStatus();
    void updateParameter();
    void saveParameter();
    void setPredicate(Solid::Predicate predicate);
};

#endif
