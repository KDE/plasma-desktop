/*
 * SPDX-FileCopyrightText: 2026 Alexander Wilms <f.alexander.wilms@outlook.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <KCModuleData>

class DeviceModel;

class GameControllerModuleData : public KCModuleData
{
    Q_OBJECT

public:
    GameControllerModuleData(QObject *parent = nullptr);

private:
    void updateRelevance();
};
