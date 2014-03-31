/*
 *  Copyright (C) 2012 Shivam Makkar (amourphious1992@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef ALIASES_H
#define ALIASES_H

#include <QtCore/QMap>

class Aliases
{
private:
	QMap<QString,QString>qwerty;
    QMap<QString,QString>azerty;
    QMap<QString,QString>qwertz;
	QString findaliasdir();
public:
    Aliases();
    QString getAlias(const QString &type, const QString &name);
};

#endif // ALIASES_H
