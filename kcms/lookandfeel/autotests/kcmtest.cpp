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
#include <QtTest>
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KSycoca>
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

    //we need an existing colorscheme file, even if empty
    QVERIFY(m_dataDir.mkpath(QStringLiteral("color-schemes")));
    QFile f(m_dataDir.path() + QStringLiteral("/color-schemes/TestValue.colors"));
    f.open(QIODevice::WriteOnly);
    f.close();

    const QString packagePath = QFINDTESTDATA("lookandfeel");

    KPackage::Package p = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
    p.setPath(packagePath);
    QVERIFY(p.isValid());

    const QString packageRoot = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/plasma/look-and-feel/";
    auto installJob = p.install(packagePath, packageRoot);
    installJob->exec();

    KConfig config(QStringLiteral("kdeglobals"));
    KConfigGroup cg(&config, "KDE");
    cg.writeEntry("LookAndFeelPackage", "org.kde.test");
    cg.sync();
    m_KCMLookandFeel = new KCMLookandFeel(nullptr, QVariantList());
    m_KCMLookandFeel->load();
}

void KcmTest::cleanupTestCase()
{
    m_configDir.removeRecursively();
    m_dataDir.removeRecursively();
}


void KcmTest::testWidgetStyle()
{
    m_KCMLookandFeel->setWidgetStyle(QStringLiteral("customTestValue"));

    KConfig config(QStringLiteral("kdeglobals"));
    KConfigGroup cg(&config, "KDE");
    QCOMPARE(cg.readEntry("widgetStyle", QString()), QString("customTestValue"));
}

void KcmTest::testColors()
{
    //TODO: test colorFile as well
    m_KCMLookandFeel->setColors(QStringLiteral("customTestValue"), QString());

    KConfig config(QStringLiteral("kdeglobals"));
    KConfigGroup cg(&config, "General");
    QCOMPARE(cg.readEntry("ColorScheme", QString()), QString("customTestValue"));
}

void KcmTest::testIcons()
{
    m_KCMLookandFeel->setIcons(QStringLiteral("customTestValue"));

    KConfig config(QStringLiteral("kdeglobals"));
    KConfigGroup cg(&config, "Icons");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("customTestValue"));
}

void KcmTest::testPlasmaTheme()
{
    m_KCMLookandFeel->setPlasmaTheme(QStringLiteral("customTestValue"));

    KConfig config(QStringLiteral("plasmarc"));
    KConfigGroup cg(&config, "Theme");
    QCOMPARE(cg.readEntry("name", QString()), QString("customTestValue"));
}

void KcmTest::testCursorTheme()
{
    m_KCMLookandFeel->setCursorTheme(QStringLiteral("customTestValue"));

    KConfig config(QStringLiteral("kcminputrc"));
    KConfigGroup cg(&config, "Mouse");
    QCOMPARE(cg.readEntry("cursorTheme", QString()), QString("customTestValue"));
}

void KcmTest::testSplashScreen()
{
    m_KCMLookandFeel->setSplashScreen(QStringLiteral("customTestValue"));

    KConfig config(QStringLiteral("ksplashrc"));
    KConfigGroup cg(&config, "KSplash");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("customTestValue"));
    QCOMPARE(cg.readEntry("Engine", QString()), QString("KSplashQML"));
}

void KcmTest::testLockScreen()
{
    m_KCMLookandFeel->setLockScreen(QStringLiteral("customTestValue"));

    KConfig config(QStringLiteral("kscreenlockerrc"));
    KConfigGroup cg(&config, "Greeter");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("customTestValue"));
}

void KcmTest::testWindowSwitcher()
{
    m_KCMLookandFeel->setWindowSwitcher(QStringLiteral("customTestValue"));

    KConfig config(QStringLiteral("kwinrc"));
    KConfigGroup cg(&config, "TabBox");
    QCOMPARE(cg.readEntry("LayoutName", QString()), QStringLiteral("customTestValue"));
}

void KcmTest::testDesktopSwitcher()
{
    m_KCMLookandFeel->setDesktopSwitcher(QStringLiteral("customTestValue"));

    KConfig config(QStringLiteral("kwinrc"));
    KConfigGroup cg(&config, "TabBox");
    QCOMPARE(cg.readEntry("DesktopLayout", QString()), QStringLiteral("customTestValue"));
    QCOMPARE(cg.readEntry("DesktopListLayout", QString()), QStringLiteral("customTestValue"));
}

void KcmTest::testKCMSave()
{
    m_KCMLookandFeel->save();

    KConfig config(QStringLiteral("kdeglobals"));
    KConfigGroup cg(&config, "KDE");
    QCOMPARE(cg.readEntry("widgetStyle", QString()), QString("testValue"));

    cg = KConfigGroup(&config, "General");
    //save() capitalizes the ColorScheme
    QCOMPARE(cg.readEntry("ColorScheme", QString()), QString("TestValue"));

    cg = KConfigGroup(&config, "Icons");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("testValue"));

    KConfig plasmaConfig(QStringLiteral("plasmarc"));
    cg = KConfigGroup(&plasmaConfig, "Theme");
    QCOMPARE(cg.readEntry("name", QString()), QString("testValue"));

    KConfig inputConfig(QStringLiteral("kcminputrc"));
    cg = KConfigGroup(&inputConfig, "Mouse");
    QCOMPARE(cg.readEntry("cursorTheme", QString()), QString("testValue"));

    KConfig splashConfig(QStringLiteral("ksplashrc"));
    cg = KConfigGroup(&splashConfig, "KSplash");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("org.kde.test"));
    QCOMPARE(cg.readEntry("Engine", QString()), QString("KSplashQML"));

    KConfig lockerConfig(QStringLiteral("kscreenlockerrc"));
    cg = KConfigGroup(&lockerConfig, "Greeter");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("org.kde.test"));

    KConfig kwinConfig(QStringLiteral("kwinrc"));
    cg = KConfigGroup(&kwinConfig, "TabBox");
    QCOMPARE(cg.readEntry("LayoutName", QString()), QStringLiteral("testValue"));
    QCOMPARE(cg.readEntry("DesktopLayout", QString()), QStringLiteral("testDesktopValue"));
    QCOMPARE(cg.readEntry("DesktopListLayout", QString()), QStringLiteral("testDesktopValue"));
}

QTEST_MAIN(KcmTest)
#include "kcmtest.moc"
