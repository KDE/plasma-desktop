/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileCopyrightText: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "positionertest.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include "foldermodel.h"
#include "positioner.h"
#include "screenmapper.h"

QTEST_MAIN(PositionerTest)

static const QLatin1String desktop(QLatin1String("Desktop"));

void PositionerTest::initTestCase()
{
    m_folderDir = new QTemporaryDir();

    QDir dir(m_folderDir->path());
    dir.mkdir(desktop);
    dir.cd(desktop);
    dir.mkdir(QStringLiteral("firstDir"));
    QFile f;
    for (int i = 1; i < 10; i++) {
        f.setFileName(QStringLiteral("%1/file%2.txt").arg(dir.path(), QString::number(i)));
        f.open(QFile::WriteOnly);
        f.close();
    }
}

void PositionerTest::cleanupTestCase()
{
    delete m_folderDir;
}

void PositionerTest::init()
{
    m_folderModel = new FolderModel(this);
    m_folderModel->classBegin();
    m_folderModel->setScreen(0);
    m_folderModel->setUsedByContainment(true);
    m_folderModel->componentComplete();
    m_positioner = new Positioner(this);
    m_positioner->setEnabled(true);
    m_positioner->setFolderModel(m_folderModel);
    m_positioner->setPerStripe(3);

    m_folderModel->setUrl(m_folderDir->path() + QDir::separator() + desktop);
    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);
    s.wait(1000);
}

void PositionerTest::cleanup()
{
    delete m_folderModel;
    m_folderModel = nullptr;
    delete m_positioner;
    m_positioner = nullptr;
}

void PositionerTest::tst_positions_data()
{
    QTest::addColumn<int>("perStripe");
    QTest::newRow("3 per column") << 3;
    QTest::newRow("5 per column") << 5;
}

void PositionerTest::tst_positions()
{
    QFETCH(int, perStripe);
    m_positioner->setPerStripe(perStripe);
    checkPositions(perStripe);
}

void PositionerTest::tst_map()
{
    // by default the mapping is 1-1
    for (int i = 0; i < m_positioner->rowCount(); i++) {
        QCOMPARE(m_positioner->map(i), i);
    }
}

void PositionerTest::tst_move_data()
{
    QTest::addColumn<QVariantList>("moves");
    QTest::addColumn<QVector<int>>("result");
    QTest::newRow("First to last") << QVariantList({0, 10}) << QVector<int>({-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0});
    QTest::newRow("First to after last") << QVariantList({0, 11}) << QVector<int>({-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, 0});
    QTest::newRow("Switch 2nd with 3rd ") << QVariantList({1, 2, 2, 1}) << QVector<int>({0, 2, 1, 3, 4, 5, 6, 7, 8, 9});
    QTest::newRow("Switch 2nd with 2nd ") << QVariantList({1, 1, 1, 1}) << QVector<int>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
    QTest::newRow("2nd to last") << QVariantList({2, 10}) << QVector<int>({0, 1, -1, 3, 4, 5, 6, 7, 8, 9, 2});
}

void PositionerTest::tst_move()
{
    QFETCH(QVariantList, moves);
    QFETCH(QVector<int>, result);
    m_positioner->move(moves);
    for (int i = 0; i < m_positioner->rowCount(); i++) {
        QCOMPARE(m_positioner->map(i), result[i]);
    }
}

void PositionerTest::tst_nearestitem_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<QVector<int>>("result");
    QTest::newRow("Around first") << 0 << QVector<int>{-1, -1, 3, -1, 1};
    QTest::newRow("Around second") << 1 << QVector<int>{-1, -1, 4, 0, 2};
    QTest::newRow("Around 5th") << 4 << QVector<int>{-1, 1, 7, 3, 5};
    QTest::newRow("Around last") << 9 << QVector<int>{-1, 6, -1, -1, 7};
    QTest::newRow("Around invalid") << 11 << QVector<int>{-1, -1, -1, -1, -1};
}

