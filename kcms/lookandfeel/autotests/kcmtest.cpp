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
#include "../duplicate/shellpluginloader.h"
// Qt
#include <QtTest/QtTest>
#include <Plasma/Package>
#include <Plasma/PluginLoader>
#include <ksycoca.h>

class KcmTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testWidgetStyle();
    void testKCMSave();

private:
    QDir m_configDir;
    KCMLookandFeel *m_KCMLookandFeel;
};


void KcmTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    ShellPluginLoader::init();

    m_configDir = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    m_configDir.removeRecursively();

    QVERIFY(m_configDir.mkpath("."));

    const QString packagePath = QFINDTESTDATA("lookandfeel");
    Plasma::Package p = Plasma::PluginLoader::self()->loadPackage("Plasma/LookAndFeel");
    p.setPath(packagePath);
    QVERIFY(p.isValid());
    p.install(packagePath, QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/plasma/look-and-feel/");

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
}


void KcmTest::testWidgetStyle()
{
    m_KCMLookandFeel->setWidgetStyle("customTestValue");

    KConfig config("kdeglobals");
    KConfigGroup cg(&config, "KDE");
    QCOMPARE(cg.readEntry("widgetStyle", QString()), QString("customTestValue"));
}


void KcmTest::testKCMSave()
{
    m_KCMLookandFeel->save();

    KConfig config("kdeglobals");
    KConfigGroup cg(&config, "KDE");
    QCOMPARE(cg.readEntry("widgetStyle", QString()), QString("testValue"));
    QCOMPARE(cg.readEntry("ColorScheme", QString()), QString("TestValue"));

    cg = KConfigGroup(&config, "Icons");
    QCOMPARE(cg.readEntry("Theme", QString()), QString("testValue"));

    KConfig plasmaConfig("plasmarc");
    cg = KConfigGroup(&plasmaConfig, "Theme");
    QCOMPARE(cg.readEntry("name", QString()), QString("testValue"));

    KConfig inputConfig("kcminputrc");
    cg = KConfigGroup(&inputConfig, "Mouse");
    QCOMPARE(cg.readEntry("cursorTheme", QString()), QString("testValue"));
}

QTEST_MAIN(KcmTest)
#include "kcmtest.moc"
