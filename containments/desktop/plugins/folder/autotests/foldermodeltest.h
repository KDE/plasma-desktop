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
