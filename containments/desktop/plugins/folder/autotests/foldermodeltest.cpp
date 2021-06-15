/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "foldermodeltest.h"
#include "foldermodel.h"
#include "screenmapper.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

QTEST_MAIN(FolderModelTest)

static const QLatin1String desktop(QLatin1String("Desktop"));

static QUrl stringToUrl(const QString &path)
{
    return QUrl::fromUserInput(path, {}, QUrl::AssumeLocalFile);
}

void FolderModelTest::createTestFolder(const QString &path)
{
    QDir dir(m_folderDir->path());
    dir.mkdir(path);
    dir.cd(path);
    dir.mkdir(QStringLiteral("firstDir"));
    QFile f;
    for (int i = 1; i < 10; i++) {
        f.setFileName(QStringLiteral("%1/file%2.txt").arg(dir.path(), QString::number(i)));
        f.open(QFile::WriteOnly);
        f.close();
    }
}

void FolderModelTest::init()
{
    m_folderDir = new QTemporaryDir();
    createTestFolder(desktop);
    m_folderModel = new FolderModel(this);
    m_folderModel->classBegin();
    m_folderModel->setUrl(m_folderDir->path() + QDir::separator() + desktop);
    m_folderModel->componentComplete();
    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);
    s.wait(1000);
}

void FolderModelTest::cleanup()
{
    delete m_folderDir;
    m_folderDir = nullptr;
    delete m_folderModel;
    m_folderModel = nullptr;
}

void FolderModelTest::tst_listing()
{
    QCOMPARE(m_folderModel->url(), m_folderDir->path() + QDir::separator() + desktop);

    const auto count = m_folderModel->rowCount();
    QCOMPARE(count, 10);
    QCOMPARE(m_folderModel->index(0, 0).data(FolderModel::FileNameRole).toString(), QLatin1String("firstDir"));
    for (int i = 1; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        QCOMPARE(index.data(FolderModel::FileNameRole).toString(), QStringLiteral("file%1.txt").arg(i));
    }
}

void FolderModelTest::tst_listingDescending()
{
    m_folderModel->setSortDesc(true);
    QCOMPARE(m_folderModel->index(0, 0).data(FolderModel::FileNameRole).toString(), QLatin1String("firstDir"));
    const auto count = m_folderModel->rowCount();
    for (int i = 1; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        QCOMPARE(index.data(FolderModel::FileNameRole).toString(), QStringLiteral("file%1.txt").arg(count - i));
    }
}

void FolderModelTest::tst_listingFolderNotFirst()
{
    const auto count = m_folderModel->rowCount();
    m_folderModel->setSortDirsFirst(false);
    QCOMPARE(count, 10);
    QCOMPARE(m_folderModel->index(9, 0).data(FolderModel::FileNameRole).toString(), QLatin1String("firstDir"));
    for (int i = 0; i < count - 1; i++) {
        const auto index = m_folderModel->index(i, 0);
        QCOMPARE(index.data(FolderModel::FileNameRole).toString(), QStringLiteral("file%1.txt").arg(i + 1));
    }
}

void FolderModelTest::tst_filterListing()
{
    // a little bit weird API, as both pattern and mimetype needs to be set
    m_folderModel->setFilterPattern(QStringLiteral("*.txt"));
    m_folderModel->setFilterMimeTypes(QStringList{QStringLiteral("all/all")});
    m_folderModel->setFilterMode(FolderModel::FilterShowMatches);
    const auto count = m_folderModel->rowCount();
    QCOMPARE(count, 9);
    for (int i = 0; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        QCOMPARE(index.data(FolderModel::FileNameRole).toString(), QStringLiteral("file%1.txt").arg(i + 1));
    }
}

void FolderModelTest::tst_cd()
{
    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);

    // go into firstDir subfolder
    const auto url = m_folderModel->resolvedUrl();
    m_folderModel->cd(0);
    QVERIFY(s.wait(500));
    const auto url2 = m_folderModel->resolvedUrl();
    QVERIFY(url.isParentOf(url2));

    // go back to Desktop
    m_folderModel->up();
    QVERIFY(s.wait(500));
    QCOMPARE(m_folderModel->resolvedUrl(), url);

    // try to cd to an invalid entry (a file)
    m_folderModel->cd(1);
    // Signal is not emitted here as it's invalided
    QVERIFY(!s.wait(500));
    QCOMPARE(m_folderModel->resolvedUrl(), url);
}

