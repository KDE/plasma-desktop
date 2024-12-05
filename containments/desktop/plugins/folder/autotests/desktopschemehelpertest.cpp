/*
    SPDX-FileCopyrightText: 2022 Alexander Wilms <f.alexander.wilms@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "desktopschemehelpertest.h"
#include "desktopschemehelper.h"

#include <QStandardPaths>
#include <QTest>
#include <QUrl>

QTEST_MAIN(DesktopSchemeHelperTest)

void DesktopSchemeHelperTest::returnsExpectedValues()
{
    DesktopSchemeHelper h;
    QString desktopWithoutTrailingSlash = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0);
    QString desktopWithTrailingSlash = desktopWithoutTrailingSlash + QStringLiteral("/");
    QString childOfDesktop = desktopWithTrailingSlash + QStringLiteral("Productivity/");
    QString parentOfDesktop = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0) + QStringLiteral("/");
    QString remotePath(QStringLiteral("smb://192.168.001.002"));

    // file: -> desktop: if possible
    QCOMPARE(h.getDesktopUrl(childOfDesktop), QStringLiteral("desktop:/Productivity/"));
    QCOMPARE(h.getDesktopUrl(desktopWithoutTrailingSlash), QStringLiteral("desktop:/"));
    QCOMPARE(h.getDesktopUrl(desktopWithTrailingSlash), QStringLiteral("desktop:/"));
    QCOMPARE(h.getDesktopUrl(parentOfDesktop), parentOfDesktop);
    QCOMPARE(h.getDesktopUrl(remotePath), remotePath);

    // desktop: -> file: if possible
    QCOMPARE(h.getFileUrl(QStringLiteral("desktop:/Productivity/")), childOfDesktop);
    QCOMPARE(h.getFileUrl(QStringLiteral("desktop:/Productivity/.")), childOfDesktop);
    QCOMPARE(h.getFileUrl(QStringLiteral("desktop:/")), desktopWithTrailingSlash);
    QCOMPARE(h.getFileUrl(QStringLiteral("desktop:/.")), desktopWithTrailingSlash);
    QCOMPARE(h.getFileUrl(QStringLiteral("desktop://")), desktopWithTrailingSlash + QStringLiteral("/"));
    QCOMPARE(h.getFileUrl(parentOfDesktop), parentOfDesktop);
    QCOMPARE(h.getFileUrl(remotePath), remotePath);

    // test that methods are static

    // file: -> desktop: if possible
    QCOMPARE(DesktopSchemeHelper::getDesktopUrl(childOfDesktop), QStringLiteral("desktop:/Productivity/"));
    QCOMPARE(DesktopSchemeHelper::getDesktopUrl(desktopWithoutTrailingSlash), QStringLiteral("desktop:/"));
    QCOMPARE(DesktopSchemeHelper::getDesktopUrl(desktopWithTrailingSlash), QStringLiteral("desktop:/"));
    QCOMPARE(DesktopSchemeHelper::getDesktopUrl(parentOfDesktop), parentOfDesktop);
    QCOMPARE(DesktopSchemeHelper::getDesktopUrl(remotePath), remotePath);

    // desktop: -> file: if possible
    QCOMPARE(DesktopSchemeHelper::getFileUrl(QStringLiteral("desktop:/Productivity/")), childOfDesktop);
    QCOMPARE(DesktopSchemeHelper::getFileUrl(QStringLiteral("desktop:/Productivity/.")), childOfDesktop);
    QCOMPARE(DesktopSchemeHelper::getFileUrl(QStringLiteral("desktop:/")), desktopWithTrailingSlash);
    QCOMPARE(DesktopSchemeHelper::getFileUrl(QStringLiteral("desktop:/.")), desktopWithTrailingSlash);
    QCOMPARE(DesktopSchemeHelper::getFileUrl(QStringLiteral("desktop://")), desktopWithTrailingSlash + QStringLiteral("/"));
    QCOMPARE(DesktopSchemeHelper::getFileUrl(parentOfDesktop), parentOfDesktop);
    QCOMPARE(DesktopSchemeHelper::getFileUrl(remotePath), remotePath);

    // LabelGenerator expects a leading slash
    QCOMPARE(DesktopSchemeHelper::getFileUrl(QStringLiteral("desktop:Productivity/")), childOfDesktop);
}

#include "moc_desktopschemehelpertest.cpp"
