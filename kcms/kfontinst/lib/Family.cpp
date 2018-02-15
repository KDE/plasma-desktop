/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2009 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QTextStream>
#include <QDomElement>
#include <QDebug>
#include "Family.h"
#include "Misc.h"
#include "XmlStrings.h"

#define KFI_DBUG qDebug() << time(0L)

namespace KFI
{

Family::Family(const QDomElement &elem, bool loadStyles)
{
    if(elem.hasAttribute(FAMILY_ATTR))
        itsName=elem.attribute(FAMILY_ATTR);
    if(elem.hasAttribute(NAME_ATTR))
        itsName=elem.attribute(NAME_ATTR);
    if(loadStyles)
    {
        for(QDomNode n=elem.firstChild(); !n.isNull(); n=n.nextSibling())
        {
            QDomElement ent=n.toElement();

            if(FONT_TAG==ent.tagName())
            {
                Style style(ent, loadStyles);

                if(!style.files().isEmpty())
                    itsStyles.insert(style);
            }
        }
    }
}

void Family::toXml(bool disabled, QTextStream &s) const
{
    QString                  family(KFI::Misc::encodeText(itsName, s));
    QStringList              entries;
    StyleCont::ConstIterator it(itsStyles.begin()),
                             end(itsStyles.end());

    for(; it!=end; ++it)
    {
        QString entry((*it).toXml(disabled, disabled ? family : QString(), s));

        if(!entry.isEmpty())
            entries.append(entry);
    }

    if(entries.count()>0)
    {
        if(!disabled)
            s << " <" FAMILY_TAG " " NAME_ATTR "=\"" << KFI::Misc::encodeText(itsName, s) << "\">\n";

        QStringList::ConstIterator it(entries.begin()),
                                   end(entries.end());

        for(; it!=end; ++it)
            s << *it << endl;

        if(!disabled)
            s << " </" FAMILY_TAG ">" << endl;
    }
}

}

QDBusArgument & operator<<(QDBusArgument &argument, const KFI::Family &obj)
{
    argument.beginStructure();
    argument << obj.name();

    argument.beginArray(qMetaTypeId<KFI::Style>());
    KFI::StyleCont::ConstIterator it(obj.styles().begin()),
                                  end(obj.styles().end());
    for(; it!=end; ++it)
        argument << *it;
    argument.endArray();
    argument.endStructure();
    return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument, KFI::Family &obj)
{
    QString name;
    argument.beginStructure();
    argument >> name;
    obj=KFI::Family(name);
    argument.beginArray();
    while(!argument.atEnd())
    {
        KFI::Style st;
        argument >> st;
        obj.add(st);
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}

QDBusArgument & operator<<(QDBusArgument &argument, const KFI::Families &obj)
{
    argument.beginStructure();
    argument << obj.isSystem;

    argument.beginArray(qMetaTypeId<KFI::Family>());
    KFI::FamilyCont::ConstIterator it(obj.items.begin()),
                                   end(obj.items.end());

    for(; it!=end; ++it)
        argument << *it;
    argument.endArray();
    argument.endStructure();
    return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument, KFI::Families &obj)
{
    argument.beginStructure();
    argument >> obj.isSystem;
    argument.beginArray();
    while(!argument.atEnd())
    {
        KFI::Family fam;
        argument >> fam;
        obj.items.insert(fam);
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}