void PositionerTest::tst_nearestitem()
{
    QFETCH(int, index);
    QFETCH(QVector<int>, result);
    for (int i = Qt::NoArrow; i <= Qt::RightArrow; i++) {
        QCOMPARE(m_positioner->nearestItem(index, (Qt::ArrowType)i), result[i]);
    }
}

void PositionerTest::tst_isBlank()
{
    QCOMPARE(m_positioner->isBlank(0), false);
    QCOMPARE(m_positioner->isBlank(11), true);
    m_positioner->move({0, 10});
    QCOMPARE(m_positioner->isBlank(0), true);
}

void PositionerTest::tst_reset()
{
    m_positioner->move({0, 10});
    m_positioner->reset();
    QSignalSpy s(m_positioner, &Positioner::positionsChanged);
    s.wait(500);
    QCOMPARE(s.count(), 1);
    checkPositions(3);

    for (int i = 0; i < m_positioner->rowCount(); i++) {
        QCOMPARE(m_positioner->map(i), i);
    }
}

void PositionerTest::tst_defaultValues()
{
    Positioner positioner;
    QVERIFY(!positioner.enabled());
    QVERIFY(!positioner.folderModel());
    QCOMPARE(positioner.perStripe(), 0);
    QVERIFY(positioner.positions().isEmpty());
}

void PositionerTest::tst_changeEnabledStatus()
{
    Positioner positioner;
    QVERIFY(!positioner.enabled());
    QSignalSpy s(&positioner, &Positioner::enabledChanged);
    positioner.setEnabled(true);
    QCOMPARE(s.count(), 1);

    positioner.setEnabled(false);
    QCOMPARE(s.count(), 2);

    // No change
    positioner.setEnabled(false);
    QCOMPARE(s.count(), 2);
}

void PositionerTest::tst_changePerStripe()
{
    Positioner positioner;
    QCOMPARE(positioner.perStripe(), 0);
    QSignalSpy s(&positioner, &Positioner::perStripeChanged);
    positioner.setPerStripe(1);
    QCOMPARE(s.count(), 1);

    // No change
    positioner.setPerStripe(1);
    QCOMPARE(s.count(), 1);

    positioner.setPerStripe(4);
    QCOMPARE(s.count(), 2);
}

