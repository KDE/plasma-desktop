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

#include <QDomElement>
#include <QTextStream>
#include <QDir>
#include "File.h"
#include "Misc.h"
#include "XmlStrings.h"

namespace KFI
{

static QString expandHome(const QString &path)
{
    QString p(path);

    return !p.isEmpty() && '~'==p[0]
        ? 1==p.length() ? QDir::homePath() : p.replace(0, 1, QDir::homePath())
        : p;
}

File::File(const QDomElement &elem, bool disabled)
{
    QString path=expandHome(elem.attribute(PATH_ATTR));

    if(!disabled || Misc::fExists(path)) // Only need to check instance if we are loading the disabled file...
    {
        itsFoundry=elem.attribute(FOUNDRY_ATTR);
        itsIndex=elem.hasAttribute(FACE_ATTR) ? elem.attribute(FACE_ATTR).toInt() : 0;
        itsPath=path;
    }
}
    
QString File::toXml(bool disabled, QTextStream &s) const
{
    if(!disabled || Misc::isHidden(Misc::getFile(itsPath)))
    {
        QString str(PATH_ATTR"=\""+KFI::Misc::encodeText(KFI::Misc::contractHome(itsPath), s)+"\"");

        if(!itsFoundry.isEmpty() && QString::fromLatin1("unknown")!=itsFoundry)
            str+=" " FOUNDRY_ATTR "=\""+KFI::Misc::encodeText(itsFoundry, s)+"\"";

        if(itsIndex>0)
            str+=" " FACE_ATTR "=\""+QString::number(itsIndex)+"\"";

        return str;
    }

    return QString();
}

}

QDBusArgument & operator<<(QDBusArgument &argument, const KFI::File &obj)
{
    argument.beginStructure();
    argument << obj.path() << obj.foundry() << obj.index();
    argument.endStructure();
    return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument, KFI::File &obj)
{
    QString path,
            foundry;
    int     index;
    argument.beginStructure();
    argument >> path >> foundry >> index;
    obj=KFI::File(path, foundry, index);
    argument.endStructure();
    return argument;
}
