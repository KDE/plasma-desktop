/***************************************************************************
 *   Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company *
 *                      <info@kdab.com>                                    *
 *   Author: Andras Mantia <andras.mantia@kdab.com>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "foldermodeltest.h"
#include "foldermodel.h"

#include <QTest>
#include <QTemporaryDir>
#include <QSignalSpy>

QTEST_MAIN(FolderModelTest)

static const QLatin1String desktop(QLatin1String("Desktop"));

void FolderModelTest::initTestCase()
{
    m_folderDir = new QTemporaryDir();
        
    QDir dir(m_folderDir->path());
    dir.mkdir(desktop);
    dir.cd(desktop);
    dir.mkdir("firstDir");
    QFile f;
    for (int i = 1; i < 10; i++) {
        f.setFileName(QStringLiteral("%1/file%2.txt").arg(dir.path(), QString::number(i)));
        f.open(QFile::WriteOnly);
        f.close();
    }
    
}

void FolderModelTest::cleanupTestCase()
{
    delete m_folderDir;
}

void FolderModelTest::init()
{
    m_folderModel = new FolderModel(this);
    m_folderModel->setUrl(m_folderDir->path()  + QDir::separator() + desktop );
    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);
    s.wait(1000);
}

void FolderModelTest::cleanup()
{
    delete m_folderModel;
    m_folderModel = nullptr;
}

void FolderModelTest::tst_listing()
{
    
    QCOMPARE(m_folderModel->url(), m_folderDir->path() + QDir::separator() + desktop);

    const auto count = m_folderModel->rowCount();
    QCOMPARE(count, 10);
    QCOMPARE(m_folderModel->index(0, 0).data(FolderModel::FileNameRole).toString(),
             QLatin1String("firstDir"));
    for (int i = 1; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        QCOMPARE(index.data(FolderModel::FileNameRole).toString(),
                 QStringLiteral("file%1.txt").arg(i));
    }
}

void FolderModelTest::tst_listingDescending()
{
    m_folderModel->setSortDesc(true);
    QCOMPARE(m_folderModel->index(0, 0).data(FolderModel::FileNameRole).toString(),
             QLatin1String("firstDir"));
    const auto count = m_folderModel->rowCount();
    for (int i = 1; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        QCOMPARE(index.data(FolderModel::FileNameRole).toString(),
                 QStringLiteral("file%1.txt").arg(count - i));
    }
}

void FolderModelTest::tst_listingFolderNotFirst()
{
    const auto count = m_folderModel->rowCount();
    m_folderModel->setSortDirsFirst(false);
    QCOMPARE(count, 10);
    QCOMPARE(m_folderModel->index(9, 0).data(FolderModel::FileNameRole).toString(),
             QLatin1String("firstDir"));
    for (int i = 0; i < count - 1; i++) {
        const auto index = m_folderModel->index(i, 0);
        QCOMPARE(index.data(FolderModel::FileNameRole).toString(),
                 QStringLiteral("file%1.txt").arg(i + 1));
    }
}

void FolderModelTest::tst_filterListing()
{
    // a little bit weird API, as both pattern and mimetype needs to be set
    m_folderModel->setFilterPattern("*.txt");
    m_folderModel->setFilterMimeTypes({"all/all"});
    m_folderModel->setFilterMode(FolderModel::FilterShowMatches);
    const auto count = m_folderModel->rowCount();
    QCOMPARE(count, 9);
    for (int i = 0; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        QCOMPARE(index.data(FolderModel::FileNameRole).toString(),
                 QStringLiteral("file%1.txt").arg(i + 1));
    }
}

void FolderModelTest::tst_cd()
{
    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);

    //go into firstDir subfolder
    const auto url = m_folderModel->resolvedUrl();
    m_folderModel->cd(0);
    QVERIFY(s.wait(500));
    const auto url2 = m_folderModel->resolvedUrl();
    QVERIFY(url.isParentOf(url2));

    //go back to Desktop
    m_folderModel->up();
    QVERIFY(s.wait(500));
    QCOMPARE(m_folderModel->resolvedUrl(), url);

    //try to cd to an invalid entry (a file)
    m_folderModel->cd(1);
    //Signal is not emitted here as it's invalided
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
}

void FolderModelTest::tst_actionMenu()
{
    const QStringList lst {
        QStringLiteral("open"),
                QStringLiteral("cut"),
                QStringLiteral("open"),
                QStringLiteral("cut"),
                QStringLiteral("undo"),
                QStringLiteral("copy"),
                QStringLiteral("paste"),
                QStringLiteral("pasteto"),
                QStringLiteral("reload"),
                QStringLiteral("refresh"),
                QStringLiteral("rename"),
                QStringLiteral("trash"),
                QStringLiteral("del"),
                QStringLiteral("restoreFromTrash"),
                QStringLiteral("emptyTrash")
    };
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
