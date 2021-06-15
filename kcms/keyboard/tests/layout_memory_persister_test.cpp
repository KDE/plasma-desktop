/*
    SPDX-FileCopyrightText: 2011 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>
#include <QtTest>

#include "../keyboard_config.h"
#include "../layout_memory.h"
#include "../layout_memory_persister.h"

class TestLayoutMemory : public LayoutMemory
{
public:
    TestLayoutMemory(const KeyboardConfig &keyboardConfig)
        : LayoutMemory(keyboardConfig)
    {
    }
    QMap<QString, LayoutSet> &getLayoutMap()
    {
        return layoutMap;
    }
};

class LayoutMemoryPersisterTest : public QObject
{
    Q_OBJECT

    QString path;
    KeyboardConfig keyboardConfig;
    TestLayoutMemory *layoutMemory;
    LayoutMemoryPersister *layoutMemoryPersister;

    const LayoutUnit layoutUnit1;
    const LayoutUnit layoutUnit2;
    const LayoutUnit layoutUnit3;

public:
    LayoutMemoryPersisterTest()
        : layoutUnit1("xx")
        , layoutUnit2("yy", "var1")
        , layoutUnit3("zz", "var2")
    {
    }

private Q_SLOTS:
    void initTestCase()
    {
        path = "keyboard_memory_test.xml";
        QFile(path).remove();

        layoutMemory = new TestLayoutMemory(keyboardConfig);
        layoutMemoryPersister = new LayoutMemoryPersister(*layoutMemory);

        //    	QFile(path).remove();
    }

    void cleanupTestCase()
    {
        delete layoutMemoryPersister;
        delete layoutMemory;
    }

    void testSaveNA()
    {
        QFile file(path);

        keyboardConfig.switchingPolicy = KeyboardConfig::SWITCH_POLICY_WINDOW;
        QVERIFY(!layoutMemoryPersister->saveToFile(file));
        QVERIFY(QFile(path).size() == 0);

        QVERIFY(!layoutMemoryPersister->restoreFromFile(file));
    }

    void testSaveGlobal()
    {
        QFile file(path);

        keyboardConfig.switchingPolicy = KeyboardConfig::SWITCH_POLICY_GLOBAL;
        layoutMemoryPersister->setGlobalLayout(layoutUnit1);
        QVERIFY(layoutMemoryPersister->saveToFile(file));
        QVERIFY(QFile(path).size() > 0);
        //        file.close();

        keyboardConfig.layouts.clear();

        QVERIFY(layoutMemoryPersister->restoreFromFile(file));
        QVERIFY(!layoutMemoryPersister->getGlobalLayout().isValid());

        //        file.close();
        keyboardConfig.layouts << layoutUnit1;
        QVERIFY(layoutMemoryPersister->restoreFromFile(file));
        QVERIFY(layoutUnit1.isValid());
        QVERIFY(layoutMemoryPersister->getGlobalLayout().isValid());
        QCOMPARE(layoutMemoryPersister->getGlobalLayout(), layoutUnit1);
    }

    void testSaveByApp()
    {
        QFile file(path);

        keyboardConfig.switchingPolicy = KeyboardConfig::SWITCH_POLICY_APPLICATION;

        layoutMemoryPersister->setGlobalLayout(LayoutUnit());
        layoutMemory->getLayoutMap().clear();

        keyboardConfig.layouts.clear();
        keyboardConfig.layouts << layoutUnit1 << layoutUnit2;

        LayoutSet layoutSet1;
        layoutSet1.layouts << layoutUnit1 << layoutUnit2;
        layoutSet1.currentLayout = layoutUnit1;
        layoutMemory->getLayoutMap().insert(QString("app1"), layoutSet1);

        LayoutSet layoutSet2;
        layoutSet2.layouts << layoutUnit2 << layoutUnit1 << layoutUnit3;
        layoutSet2.currentLayout = layoutUnit2;
        layoutMemory->getLayoutMap().insert(QString("app2"), layoutSet2);

        QVERIFY(layoutMemoryPersister->saveToFile(file));
        QVERIFY(QFile(path).size() > 0);
        file.close();

        layoutMemory->getLayoutMap().clear();
        QVERIFY(!layoutMemory->getLayoutMap().value("app1").isValid());
        QVERIFY(!layoutMemory->getLayoutMap().value("app2").isValid());

        QVERIFY(layoutMemoryPersister->restoreFromFile(file));
        QVERIFY(!layoutMemoryPersister->getGlobalLayout().isValid());
        QCOMPARE(layoutMemory->getLayoutMap().value("app1"), layoutSet1);
        QVERIFY(!layoutMemory->getLayoutMap().value("app2").isValid());

        keyboardConfig.layouts << layoutUnit3;

        file.close();
        QVERIFY(layoutMemoryPersister->restoreFromFile(file));
        QVERIFY(!layoutMemoryPersister->getGlobalLayout().isValid());
        QCOMPARE(layoutMemory->getLayoutMap().value("app1"), layoutSet1);
        QCOMPARE(layoutMemory->getLayoutMap().value("app2"), layoutSet2);
    }

    //    void layoutMemroyPersisterBenchmark() {
    //    	QBENCHMARK {
    //    		//TODO: generate big map
    //    		layoutMemoryPersister->save();
    //    		layoutMemoryPersister->restore();
    //    	}
    //    }
};

QTEST_MAIN(LayoutMemoryPersisterTest)

#include "layout_memory_persister_test.moc"
