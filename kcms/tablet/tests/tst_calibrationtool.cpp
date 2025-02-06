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

        const QSignalSpy widthSpy(&tool, &CalibrationTool::widthChanged);
        const QSignalSpy heightSpy(&tool, &CalibrationTool::heightChanged);
        const QSignalSpy finishedSpy(&tool, &CalibrationTool::calibrationCreated);
        const QSignalSpy targetChangedSpy(&tool, &CalibrationTool::currentTargetChanged);

        tool.setWidth(500);
        tool.setHeight(500);

        // The initial state should be in calibration mode
        QCOMPARE(tool.state(), CalibrationTool::State::Calibrating);

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

        // lower right, and this should begin the confimration process
        tool.calibrate(450.0, 450.0, 450.0, 450.0);
        QCOMPARE(targetChangedSpy.count(), 5);
        QCOMPARE(tool.state(), CalibrationTool::State::Confirming);

        // Ensure we get an identity matrix since every point was perfect
        QCOMPARE(finishedSpy.count(), 1);
        QCOMPARE(finishedSpy.at(0).at(0).value<QMatrix4x4>(), QMatrix4x4());
    }
};

QTEST_MAIN(CalibrationToolTest)

#include "tst_calibrationtool.moc"
