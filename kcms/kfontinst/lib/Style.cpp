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
#include <QStringList>
#include <QDomElement>
#include "Fc.h"
#include "Style.h"
#include "WritingSystems.h"
#include "XmlStrings.h"

namespace KFI
{

Style::Style(const QDomElement &elem, bool loadFiles)
{
    bool ok;
    int  weight(KFI_NULL_SETTING), width(KFI_NULL_SETTING), slant(KFI_NULL_SETTING),
         tmp(KFI_NULL_SETTING);

    if(elem.hasAttribute(WEIGHT_ATTR))
    {
        tmp=elem.attribute(WEIGHT_ATTR).toInt(&ok);
        if(ok)
            weight=tmp;
    }
    if(elem.hasAttribute(WIDTH_ATTR))
    {
        tmp=elem.attribute(WIDTH_ATTR).toInt(&ok);
        if(ok)
            width=tmp;
    }

    if(elem.hasAttribute(SLANT_ATTR))
    {
        tmp=elem.attribute(SLANT_ATTR).toInt(&ok);
        if(ok)
            slant=tmp;
    }

    itsScalable=!elem.hasAttribute(SCALABLE_ATTR) || elem.attribute(SCALABLE_ATTR)!="false";
    itsValue=FC::createStyleVal(weight, width, slant);
    itsWritingSystems=0;

    if(elem.hasAttribute(LANGS_ATTR))
        itsWritingSystems=WritingSystems::instance()->get(elem.attribute(LANGS_ATTR).split(LANG_SEP, Qt::SkipEmptyParts));

    if(loadFiles)
    {
        if(elem.hasAttribute(PATH_ATTR))
        {
            File file(elem, false);

            if(!file.path().isEmpty())
                itsFiles.insert(file);
        }
        else
        {
            for(QDomNode n=elem.firstChild(); !n.isNull(); n=n.nextSibling())
            {
                QDomElement ent=n.toElement();

                if(FILE_TAG==ent.tagName())
                {
                    File file(ent, false);

                    if(!file.path().isEmpty())
                        itsFiles.insert(file);
                }
            }
        }
    }
}

QString Style::toXml(bool disabled, const QString &family, QTextStream &s) const
{
    QStringList             files;
    FileCont::ConstIterator it(itsFiles.begin()),
                            end(itsFiles.end());

    for(; it!=end; ++it)
    {
        QString f((*it).toXml(disabled, s));

        if(!f.isEmpty())
            files.append(f);
    }

    if(files.count()>0)
    {
        QString str("  <" FONT_TAG " ");
        int     weight,
                width,
                slant;

        KFI::FC::decomposeStyleVal(itsValue, weight, width, slant);

        if(!family.isEmpty())
            str+=FAMILY_ATTR"=\""+family+"\" ";
        if(KFI_NULL_SETTING!=weight)
            str+=WEIGHT_ATTR"=\""+QString::number(weight)+"\" ";
        if(KFI_NULL_SETTING!=width)
            str+=WIDTH_ATTR"=\""+QString::number(width)+"\" ";
        if(KFI_NULL_SETTING!=slant)
            str+=SLANT_ATTR"=\""+QString::number(slant)+"\" ";
        if(!itsScalable)
            str+=SCALABLE_ATTR"=\"false\" ";

        QStringList ws(WritingSystems::instance()->getLangs(itsWritingSystems));

        if(!ws.isEmpty())
            str+=LANGS_ATTR"=\""+ws.join(LANG_SEP)+"\" ";


        if(1==files.count())
           str+=(*files.begin())+"/>";
        else
        {
            QStringList::ConstIterator it(files.begin()),
                                       end(files.end());

            str+=">\n";
            for(; it!=end; ++it)
                str+="   <" FILE_TAG " "+(*it)+"/>\n";
            str+="  </" FONT_TAG ">";
        }

        return str;
    }

    return QString();
}

}

QDBusArgument & operator<<(QDBusArgument &argument, const KFI::Style &obj)
{
    argument.beginStructure();

    argument << obj.value() << obj.scalable() << obj.writingSystems();
    argument.beginArray(qMetaTypeId<KFI::File>());
    KFI::FileCont::ConstIterator it(obj.files().begin()),
                                 end(obj.files().end());
    for(; it!=end; ++it)
        argument << *it;
    argument.endArray();
    argument.endStructure();
    return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument, KFI::Style &obj)
{
    quint32    value;
    bool       scalable;
    qulonglong ws;
    argument.beginStructure();
    argument >> value >> scalable >> ws;
    obj=KFI::Style(value, scalable, ws);
    argument.beginArray();
    while(!argument.atEnd())
    {
        KFI::File f;
        argument >> f;
        obj.add(f);
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}
