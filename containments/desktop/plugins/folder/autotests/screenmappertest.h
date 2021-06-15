/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    Work sponsored by the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
