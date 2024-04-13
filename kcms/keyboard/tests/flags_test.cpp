/*
    SPDX-FileCopyrightText: 2011 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QIcon>
#include <QTest>

#include "../flags.h"
#include "../keyboard_config.h"
#include "../keyboardsettings.h"
#include "../x11_helper.h"
#include "../xkb_rules.h"

class FlagsTest : public QObject
{
    Q_OBJECT

    Flags *flags;
    const Rules *rules;

private Q_SLOTS:
    void initTestCase()
    {
        flags = new Flags();
        rules = nullptr;
    }

    void cleanupTestCase()
    {
        delete flags;
        delete rules;
    }

    void testRules()
    {
        QVERIFY(flags != nullptr);

        const QIcon iconUs(flags->getIcon(QStringLiteral("us")));
        QVERIFY(!iconUs.isNull());
        QVERIFY(flags->getIcon(QString()).isNull());

        KeyboardSettings *keyboardSettings = new KeyboardSettings(this);
        KeyboardConfig *keyboardConfig = new KeyboardConfig(keyboardSettings, this);
        LayoutUnit layoutUnit(QStringLiteral("us"));
        LayoutUnit layoutUnit1(QStringLiteral("us"), QStringLiteral("intl"));
        layoutUnit1.setDisplayName(QStringLiteral("usi"));
        LayoutUnit layoutUnit2(QStringLiteral("us"), QStringLiteral("other"));

        keyboardConfig->layouts().append(layoutUnit1);
        QCOMPARE(flags->getShortText(layoutUnit, *keyboardConfig), QString("us"));
        QCOMPARE(flags->getShortText(layoutUnit1, *keyboardConfig), QString("usi"));
        QCOMPARE(flags->getShortText(layoutUnit2, *keyboardConfig), QString("us"));

        const Rules *rules = Rules::readRules();
        QCOMPARE(flags->getLongText(layoutUnit, rules), QString("English (US)"));
        QCOMPARE(flags->getLongText(layoutUnit2, rules), QString("other"));

        rules = nullptr; // when no rules found
        QCOMPARE(flags->getLongText(layoutUnit1, rules), QString("us - intl"));
    }

    //    void loadRulesBenchmark() {
    //    	QBENCHMARK {
    //    		Flags* flags = new Flags();
    //    		delete flags;
    //    	}
    //    }
};

// need GUI for xkb protocol in xkb_rules.cpp
QTEST_MAIN(FlagsTest)

#include "flags_test.moc"
