/*
    SPDX-FileCopyrightText: 2011 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QtTest>

#include "../iso_codes.h"

class IsoCodesTest : public QObject
{
    Q_OBJECT

    IsoCodes *isoCodes;

private Q_SLOTS:
    void initTestCase()
    {
        //    	isoCodes = new IsoCodes(IsoCodes::iso_639);
        isoCodes = new IsoCodes(IsoCodes::iso_639_3);
    }

    void cleanupTestCase()
    {
        delete isoCodes;
    }

    void testIsoCodes()
    {
        QVERIFY(isoCodes != nullptr);
        QVERIFY(!isoCodes->getEntryList().isEmpty());
        //        const IsoCodeEntry* isoEntry = isoCodes->getEntry(IsoCodes::attr_iso_639_2T_code, "eng");
        const IsoCodeEntry *isoEntry = isoCodes->getEntry(IsoCodes::attr_iso_639_3_id, QStringLiteral("eng"));
        QVERIFY(isoEntry != nullptr);
        QVERIFY(!isoEntry->empty());
        //        QCOMPARE( isoEntry->value(IsoCodes::attr_iso_639_2T_code), QString("eng") );
        //        QCOMPARE( isoEntry->value(IsoCodes::attr_iso_639_2B_code), QString("eng") );
        //        QCOMPARE( isoEntry->value(IsoCodes::attr_iso_639_1_code), QString("en") );
        QCOMPARE(isoEntry->value("name"), QString("English"));
        //        QCOMPARE( isoEntry->value("status"), QString("Active") );
    }

    void testIso639_3_Codes()
    {
        QVERIFY(isoCodes != nullptr);
        QVERIFY(!isoCodes->getEntryList().isEmpty());
        const IsoCodeEntry *isoEntry = isoCodes->getEntry(IsoCodes::attr_iso_639_3_id, QStringLiteral("ant"));
        QVERIFY(isoEntry != nullptr);
        QVERIFY(!isoEntry->empty());
        QVERIFY(isoEntry->value("name") != QString("ant"));
        QCOMPARE(isoEntry->value("name"), QString("Antakarinya"));
    }

    void loadIsoCodesBenchmark()
    {
        QBENCHMARK {
            IsoCodes *isoCodes = new IsoCodes(IsoCodes::iso_639_3);
            delete isoCodes;
        }
    }
};

QTEST_MAIN(IsoCodesTest)

#include "iso_codes_test.moc"