void FolderModelTest::tst_rename_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("name");
    QTest::newRow("Folder rename") << 0 << "firstDirRenamed";
    QTest::newRow("File rename") << 1 << "file1.pdf";
    QTest::newRow("Invalid rename") << 11 << "foo";
}

void FolderModelTest::tst_rename()
{
    QFETCH(int, row);
    QFETCH(QString, name);
    m_folderModel->rename(row, name);
    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);
    const auto index = m_folderModel->index(row, 0);
    s.wait(500);
    QEXPECT_FAIL("Invalid rename", "This is expected to fail", Continue);
    QCOMPARE(index.data(FolderModel::FileNameRole).toString(), name);
}

void FolderModelTest::tst_selection()
{
    m_folderModel->setSelected(1);
    QVERIFY(m_folderModel->hasSelection());
    QVERIFY(m_folderModel->isSelected(1));

    m_folderModel->clearSelection();
    QVERIFY(!m_folderModel->hasSelection());

    m_folderModel->toggleSelected(1);
    QVERIFY(m_folderModel->isSelected(1));
    m_folderModel->toggleSelected(1);
    QVERIFY(!m_folderModel->isSelected(1));

    m_folderModel->setRangeSelected(1, 4);
    QVERIFY(m_folderModel->hasSelection());
    for (int i = 1; i <= 4; i++) {
        QVERIFY(m_folderModel->isSelected(i));
    }

    m_folderModel->updateSelection({5, 6}, false);
    for (int i = 1; i <= 4; i++) {
        QVERIFY(!m_folderModel->isSelected(i));
    }
    QVERIFY(m_folderModel->isSelected(5));
    QVERIFY(m_folderModel->isSelected(6));

    m_folderModel->setRangeSelected(1, 4);
    m_folderModel->pinSelection();
    m_folderModel->updateSelection({5, 6}, true);
    for (int i = 1; i <= 6; i++) {
        QVERIFY(m_folderModel->isSelected(i));
    }

    m_folderModel->unpinSelection();
    m_folderModel->updateSelection({5, 6}, true);
    for (int i = 1; i <= 6; i++) {
        if (i < 5) {
            QVERIFY(!m_folderModel->isSelected(i));
        } else {
            QVERIFY(m_folderModel->isSelected(i));
        }
    }
}

void FolderModelTest::tst_defaultValues()
{
    FolderModel folderModel;
    QCOMPARE(folderModel.status(), FolderModel::Status::None);
    QVERIFY(folderModel.locked());
    QVERIFY(!folderModel.sortDesc());
    QVERIFY(folderModel.sortDirsFirst());
    QVERIFY(!folderModel.parseDesktopFiles());
    QVERIFY(!folderModel.previews());
    QVERIFY(!folderModel.usedByContainment());
    QCOMPARE(folderModel.sortMode(), 0);
    QCOMPARE(folderModel.filterMode(), (int)FolderModel::NoFilter);
    QVERIFY(folderModel.newMenu());
    QCOMPARE(folderModel.filterCaseSensitivity(), Qt::CaseInsensitive);
    QVERIFY(folderModel.dynamicSortFilter());
    QVERIFY(folderModel.isSortLocaleAware());
}

void FolderModelTest::tst_actionMenu()
{
    const QStringList lst{QStringLiteral("open"),
                          QStringLiteral("cut"),
                          QStringLiteral("open"),
                          QStringLiteral("cut"),
                          QStringLiteral("undo"),
                          QStringLiteral("copy"),
                          QStringLiteral("paste"),
                          QStringLiteral("pasteto"),
                          QStringLiteral("refresh"),
                          QStringLiteral("rename"),
                          QStringLiteral("trash"),
                          QStringLiteral("del"),
                          QStringLiteral("restoreFromTrash"),
                          QStringLiteral("emptyTrash")};
    for (const QString &str : lst) {
        QVERIFY(m_folderModel->action(str));
    }
}

void FolderModelTest::tst_lockedChanged()
{
    QSignalSpy s(m_folderModel, &FolderModel::lockedChanged);
    m_folderModel->setLocked(false);
    QCOMPARE(s.count(), 1);
    m_folderModel->setLocked(false);
    QCOMPARE(s.count(), 1);
    m_folderModel->setLocked(true);
    QCOMPARE(s.count(), 2);
}

