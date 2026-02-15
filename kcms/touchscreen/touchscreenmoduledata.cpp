/**
 * SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "touchscreenmoduledata.h"

#include "devicesmodel.h"

TouchscreenModuleData::TouchscreenModuleData(QObject *parent)
    : KCModuleData(parent)
    , m_devices(new KWinDevices::DevicesModel(KWinDevices::DevicesModel::Kind::Touch, {}, this))
{
    connect(m_devices, &QAbstractItemModel::rowsInserted, this, &TouchscreenModuleData::updateRelevance);
    connect(m_devices, &QAbstractItemModel::rowsRemoved, this, &TouchscreenModuleData::updateRelevance);
    connect(m_devices, &QAbstractItemModel::modelReset, this, &TouchscreenModuleData::updateRelevance);
    updateRelevance();
}

void TouchscreenModuleData::updateRelevance()
{
    bool relevant = m_devices->rowCount(QModelIndex()) > 0;
    setRelevant(relevant);
}

#include "moc_touchscreenmoduledata.cpp"
