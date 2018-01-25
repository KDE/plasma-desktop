/***************************************************************************
 *   Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company *
 *                      <info@kdab.com>                                    *
 *   Author: Andras Mantia <andras.mantia@kdab.com>                        *
 *           Work sponsored by the LiMux project of the city of Munich.    *
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

#ifndef SCREENMAPPERTEST_H
#define SCREENMAPPERTEST_H

#include <QObject>

class ScreenMapper;

class ScreenMapperTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();

    void tst_addScreens();
    void tst_removeScreens();
    void tst_addMapping();
    void tst_addRemoveScreenWithItems();
    void tst_addRemoveScreenDifferentPaths();

private:
    void addScreens(const QUrl &path);

    ScreenMapper *m_screenMapper;
};

#endif // SCREENMAPPERTEST_H