void FolderModelTest::tst_multiScreen()
{
    delete m_folderModel;
    // Custom instance for this test to set used by containment before marking component
    // as complete.
    m_folderModel = new FolderModel(this);
    m_folderModel->classBegin();
    m_folderModel->setUrl(m_folderDir->path() + QDir::separator() + desktop);
    m_folderModel->setUsedByContainment(true);
    m_folderModel->setScreen(0);
    m_folderModel->componentComplete();

    auto *screenMapper = ScreenMapper::instance();

    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);
    QVERIFY(s.wait(1000));
    const auto count = m_folderModel->rowCount();
    for (int i = 0; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        const auto name = index.data(FolderModel::UrlRole).toUrl();
        // all items are on the first screen by default
        QCOMPARE(screenMapper->screenForItem(name), 0);
    }

    // move one file to a new screen
    const auto movedItem = m_folderModel->index(0, 0).data(FolderModel::UrlRole).toUrl();
    FolderModel secondFolderModel;
    secondFolderModel.classBegin();
    secondFolderModel.setUrl(m_folderDir->path() + QDir::separator() + desktop);
    secondFolderModel.setUsedByContainment(true);
    secondFolderModel.setScreen(1);
    secondFolderModel.componentComplete();
    QSignalSpy s2(&secondFolderModel, &FolderModel::listingCompleted);
    QVERIFY(s2.wait(1000));
    const auto count2 = secondFolderModel.rowCount();
    QCOMPARE(count2, 0);

    screenMapper->addMapping(movedItem, 1);
    m_folderModel->invalidate();
    secondFolderModel.invalidate();
    s.wait(1000);
    s2.wait(1000);
    // we have one less item
    QCOMPARE(m_folderModel->rowCount(), count - 1);
    QCOMPARE(secondFolderModel.rowCount(), 1);
    QCOMPARE(secondFolderModel.index(0, 0).data(FolderModel::UrlRole).toUrl(), movedItem);
    QCOMPARE(screenMapper->screenForItem(movedItem), 1);

    // remove extra screen, we have all items back
    screenMapper->removeScreen(1, stringToUrl(m_folderModel->url()));
    s.wait(500);
    QCOMPARE(m_folderModel->rowCount(), count);
    QCOMPARE(secondFolderModel.rowCount(), 0);
    QCOMPARE(screenMapper->screenForItem(movedItem), 0);

    // add back extra screen, the item is moved there
    screenMapper->addScreen(1, stringToUrl(m_folderModel->url()));
    s.wait(500);
    s2.wait(500);
    QCOMPARE(m_folderModel->rowCount(), count - 1);
    QCOMPARE(secondFolderModel.rowCount(), 1);
    QCOMPARE(secondFolderModel.index(0, 0).data(FolderModel::UrlRole).toUrl(), movedItem);
    QCOMPARE(screenMapper->screenForItem(movedItem), 1);

    // create a new item, it appears on the first screen
    QDir dir(m_folderDir->path());
    dir.cd(desktop);
    dir.mkdir(QStringLiteral("secondDir"));
    dir.cd(QStringLiteral("secondDir"));
    s.wait(1000);
    QCOMPARE(m_folderModel->rowCount(), count);
    QCOMPARE(secondFolderModel.rowCount(), 1);
    QCOMPARE(screenMapper->screenForItem(stringToUrl(QLatin1String("file://") + dir.path())), 0);
}

void FolderModelTest::tst_multiScreenDifferenPath()
{
    m_folderModel->setUsedByContainment(true);
    m_folderModel->setScreen(0);
    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);
    s.wait(1000);
    const auto count = m_folderModel->rowCount();
    QCOMPARE(count, 10);

    const QLatin1String desktop2(QLatin1String("Desktop2"));
    createTestFolder(desktop2);
    FolderModel secondFolderModel;
    secondFolderModel.setUsedByContainment(true);
    secondFolderModel.setUrl(m_folderDir->path() + QDir::separator() + desktop2);
    secondFolderModel.setScreen(1);
    QSignalSpy s2(&secondFolderModel, &FolderModel::listingCompleted);
    s2.wait(1000);
    const auto count2 = secondFolderModel.rowCount();
    QCOMPARE(count2, 10);

    // create a new item, it appears on the first screen
    QDir dir(m_folderDir->path());
    dir.cd(desktop);
    dir.mkdir(QStringLiteral("secondDir"));
    s.wait(1000);
    QCOMPARE(m_folderModel->rowCount(), count + 1);
    QCOMPARE(secondFolderModel.rowCount(), count2);

    // create a new item, it appears on the second screen
    dir.cd(m_folderDir->path() + QDir::separator() + desktop2);
    dir.mkdir(QStringLiteral("secondDir2"));
    s.wait(1000);
    QCOMPARE(m_folderModel->rowCount(), count + 1);
    QCOMPARE(secondFolderModel.rowCount(), count2 + 1);
}
