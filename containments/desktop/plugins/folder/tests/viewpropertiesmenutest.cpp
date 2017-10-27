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

#include "viewpropertiesmenutest.h"
#include "viewpropertiesmenu.h"
#include <QTest>
QTEST_MAIN(ViewPropertiesMenuTest)

void ViewPropertiesMenuTest::shouldHaveDefaultValues()
{
    ViewPropertiesMenu m;
    QVERIFY(m.menu());

    QVERIFY(m.showLayoutActions());
    QVERIFY(m.showLockAction());
    QVERIFY(m.showIconSizeActions());
    QVERIFY(!m.locked());
    QVERIFY(!m.sortDesc());
    QVERIFY(!m.sortDirsFirst());
}
