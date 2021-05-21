/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
