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


#ifndef POSITIONERTEST_H
#define POSITIONERTEST_H

#include <QObject>

class QTemporaryDir;
class FolderModel;
class Positioner;

class PositionerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void tst_positions_data();
    void tst_positions();
    void tst_map();
    void tst_move_data();
    void tst_move();
    void tst_nearestitem_data();
    void tst_nearestitem();
    void tst_isBlank();
    void tst_reset();
    void tst_defaultValues();
    void tst_changeEnabledStatus();
    void tst_changePerStripe();
    void tst_proxyMapping();

private:
    void checkPositions(int perStripe);

    Positioner *m_positioner;
    FolderModel *m_folderModel;
    QTemporaryDir *m_folderDir;
};

#endif // POSITIONERTEST_H
