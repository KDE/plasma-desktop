/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: 2017 Laurent Montel <laurent.montel@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
