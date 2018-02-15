#ifndef __FILE_H__
#define __FILE_H__

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

#include <QSet>
#include <QMetaType>
#include <QDBusArgument>
#include "kfontinst_export.h"

class QDomElement;
class QTextStream;

namespace KFI
{

class KFONTINST_EXPORT File
{
    public:

    static bool equalIndex(int a, int b)
    {
        return a<=1 && b<=1;
    }

    File(const QString &pth=QString(), const QString &fndry=QString(), int idx=0)
        : itsPath(pth), itsFoundry(fndry), itsIndex(idx) { }
    File(const QDomElement &elem, bool disabled);

    bool operator==(const File &o) const
    {
        return equalIndex(itsIndex, o.itsIndex) && itsPath==o.itsPath;
    }

    QString toXml(bool disabledOnly, QTextStream &s) const;

    const QString & path() const       { return itsPath;  }
    const QString & foundry() const    { return itsFoundry; }
    int             index() const      { return itsIndex; }

    private:

    QString itsPath,
            itsFoundry;
    int     itsIndex;
};

typedef QSet<File> FileCont;

inline Q_DECL_EXPORT uint qHash(const File &key)
{
    return qHash(key.path()); // +qHash(key.index());
}

}

Q_DECLARE_METATYPE(KFI::File)
Q_DECL_EXPORT QDBusArgument & operator<<(QDBusArgument &argument, const KFI::File &obj);
Q_DECL_EXPORT const QDBusArgument & operator>>(const QDBusArgument &argument, KFI::File &obj);

#endif