void PositionerTest::tst_proxyMapping()
{
    auto *screenMapper = ScreenMapper::instance();
    FolderModel secondFolderModel;
    secondFolderModel.classBegin();
    secondFolderModel.setUrl(m_folderDir->path() + QDir::separator() + desktop);
    secondFolderModel.setUsedByContainment(true);
    secondFolderModel.setScreen(1);
    secondFolderModel.componentComplete();
    Positioner secondPositioner;
    secondPositioner.setEnabled(true);
    secondPositioner.setFolderModel(&secondFolderModel);
    secondPositioner.setPerStripe(3);

    QSignalSpy s2(&secondFolderModel, &FolderModel::listingCompleted);
    QVERIFY(s2.wait(1000));

    QHash<int, int> expectedSource2ProxyScreen0;
    QHash<int, int> expectedProxy2SourceScreen0;
    QHash<int, int> expectedProxy2SourceScreen1;
    QHash<int, int> expectedSource2ProxyScreen1;

    for (int i = 0; i < m_folderModel->rowCount(); i++) {
        expectedSource2ProxyScreen0[i] = i;
        expectedProxy2SourceScreen0[i] = i;
    }

    // swap items 1 and 2 in the positioner
    m_positioner->move({1, 2, 2, 1});
    expectedSource2ProxyScreen0[1] = 2;
    expectedSource2ProxyScreen0[2] = 1;
    expectedProxy2SourceScreen0[1] = 2;
    expectedProxy2SourceScreen0[2] = 1;

    auto savedSource2ProxyScreen0 = expectedSource2ProxyScreen0;
    auto savedProxy2SourceScreen0 = expectedProxy2SourceScreen0;

    auto verifyMapping = [](const QHash<int, int> &actual, const QHash<int, int> &expected) {
        auto ensureUnique = [](const QHash<int, int> mapping) {
            auto values = mapping.values();
            std::sort(values.begin(), values.end());
            auto uniqueValues = values.toSet().values();
            std::sort(uniqueValues.begin(), uniqueValues.end());
            QVERIFY(uniqueValues == values);
        };

        ensureUnique(actual);
        QCOMPARE(actual, expected);
    };

    verifyMapping(m_positioner->proxyToSourceMapping(), expectedProxy2SourceScreen0);
    verifyMapping(m_positioner->sourceToProxyMapping(), expectedSource2ProxyScreen0);
    verifyMapping(secondPositioner.proxyToSourceMapping(), expectedProxy2SourceScreen1);
    verifyMapping(secondPositioner.sourceToProxyMapping(), expectedSource2ProxyScreen1);

    const auto movedItem = m_folderModel->index(1, 0).data(FolderModel::UrlRole).toUrl();

    // move the item 1 from source (now in position 2) to the second screen
    screenMapper->addMapping(movedItem, 1);

    expectedProxy2SourceScreen1[0] = 0;
    expectedSource2ProxyScreen1[0] = 0;
    expectedSource2ProxyScreen0.clear();
    expectedProxy2SourceScreen0.clear();
    for (int i = 0; i < m_folderModel->rowCount(); i++) {
        // as item 1 disappeared, the mapping of all items after that are shifted
        auto proxyIndex = (i <= 1) ? i : i + 1;
        expectedProxy2SourceScreen0[proxyIndex] = i;
        expectedSource2ProxyScreen0[i] = proxyIndex;
    }

    verifyMapping(m_positioner->proxyToSourceMapping(), expectedProxy2SourceScreen0);
    verifyMapping(m_positioner->sourceToProxyMapping(), expectedSource2ProxyScreen0);
    verifyMapping(secondPositioner.proxyToSourceMapping(), expectedProxy2SourceScreen1);
    verifyMapping(secondPositioner.sourceToProxyMapping(), expectedSource2ProxyScreen1);

    // move back the same item to the first screen
    screenMapper->addMapping(movedItem, 0);

    // nothing on the second screen
    expectedSource2ProxyScreen1.clear();
    expectedProxy2SourceScreen1.clear();
    // first screen should look like in the beginning
    expectedSource2ProxyScreen0 = savedSource2ProxyScreen0;
    expectedProxy2SourceScreen0 = savedProxy2SourceScreen0;

    verifyMapping(m_positioner->proxyToSourceMapping(), expectedProxy2SourceScreen0);
    verifyMapping(m_positioner->sourceToProxyMapping(), expectedSource2ProxyScreen0);
    verifyMapping(secondPositioner.proxyToSourceMapping(), expectedProxy2SourceScreen1);
    verifyMapping(secondPositioner.sourceToProxyMapping(), expectedSource2ProxyScreen1);
}

void PositionerTest::checkPositions(int perStripe)
{
    QSignalSpy s(m_positioner, &Positioner::positionsChanged);
    s.wait(500);

    const auto positions = m_positioner->positions();
    struct Pos {
        int x;
        int y;
    };
    const auto fileCount = m_folderModel->rowCount();
    QHash<QString, Pos> posHash;
    QCOMPARE(positions[0].toInt(), 1 + ((fileCount - 1) / perStripe)); // rows
    QCOMPARE(positions[1].toInt(), perStripe); // columns
    for (int i = 2; i < positions.length() - 2; i += 3) {
        posHash[positions[i]] = {positions[i + 1].toInt(), positions[i + 2].toInt()};
    }

    int row = 0;
    int col = 0;
    for (int i = 0; i < fileCount; i++) {
        const auto index = m_folderModel->index(i, 0);
        const auto url = index.data(FolderModel::UrlRole).toString();
        const Pos pos = posHash[url];
        QCOMPARE(pos.x, row);
        QCOMPARE(pos.y, col);
        col++;
        if (col == perStripe) {
            row++;
            col = 0;
        }
    }
}
