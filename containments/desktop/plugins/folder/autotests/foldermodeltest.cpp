/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "foldermodeltest.h"
#include "foldermodel.h"
#include "screenmapper.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QMimeData>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QStandardPaths>
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
    for (int i = 1; i < 15; i++) {
        if (i < 10) {
            f.setFileName(QStringLiteral("%1/file%2.txt").arg(dir.path(), QString::number(i)));
        } else {
            f.setFileName(QStringLiteral("%1/.file%2.txt").arg(dir.path(), QString::number(i - 9)));
        }

        f.open(QFile::WriteOnly);
        f.close();
    }
}

void FolderModelTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    KConfigGroup confirmGroup(KSharedConfig::openConfig(QStringLiteral("kiorc"), KConfig::NoGlobals), QStringLiteral("Confirmations"));
    confirmGroup.writeEntry("ConfirmRenameFileType", false);
    confirmGroup.writeEntry("ConfirmHide", false);
    confirmGroup.sync();
}

void FolderModelTest::init()
{
    m_currentActivity = QStringLiteral("00000000-0000-0000-0000-000000000000");
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

void FolderModelTest::tst_listingHidden()
{
    m_folderModel->setShowHiddenFiles(true);
    const auto count = m_folderModel->rowCount();
    QCOMPARE(count, 15);
    QCOMPARE(m_folderModel->index(0, 0).data(FolderModel::FileNameRole).toString(), QLatin1String("firstDir"));
    for (int i = 1; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        QString fileName = index.data(FolderModel::FileNameRole).toString();
        // hidden files are listed first
        if (i <= 5) {
            QCOMPARE(fileName, QStringLiteral(".file%1.txt").arg(i));
        } else {
            QCOMPARE(fileName, QStringLiteral("file%1.txt").arg(i - 5));
        }
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
    // The first signal might come from KCoreDirListerCache::emitItemsFromCache
    QVERIFY(s.wait(500));
    // The second signal comes from KCoreDirListerCache::slotUpdateResult
    s.wait(500);
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
        QCOMPARE(screenMapper->screenForItem(name, m_currentActivity), 0);
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

    screenMapper->addMapping(movedItem, 1, m_currentActivity);
    m_folderModel->invalidate();
    secondFolderModel.invalidate();
    s.wait(1000);
    s2.wait(1000);
    // we have one less item
    QCOMPARE(m_folderModel->rowCount(), count - 1);
    QCOMPARE(secondFolderModel.rowCount(), 1);
    QCOMPARE(secondFolderModel.index(0, 0).data(FolderModel::UrlRole).toUrl(), movedItem);
    QCOMPARE(screenMapper->screenForItem(movedItem, m_currentActivity), 1);

    // remove extra screen, we have all items back
    screenMapper->removeScreen(1, m_currentActivity, stringToUrl(m_folderModel->url()));
    s.wait(500);
    QCOMPARE(m_folderModel->rowCount(), count);
    QCOMPARE(secondFolderModel.rowCount(), 0);
    QCOMPARE(screenMapper->screenForItem(movedItem, m_currentActivity), 0);

    // add back extra screen, the item is moved there
    screenMapper->addScreen(1, m_currentActivity, stringToUrl(m_folderModel->url()));
    s.wait(500);
    s2.wait(500);
    QCOMPARE(m_folderModel->rowCount(), count - 1);
    QCOMPARE(secondFolderModel.rowCount(), 1);
    QCOMPARE(secondFolderModel.index(0, 0).data(FolderModel::UrlRole).toUrl(), movedItem);
    QCOMPARE(screenMapper->screenForItem(movedItem, m_currentActivity), 1);

    // create a new item, it appears on the first screen
    QDir dir(m_folderDir->path());
    dir.cd(desktop);
    dir.mkdir(QStringLiteral("secondDir"));
    dir.cd(QStringLiteral("secondDir"));
    s.wait(1000);
    QCOMPARE(m_folderModel->rowCount(), count);
    QCOMPARE(secondFolderModel.rowCount(), 1);
    QCOMPARE(screenMapper->screenForItem(stringToUrl(QLatin1String("file://") + dir.path()), m_currentActivity), 0);
}

void FolderModelTest::tst_replacementModel()
{
    m_folderModel->setUsedByContainment(true);
    m_folderModel->setScreen(0);
    m_folderModel->invalidate();
    QTRY_VERIFY(m_folderModel->rowCount() > 0);
    const auto count = m_folderModel->rowCount();

    FolderModel replacement;
    replacement.classBegin();
    replacement.setUrl(m_folderModel->url());
    replacement.setUsedByContainment(true);
    replacement.setScreen(0);
    replacement.componentComplete();
    QTRY_COMPARE(replacement.rowCount(), count);

    delete m_folderModel;
    m_folderModel = nullptr;
    replacement.invalidate();
    QCOMPARE(replacement.rowCount(), count);
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

void FolderModelTest::tst_initializeOnCorrectScreens()
{
    createTestFolder(desktop);
    const QString path = m_folderDir->path() + QDir::separator() + desktop;

    // Create screen_0
    auto folderFirstScreen = new FolderModel(this);
    folderFirstScreen->classBegin();
    folderFirstScreen->setUrl(path);
    folderFirstScreen->setUsedByContainment(true);
    folderFirstScreen->setScreen(0);
    folderFirstScreen->componentComplete();
    QSignalSpy s1(folderFirstScreen, &FolderModel::listingCompleted);
    QVERIFY(s1.wait(1000));

    // Create a mapping where one file is already on disabled screen_1 for testing purposes
    auto *screenMapper = ScreenMapper::instance();
    auto newMap = screenMapper->screenMapping();
    newMap[1] = QString::number(1);

    // The item on disabled screen_1 should move to enabled screen_0
    QSignalSpy screenMapSpy(screenMapper, &ScreenMapper::screenMappingChanged);
    screenMapper->setScreenMapping(newMap);
    QCOMPARE(screenMapSpy.count(), 1);
    QCOMPARE(screenMapper->screenMapping().at(1), QStringLiteral("0"));

    // Create screen_1
    auto folderSecondScreen = new FolderModel(this);
    folderSecondScreen->classBegin();
    folderSecondScreen->setUrl(path);
    folderSecondScreen->setUsedByContainment(true);
    folderSecondScreen->setScreen(1);
    folderSecondScreen->componentComplete();
    QSignalSpy s2(folderSecondScreen, &FolderModel::listingCompleted);
    QVERIFY(s2.wait(1000));

    // Now that the screen_1 is enabled again, the item should be back on the screen_1
    QCOMPARE(screenMapper->screenMapping(), newMap);
}

void FolderModelTest::tst_mapDropUrl_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow("plain") << QStringLiteral("file.txt");
    QTest::newRow("space") << QStringLiteral("a file.txt");
    QTest::newRow("question mark") << QStringLiteral("show_bug.cgi?id=507060.desktop");
    QTest::newRow("percent") << QStringLiteral("100%.txt");
    QTest::newRow("hash") << QStringLiteral("a#b.txt");
    QTest::newRow("link from url") << QString::fromUtf8("https:⁄⁄example.org⁄page?x=1#top.desktop");
}

void FolderModelTest::tst_mapDropUrl()
{
    QFETCH(QString, fileName);

    const QUrl localDirUrl = QUrl::fromLocalFile(QStringLiteral("/home/user/Desktop"));
    const QUrl url = QUrl::fromLocalFile(QStringLiteral("/home/user/Desktop/") + fileName);

    QUrl expected(QStringLiteral("desktop:/"));
    expected.setPath(QLatin1Char('/') + fileName);
    QCOMPARE(FolderModel::mapDropUrl(url, localDirUrl, QUrl(QStringLiteral("desktop:/"))), expected);

    QUrl expectedSubDir(QStringLiteral("desktop:/"));
    expectedSubDir.setPath(QStringLiteral("/sub/") + fileName);
    const QUrl subDirUrl = QUrl::fromLocalFile(QStringLiteral("/home/user/Desktop/sub/") + fileName);
    QCOMPARE(FolderModel::mapDropUrl(subDirUrl, QUrl::fromLocalFile(QStringLiteral("/home/user/Desktop/sub")), QUrl(QStringLiteral("desktop:/sub"))),
             expectedSubDir);

    // Same scheme, or not below the local folder: unchanged
    QCOMPARE(FolderModel::mapDropUrl(url, localDirUrl, localDirUrl), url);
    const QUrl elsewhere = QUrl::fromLocalFile(QStringLiteral("/home/user/Desktop2/") + fileName);
    QCOMPARE(FolderModel::mapDropUrl(elsewhere, localDirUrl, QUrl(QStringLiteral("desktop:/"))), elsewhere);
}

void FolderModelTest::tst_dropOnOtherScreen()
{
    const QStringList fileNames{
        QStringLiteral("file.txt"),
        QStringLiteral("a file.txt"),
        QStringLiteral("show_bug.cgi?id=507060.desktop"),
        QStringLiteral("100%.txt"),
        QStringLiteral("a#b.txt"),
        QString::fromUtf8("https:⁄⁄example.org⁄page?x=1#top.desktop"),
    };

    // Earlier tests leave screens registered in the shared mapper
    ScreenMapper::instance()->cleanup();

    // The desktop:/ worker lists $HOME/Desktop in its own process
    QTemporaryDir home;
    const QByteArray oldHome = qgetenv("HOME");
    const QByteArray oldConfigHome = qgetenv("XDG_CONFIG_HOME");
    qputenv("HOME", QFile::encodeName(home.path()));
    qputenv("XDG_CONFIG_HOME", QFile::encodeName(home.path() + QStringLiteral("/.config")));
    auto restoreEnv = qScopeGuard([&] {
        qputenv("HOME", oldHome);
        qputenv("XDG_CONFIG_HOME", oldConfigHome);
    });
    const QString desktopPath = home.path() + QStringLiteral("/Desktop/");
    QDir(home.path()).mkpath(QStringLiteral("Desktop"));
    for (const QString &fileName : fileNames) {
        QFile file(desktopPath + fileName);
        QVERIFY(file.open(QFile::WriteOnly));
    }

    auto createModel = [](FolderModel &model, int screen) {
        model.classBegin();
        model.setUrl(QStringLiteral("desktop:/"));
        model.setUsedByContainment(true);
        model.setScreen(screen);
        model.componentComplete();
    };
    FolderModel firstScreen;
    QSignalSpy firstListed(&firstScreen, &FolderModel::listingCompleted);
    createModel(firstScreen, 0);
    QVERIFY(firstListed.wait(5000));
    FolderModel secondScreen;
    QSignalSpy secondListed(&secondScreen, &FolderModel::listingCompleted);
    createModel(secondScreen, 1);
    QVERIFY(secondListed.wait(5000));
    QCOMPARE(firstScreen.rowCount(), fileNames.size());
    QCOMPARE(secondScreen.rowCount(), 0);

    // Drag and drop pass local file urls, the models list desktop:/ urls
    for (const QString &fileName : fileNames) {
        QUrl itemUrl(QStringLiteral("desktop:/"));
        itemUrl.setPath(QLatin1Char('/') + fileName);

        QMimeData mimeData;
        mimeData.setUrls({QUrl::fromLocalFile(desktopPath + fileName)});
        QObject dropEvent;
        dropEvent.setProperty("mimeData", QVariant::fromValue<QObject *>(&mimeData));
        dropEvent.setProperty("x", 10);
        dropEvent.setProperty("y", 10);
        secondScreen.drop(nullptr, &dropEvent, -1);

        QVERIFY2(ScreenMapper::instance()->screenForItem(itemUrl, m_currentActivity) == 1, qPrintable(fileName));
    }

    firstScreen.invalidate();
    secondScreen.invalidate();
    QTRY_COMPARE(firstScreen.rowCount(), 0);
    QTRY_COMPARE(secondScreen.rowCount(), fileNames.size());
}

#include "moc_foldermodeltest.cpp"
