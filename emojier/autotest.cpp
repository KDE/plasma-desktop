/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#define QT_FORCE_ASSERTS 1

#include <QProcess>
#include <QSignalSpy>
#include <QTest>
#include <QThread>

using namespace Qt::StringLiterals;

class TestTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_improveCoverage();
};

void TestTest::test_improveCoverage()
{
    QProcess proc;
    proc.setProgram(u"qml6"_s);
    proc.setArguments({QFINDTESTDATA(u"app/ui/Emojier.qml"_s)});
    proc.start();
    proc.waitForStarted();
    QThread::sleep(3);
    proc.terminate();
    proc.waitForFinished();
}

QTEST_GUILESS_MAIN(TestTest)

#include "autotest.moc"
