/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileCopyrightText: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <Plasma/Applet>
#include <QJsonDocument>
#include <QObject>

class QTemporaryDir;
class FolderModel;
class Positioner;
struct Pos {
    int x;
    int y;
};

class PositionerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void tst_default_positions_data();
    void tst_default_positions();
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
    void tst_configSaveLoad();
    void tst_changeResolution();
    void tst_renameFile();
    void tst_insertFile();
    void tst_dontSaveWithoutScreen();

private:
    void checkDefaultPositions(int perStripe);
    void ensureFolderModelReady();
    QJsonDocument getCurrentConfig();
    QHash<QString, Pos> getPositionHash(QStringList positions);
    void changeResolution(QString resolution);

    QString m_currentActivity;
    Positioner *m_positioner;
    FolderModel *m_folderModel;
    QTemporaryDir *m_folderDir;
    Plasma::Applet *m_applet = nullptr;
};
