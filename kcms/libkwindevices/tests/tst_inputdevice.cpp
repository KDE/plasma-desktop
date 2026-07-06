/*
    SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "../inputdevice.h"

#include <QSignalSpy>

using namespace KWinDevices;

class InputDeviceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // Ensure we can actually do a round-trip serialization of matrices.
    void testMatrix()
    {
        QMatrix4x4 matrix{-1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

        QCOMPARE(InputDevice::deserializeMatrix(InputDevice::serializeMatrix(matrix)), matrix);
    }
};

QTEST_MAIN(InputDeviceTest)

#include "tst_inputdevice.moc"
