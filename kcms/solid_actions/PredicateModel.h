/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PREDICATEMODEL_H
#define PREDICATEMODEL_H

#include <QAbstractItemModel>

#include <Solid/Predicate>

class PredicateItem;

class PredicateModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit PredicateModel(PredicateItem *menuRoot, QObject *parent = nullptr);
    ~PredicateModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void setRootPredicate(PredicateItem *item);
    void itemUpdated(const QModelIndex &item);
    void childrenChanging(const QModelIndex &item, Solid::Predicate::Type oldType);

protected:
    PredicateItem *rootItem() const;

private:
    class Private;
    Private *const d;
};

#endif
