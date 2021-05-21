/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIONMODEL_H
#define ACTIONMODEL_H

#include <QAbstractTableModel>

class ActionItem;

class ActionModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ActionModel(QObject *parent = nullptr);
    ~ActionModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void buildActionList();
    QList<ActionItem *> actionList() const;

private:
    class Private;
    Private *const d;
};

#endif
