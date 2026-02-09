/*
 * SPDX-FileCopyrightText: 2026 Alexander Wilms <f.alexander.wilms@outlook.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "gamecontrollermoduledata.h"

#include "devicemodel.h"

GameControllerModuleData::GameControllerModuleData(QObject *parent)
    : KCModuleData(parent)
{
    connect(DeviceModel::instance(), &DeviceModel::devicesChanged, this, &GameControllerModuleData::updateRelevance);
    updateRelevance();
}

void GameControllerModuleData::updateRelevance()
{
    bool relevant = DeviceModel::instance()->rowCount({}) > 0;
    setRelevant(relevant);
}

#include "moc_gamecontrollermoduledata.cpp"
