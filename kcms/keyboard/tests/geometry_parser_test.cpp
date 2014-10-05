/*
 *  Copyright (C) 2011 Andriy Rysin (rysin@kde.org)
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

#include <QtTest/QtTest>

#include "../preview/geometry_parser.h"


Q_LOGGING_CATEGORY(KCM_KEYBOARD, "kcm_keyboard")

class GeometryParserTest : public QObject
{
    Q_OBJECT


private Q_SLOTS:
    void initTestCase() {
    }

    void cleanupTestCase() {
    }

    void testGeometryParser() {
        QString model = "pc104";
        Geometry geometry = grammar::parseGeometry(model);

        QCOMPARE(geometry.getName(), model);

	model = "hpdv5";
        geometry = grammar::parseGeometry(model);
        
        QCOMPARE(geometry.getName(), QString("dv5"));

	model = "microsoftelite";
        geometry = grammar::parseGeometry(model);
        
//        QCOMPARE(geometry.getFile(), QString("microsoft"));
        QCOMPARE(geometry.getName(), QString("elite"));
    
/*
        QVERIFY( flags != NULL );

        QVERIFY( ! flags->getTransparentPixmap().isNull() );

        const QIcon iconUs(flags->getIcon("us"));
        QVERIFY( ! iconUs.isNull() );
        QVERIFY( flags->getIcon("--").isNull() );

    	KeyboardConfig keyboardConfig;
        LayoutUnit layoutUnit("us");
        LayoutUnit layoutUnit1("us", "intl");
        layoutUnit1.setDisplayName("usi");
        LayoutUnit layoutUnit2("us", "other");

        keyboardConfig.indicatorType = KeyboardConfig::SHOW_FLAG;
        const QIcon iconUsFlag = flags->getIconWithText(layoutUnit, keyboardConfig);
        QVERIFY( ! iconUsFlag.isNull() );
        QCOMPARE( image(iconUsFlag), image(iconUs) );

        keyboardConfig.indicatorType = KeyboardConfig::SHOW_LABEL;
        const QIcon iconUsText = flags->getIconWithText(layoutUnit, keyboardConfig);
        QVERIFY( ! iconUsText.isNull() );
        QVERIFY( image(iconUsText) != image(iconUs) );

        keyboardConfig.layouts.append(layoutUnit1);
        QCOMPARE( flags->getShortText(layoutUnit, keyboardConfig), QString("us") );
        QCOMPARE( flags->getShortText(layoutUnit1, keyboardConfig), QString("usi") );
        QCOMPARE( flags->getShortText(layoutUnit2, keyboardConfig), QString("us") );

        const Rules* rules = Rules::readRules(Rules::NO_EXTRAS);
        QCOMPARE( flags->getLongText(layoutUnit, rules), QString("English (US)") );
        QVERIFY( flags->getLongText(layoutUnit1, rules).startsWith("English (US, international with dead keys)") );
        QCOMPARE( flags->getLongText(layoutUnit2, rules), QString("other") );

        rules = NULL; // when no rules found
        QCOMPARE( flags->getLongText(layoutUnit1, rules), QString("us - intl") );

        flags->clearCache();
*/
    }

};

// need kde libs for config-workspace.h used in xkb_rules.cpp
// need GUI for xkb protocol
QTEST_MAIN(GeometryParserTest)

#include "geometry_parser_test.moc"

