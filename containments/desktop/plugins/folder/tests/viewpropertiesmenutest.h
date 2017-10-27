/***************************************************************************
 *   Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company *
 *                      <info@kdab.com>                                    *
 *   Author: Laurent Montel <laurent.montel@kdab.com>                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef VIEWPROPERTIESMENUTEST_H
#define VIEWPROPERTIESMENUTEST_H

#include <QObject>

class ViewPropertiesMenuTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void shouldHaveDefaultValues();
};

#endif // VIEWPROPERTIESMENUTEST_H
