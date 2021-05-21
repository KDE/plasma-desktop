/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PREDICATEITEM_H
#define PREDICATEITEM_H

#include <Solid/Predicate>

class QString;
template<typename T>
class QList;

class PredicateItem
{
public:
    PredicateItem(Solid::Predicate item, PredicateItem *itsParent);
    ~PredicateItem();

    PredicateItem *child(int index) const;
    PredicateItem *parent() const;
    QList<PredicateItem *> &children() const;
    Solid::Predicate predicate() const;
    QString prettyName() const;
    void setTypeByInt(int item);
    void setComparisonByInt(int item);
    void updateChildrenStatus();

    Solid::Predicate::Type itemType;
    Solid::DeviceInterface::Type ifaceType;
    QString property;
    QVariant value;
    Solid::Predicate::ComparisonOperator compOperator;

private:
    class Private;
    Private *const d;
};

Q_DECLARE_METATYPE(PredicateItem *)

#endif
