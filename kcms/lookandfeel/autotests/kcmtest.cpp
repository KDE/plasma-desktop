/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 2014 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "../kcm.h"
// Qt
#include <QtTest/QtTest>
#include <Plasma/Package>
#include <Plasma/PluginLoader>
#include <ksycoca.h>
#include <KJob>

class KcmTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testWidgetStyle();
    void testColors();
    void testIcons();
    void testPlasmaTheme();
    void testCursorTheme();
    void testSplashScreen();
    void testLockScreen();
    void testWindowSwitcher();
    void testDesktopSwitcher();
    void testKCMSave();

private:
    QDir m_configDir;
    QDir m_dataDir;
    KCMLookandFeel *m_KCMLookandFeel;
};


void KcmTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    m_configDir = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    m_configDir.removeRecursively();

    m_dataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    m_dataDir.removeRecursively();

    QVERIFY(m_configDir.mkpath("."));

    const QString packagePath = QFINDTESTDATA("lookandfeel");

    Plasma::Package p = Plasma::PluginLoader::self()->loadPackage("Plasma/LookAndFeel");
    p.setPath(packagePath);
    QVERIFY(p.isValid());

    const QString packageRoot = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/plasma/look-and-feel/";
    auto installJob = p.install(packagePath, packageRoot);
    installJob->exec();

    KConfig config("kdeglobals");
    KConfigGroup cg(&config, "KDE");
    cg.writeEntry("LookAndFeelPackage", "org.kde.test");
    cg.sync();
    m_KCMLookandFeel = new KCMLookandFeel(0, QVariantList());
    m_KCMLookandFeel->load();
}

void KcmTest::cleanupTestCase()
{
    m_configDir.removeRecursively();
    m_dataDir.removeRecursively();
}


void KcmTest::testWidgetStyle()
{
    m_KCMLookandFeel->setWidgetStyle("customTestValue");

    KConfig config("kdeglobals");
    KConfigGroup cg(&config, "KDE");
    QCOMPARE(cg.readEntry("widgetStyle", QString()), QString("customTestValue"));
}

void KcmTest::testColors()
{
    //TODO: test colorFile as well
    m_KCMLookandFeel->setColors("customTestValue", QString());

    KConfig config("kdeglobals");
    KConfigGroup cg(&config, "General");
    QCOMPARE(cg.readEntry("ColorScheme", QString()), QString("customTestValue"));
}

void KcmTest::testIcons()
{
    m_KCMLookandFeel->setIcons("customTestValue");

    KConfig config("kdeglobals");
    KConfigGroup cg(&config, "Icons");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("customTestValue"));
}

void KcmTest::testPlasmaTheme()
{
    m_KCMLookandFeel->setPlasmaTheme("customTestValue");

    KConfig config("plasmarc");
    KConfigGroup cg(&config, "Theme");
    QCOMPARE(cg.readEntry("name", QString()), QString("customTestValue"));
}

void KcmTest::testCursorTheme()
{
    m_KCMLookandFeel->setCursorTheme("customTestValue");

    KConfig config("kcminputrc");
    KConfigGroup cg(&config, "Mouse");
    QCOMPARE(cg.readEntry("cursorTheme", QString()), QString("customTestValue"));
}

void KcmTest::testSplashScreen()
{
    m_KCMLookandFeel->setSplashScreen("customTestValue");

    KConfig config("ksplashrc");
    KConfigGroup cg(&config, "KSplash");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("customTestValue"));
    QCOMPARE(cg.readEntry("Engine", QString()), QString("KSplashQML"));
}

void KcmTest::testLockScreen()
{
    m_KCMLookandFeel->setLockScreen("customTestValue");

    KConfig config("kscreenlockerrc");
    KConfigGroup cg(&config, "Greeter");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("customTestValue"));
}

void KcmTest::testWindowSwitcher()
{
    m_KCMLookandFeel->setWindowSwitcher("customTestValue");

    KConfig config("kwinrc");
    KConfigGroup cg(&config, "TabBox");
    QCOMPARE(cg.readEntry("LayoutName", QString()), QStringLiteral("customTestValue"));
}

void KcmTest::testDesktopSwitcher()
{
    m_KCMLookandFeel->setDesktopSwitcher("customTestValue");

    KConfig config("kwinrc");
    KConfigGroup cg(&config, "TabBox");
    QCOMPARE(cg.readEntry("DesktopLayout", QString()), QStringLiteral("customTestValue"));
    QCOMPARE(cg.readEntry("DesktopListLayout", QString()), QStringLiteral("customTestValue"));
}

void KcmTest::testKCMSave()
{
    m_KCMLookandFeel->save();

    KConfig config("kdeglobals");
    KConfigGroup cg(&config, "KDE");
    QCOMPARE(cg.readEntry("widgetStyle", QString()), QString("testValue"));

    cg = KConfigGroup(&config, "General");
    //save() capitalizes the ColorScheme
    QCOMPARE(cg.readEntry("ColorScheme", QString()), QString("TestValue"));

    cg = KConfigGroup(&config, "Icons");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("testValue"));

    KConfig plasmaConfig("plasmarc");
    cg = KConfigGroup(&plasmaConfig, "Theme");
    QCOMPARE(cg.readEntry("name", QString()), QString("testValue"));

    KConfig inputConfig("kcminputrc");
    cg = KConfigGroup(&inputConfig, "Mouse");
    QCOMPARE(cg.readEntry("cursorTheme", QString()), QString("testValue"));

    KConfig splashConfig("ksplashrc");
    cg = KConfigGroup(&splashConfig, "KSplash");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("org.kde.test"));
    QCOMPARE(cg.readEntry("Engine", QString()), QString("KSplashQML"));

    KConfig lockerConfig("kscreenlockerrc");
    cg = KConfigGroup(&lockerConfig, "Greeter");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("org.kde.test"));

    KConfig kwinConfig("kwinrc");
    cg = KConfigGroup(&kwinConfig, "TabBox");
    QCOMPARE(cg.readEntry("LayoutName", QString()), QStringLiteral("testValue"));
    QCOMPARE(cg.readEntry("DesktopLayout", QString()), QStringLiteral("testDesktopValue"));
    QCOMPARE(cg.readEntry("DesktopListLayout", QString()), QStringLiteral("testDesktopValue"));
}

QTEST_MAIN(KcmTest)
#include "kcmtest.moc"
