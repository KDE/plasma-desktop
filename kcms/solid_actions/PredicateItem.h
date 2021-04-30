/***************************************************************************
 *   Copyright 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>        *
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
