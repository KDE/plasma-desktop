/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knetattach.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTest>

class KNetAttachTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testAFullAddressFillsTheWebFolderFields_data();
    void testAFullAddressFillsTheWebFolderFields();
    void testTheWebFolderPortFollowsEncryption();

private:
    template<typename T>
    static T *field(const KNetAttach &wizard, const char *name)
    {
        return wizard.findChild<T *>(QLatin1String(name));
    }
    static void openWebFolderPage(KNetAttach &wizard);
};

void KNetAttachTest::initTestCase()
{
    // The wizard lists the connections saved in krecentconnections.
    QStandardPaths::setTestModeEnabled(true);
}

void KNetAttachTest::openWebFolderPage(KNetAttach &wizard)
{
    // The wizard starts on its first page once it is shown.
    wizard.show();
    field<QRadioButton>(wizard, "_webfolder")->setChecked(true);
    wizard.next();
}

void KNetAttachTest::testAFullAddressFillsTheWebFolderFields_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QString>("host");
    QTest::addColumn<int>("port");
    QTest::addColumn<bool>("encrypted");
    QTest::addColumn<QString>("path");

    QTest::newRow("webdavs with a port") << QStringLiteral("webdavs://dav.example.org:8443/folder/") << QStringLiteral("dav.example.org") << 8443 << true
                                         << QStringLiteral("/folder/");
    QTest::newRow("https without a port") << QStringLiteral("https://cloud.example.org/remote.php/dav/files/user/") << QStringLiteral("cloud.example.org")
                                          << 443 << true << QStringLiteral("/remote.php/dav/files/user/");
    QTest::newRow("webdav") << QStringLiteral("webdav://nas.local/share") << QStringLiteral("nas.local") << 80 << false << QStringLiteral("/share");
    QTest::newRow("host name only") << QStringLiteral("nas.local") << QStringLiteral("nas.local") << 80 << false << QString();
}

void KNetAttachTest::testAFullAddressFillsTheWebFolderFields()
{
    QFETCH(QString, address);
    QFETCH(QString, host);
    QFETCH(int, port);
    QFETCH(bool, encrypted);
    QFETCH(QString, path);

    KNetAttach wizard;
    openWebFolderPage(wizard);

    auto hostField = field<QLineEdit>(wizard, "_host");
    hostField->setText(address);
    Q_EMIT hostField->editingFinished();

    QCOMPARE(hostField->text(), host);
    QCOMPARE(field<QSpinBox>(wizard, "_port")->value(), port);
    QCOMPARE(field<QCheckBox>(wizard, "_useEncryption")->isChecked(), encrypted);
    QCOMPARE(field<QLineEdit>(wizard, "_path")->text(), path);
}

void KNetAttachTest::testTheWebFolderPortFollowsEncryption()
{
    KNetAttach wizard;
    openWebFolderPage(wizard);
    auto port = field<QSpinBox>(wizard, "_port");
    QCOMPARE(port->value(), 80);

    field<QCheckBox>(wizard, "_useEncryption")->setChecked(true);
    QCOMPARE(port->value(), 443);

    // Coming back to the parameters page keeps the port of the encrypted connection.
    wizard.back();
    wizard.next();
    QCOMPARE(port->value(), 443);
}

QTEST_MAIN(KNetAttachTest)

#include "knetattachtest.moc"
