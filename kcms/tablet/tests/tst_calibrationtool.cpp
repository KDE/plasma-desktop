/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "../calibrationtool.h"

#include <QtMath>
#include <QSignalSpy>

class CalibrationToolTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    static void verifyMatrixApproximatelyIdentity(const QMatrix4x4 &matrix)
    {
        for (int row = 0; row < 4; ++row) {
            for (int column = 0; column < 4; ++column) {
                const float expected = row == column ? 1.0f : 0.0f;
                QVERIFY(qAbs(matrix(row, column) - expected) < 0.0001f);
            }
        }
    }

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
        verifyMatrixApproximatelyIdentity(finishedSpy.at(0).at(0).value<QMatrix4x4>());
    }

    void testPerfectCalibrationOnNonSquareDisplay()
    {
        CalibrationTool tool;

        const QSignalSpy finishedSpy(&tool, &CalibrationTool::calibrationCreated);

        tool.setWidth(1920);
        tool.setHeight(1080);

        tool.calibrate(100.0, 100.0, 100.0, 100.0);
        tool.calibrate(1820.0, 100.0, 1820.0, 100.0);
        tool.calibrate(100.0, 980.0, 100.0, 980.0);
        tool.calibrate(1820.0, 980.0, 1820.0, 980.0);

        QCOMPARE(finishedSpy.count(), 1);
        verifyMatrixApproximatelyIdentity(finishedSpy.at(0).at(0).value<QMatrix4x4>());
    }

    void testTranslationCalibration()
    {
        CalibrationTool tool;

        const QSignalSpy finishedSpy(&tool, &CalibrationTool::calibrationCreated);

        tool.setWidth(500);
        tool.setHeight(500);

        tool.calibrate(60.0, 50.0, 50.0, 50.0);
        tool.calibrate(460.0, 50.0, 450.0, 50.0);
        tool.calibrate(60.0, 450.0, 50.0, 450.0);
        tool.calibrate(460.0, 450.0, 450.0, 450.0);

        QCOMPARE(finishedSpy.count(), 1);

        const QMatrix4x4 matrix = finishedSpy.at(0).at(0).value<QMatrix4x4>();

        QVERIFY(qAbs(matrix(0, 0) - 1.0f) < 0.0001f);
        QVERIFY(qAbs(matrix(1, 1) - 1.0f) < 0.0001f);
        QVERIFY(qAbs(matrix(0, 1)) < 0.0001f);
        QVERIFY(qAbs(matrix(1, 0)) < 0.0001f);
        QVERIFY(qAbs(matrix(0, 2) + 0.02f) < 0.0001f);
        QVERIFY(qAbs(matrix(1, 2)) < 0.0001f);
    }

    void testApplyCenterCorrectionAdjustsTranslationTerms()
    {
        CalibrationTool tool;

        tool.setWidth(500);
        tool.setHeight(400);

        QMatrix4x4 matrix;
        matrix(0, 2) = -0.01f;
        matrix(1, 2) = 0.02f;

        const double residualX = 10.0;
        const double residualY = -8.0;

        const QMatrix4x4 corrected = tool.matrixWithCenterCorrection(matrix, residualX, residualY);

        QVERIFY(qAbs(corrected(0, 2) + 0.03f) < 0.0001f);
        QVERIFY(qAbs(corrected(1, 2) - 0.04f) < 0.0001f);
    }
};

QTEST_MAIN(CalibrationToolTest)

#include "tst_calibrationtool.moc"
