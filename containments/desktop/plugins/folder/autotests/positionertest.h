/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileCopyrightText: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
