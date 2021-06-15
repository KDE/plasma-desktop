/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FOLDERMODELTEST_H
#define FOLDERMODELTEST_H

#include <QObject>

class QTemporaryDir;
class FolderModel;

class FolderModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void tst_listing();
    void tst_listingDescending();
    void tst_listingFolderNotFirst();
    void tst_filterListing();
    void tst_cd();
    void tst_rename_data();
    void tst_rename();
    void tst_selection();
    void tst_defaultValues();
    void tst_actionMenu();
    void tst_lockedChanged();
    void tst_multiScreen();
    void tst_multiScreenDifferenPath();

private:
    void createTestFolder(const QString &path);

    FolderModel *m_folderModel;
    QTemporaryDir *m_folderDir;
};

#endif // FOLDERMODELTEST_H
