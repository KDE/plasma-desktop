/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knetattach.h"

#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTest>

class KNetAttachTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
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
