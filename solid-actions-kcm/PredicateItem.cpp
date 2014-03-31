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

#include "PredicateItem.h"

#include "ActionEditor.h"

#include <QList>
#include <QString>

#include <KLocale>
#include <Solid/Predicate>

class PredicateItem::Private {

public:
    Private() {}

    PredicateItem * parent;
    QList<PredicateItem*> children;

};

PredicateItem::PredicateItem( Solid::Predicate item, PredicateItem * itsParent )
    : d( new Private() )
{
    d->parent = itsParent;

    if ( d->parent ) {
        d->parent->children().append( this );
    }

    // Read data from Solid::Predicate
    itemType = item.type();
    ifaceType = item.interfaceType();
    property = item.propertyName();
    value = item.matchingValue();
    compOperator = item.comparisonOperator();

    if( itemType == Solid::Predicate::Disjunction || itemType == Solid::Predicate::Conjunction ) {
        PredicateItem * child = new PredicateItem( item.firstOperand(), this );
        PredicateItem * child2 = new PredicateItem( item.secondOperand(), this );
        Q_UNUSED( child )
        Q_UNUSED( child2 )
    }
    // We're now ready, no need to keep the Solid::Predicate for now
}

PredicateItem::~PredicateItem()
{
    qDeleteAll( d->children );
    d->children.clear();
    delete d;
}

PredicateItem * PredicateItem::child( int index ) const
{
    return d->children.at(index);
}

PredicateItem * PredicateItem::parent() const
{
    return d->parent;
}

QList<PredicateItem*>& PredicateItem::children() const
{
    return d->children;
}

Solid::Predicate PredicateItem::predicate() const
{
    Solid::Predicate item;

    switch( itemType ) {
        case Solid::Predicate::InterfaceCheck:
            item = Solid::Predicate( ifaceType );
            break;
        case Solid::Predicate::Conjunction:
            item = children().at(0)->predicate() & children().at(1)->predicate();
            break;
        case Solid::Predicate::Disjunction:
            item = children().at(0)->predicate() | children().at(1)->predicate();
            break;
        default:
            break;
    }

    if( itemType != Solid::Predicate::PropertyCheck ) {
        return item;
    }

    switch( compOperator ) {
        case Solid::Predicate::Equals:
            item = Solid::Predicate( ifaceType, property, value );
            break;
        case Solid::Predicate::Mask:
            item = Solid::Predicate( ifaceType, property, value, Solid::Predicate::Mask );
            break;
        default:
            break;
    }

    return item;
}

QString PredicateItem::prettyName() const
{
    QString typeName;
    QString compName;

    QString deviceName;
    switch( itemType ) {
        case Solid::Predicate::InterfaceCheck:
            deviceName = SolidActionData::instance()->nameFromInterface(ifaceType);
            typeName = i18n("The device must be of the type %1", deviceName);
            break;
        case Solid::Predicate::Disjunction:
            typeName = i18n("Any of the contained properties must match");
            break;
        case Solid::Predicate::Conjunction:
            typeName = i18n("All of the contained properties must match");
            break;
        default:
            break;
    }

    QString prettyProperty = SolidActionData::instance()->propertyName( ifaceType, property );
    switch( compOperator ) {
        case Solid::Predicate::Equals:
            compName = i18n("The device property %1 must equal %2", prettyProperty, value.toString());
            break;
        case Solid::Predicate::Mask:
            compName = i18n("The device property %1 must contain %2", prettyProperty, value.toString());
            break;
        default:
            break;
    }

    if( itemType == Solid::Predicate::PropertyCheck ) {
        return compName;
    }
    return typeName;
}

void PredicateItem::setTypeByInt( int item )
{
    Solid::Predicate::Type iType = Solid::Predicate::InterfaceCheck;
    switch( item ) {
        case Solid::Predicate::PropertyCheck:
            iType = Solid::Predicate::PropertyCheck;
            break;
        case Solid::Predicate::Conjunction:
            iType = Solid::Predicate::Conjunction;
            break;
        case Solid::Predicate::Disjunction:
            iType = Solid::Predicate::Disjunction;
            break;
        case Solid::Predicate::InterfaceCheck:
            iType = Solid::Predicate::InterfaceCheck;
            break;
        default:
            break;
    }
    itemType = iType;
}

void PredicateItem::setComparisonByInt( int item )
{
    switch( item ) {
        case Solid::Predicate::Equals:
            compOperator = Solid::Predicate::Equals;
            break;
        case Solid::Predicate::Mask:
            compOperator = Solid::Predicate::Mask;
            break;
        default:
            break;
    }
}

void PredicateItem::updateChildrenStatus()
{
    if( itemType != Solid::Predicate::Disjunction && itemType != Solid::Predicate::Conjunction ) {
        qDeleteAll( d->children );
        d->children.clear();
    } else if( d->children.count() == 0 ) {
        Solid::Predicate templItem = Solid::Predicate::fromString("IS StorageVolume");
        new PredicateItem( templItem, this );
        new PredicateItem( templItem, this );
    }
}
