/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "../calibrationtool.h"

#include <QSignalSpy>

class CalibrationToolTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // Check if hitting every point 1:1 generates an identity matrix
    void testPerfectCalibration()
    {
        CalibrationTool tool;

        const QSignalSpy widthSpy(&tool, SIGNAL(widthChanged()));
        const QSignalSpy heightSpy(&tool, SIGNAL(heightChanged()));
        const QSignalSpy finishedSpy(&tool, SIGNAL(finished(QMatrix4x4)));
        const QSignalSpy targetChangedSpy(&tool, SIGNAL(currentTargetChanged()));

        tool.setWidth(500);
        tool.setHeight(500);

        // Ensure screen width/height are set correctly
        QCOMPARE(widthSpy.count(), 1);
        QCOMPARE(heightSpy.count(), 1);
        QCOMPARE(tool.width(), 500);
        QCOMPARE(tool.height(), 500);

        // These shouldn't have been hit yet, make sure they aren't
        QCOMPARE(finishedSpy.count(), 0);
        QCOMPARE(targetChangedSpy.count(), 0);

        // upper left
        tool.calibrate(50.0, 50.0, 50.0, 50.0);
        QCOMPARE(targetChangedSpy.count(), 1);

        // upper right
        tool.calibrate(450.0, 50.0, 450.0, 50.0);
        QCOMPARE(targetChangedSpy.count(), 2);

        // lower left
        tool.calibrate(50.0, 450.0, 50.0, 450.0);
        QCOMPARE(targetChangedSpy.count(), 3);

        // lower right
        tool.calibrate(450.0, 450.0, 450.0, 450.0);
        QCOMPARE(targetChangedSpy.count(), 4);

        // Ensure we get an identity matrix since every point was perfect
        QCOMPARE(finishedSpy.count(), 1);
        QCOMPARE(finishedSpy.at(0).at(0).value<QMatrix4x4>(), QMatrix4x4());
    }
};

QTEST_MAIN(CalibrationToolTest)

#include "tst_calibrationtool.moc"
