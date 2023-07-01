/*
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <QFileInfo>
#include <QObject>
#include <QProcess>
#include <QStandardPaths>
#include <QTest>
#include <QTextStream>

bool fileContainsLine(const QString &fileName, const QString &line)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        if (in.readLine() == line) {
            return true;
        }
    }

    return false;
}

class PluginInstallerRcTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        qputenv("PLUGININSTALLER_TEST_MODE", "true");
    }
    void testInstall()
    {
        const QString executable = QFINDTESTDATA("krunner-plugininstaller");
        QProcess installProcess;
        installProcess.start(executable, {"install", QFINDTESTDATA("testplugin"), "--no-confirm"});

        installProcess.waitForFinished();
        qDebug() << installProcess.readAllStandardError();
        QCOMPARE(installProcess.exitCode(), 0);

        const QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        const QString desktopLocation = dataLocation + "/krunner/dbusplugins/org.kde.testplugin.desktop";
        const QString serviceLocation = dataLocation + "/dbus-1/services/org.kde.testplugin.service";
        QVERIFY(QFileInfo::exists(desktopLocation));
        QVERIFY(QFileInfo::exists(serviceLocation));
        QVERIFY(fileContainsLine(desktopLocation, "X-Plasma-DBusRunner-Service=org.kde.testplugin"));
        QVERIFY(fileContainsLine(serviceLocation, QLatin1String("Exec=python3 %1/main.py").arg(QFINDTESTDATA("testplugin"))));

        QProcess uninstallProcess;
        uninstallProcess.start(executable, {"uninstall", QFINDTESTDATA("testplugin")});

        uninstallProcess.waitForFinished();
        qDebug() << uninstallProcess.readAllStandardError();
        QVERIFY(!QFileInfo::exists(desktopLocation));
        QVERIFY(!QFileInfo::exists(serviceLocation));
    }
};

QTEST_MAIN(PluginInstallerRcTest)

#include "plugininstallerrctest.moc"
