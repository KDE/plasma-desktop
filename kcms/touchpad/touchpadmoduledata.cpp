/*
 * SPDX-FileCopyrightText: 2026 Alexander Wilms <f.alexander.wilms@outlook.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "touchpadmoduledata.h"
#include "kwinwaylandbackend.h"

TouchpadModuleData::TouchpadModuleData(QObject *parent)
    : KCModuleData(parent)
{
    m_backend = KWinWaylandBackend::implementation();
    connect(m_backend, &KWinWaylandBackend::inputDevicesChanged, this, &TouchpadModuleData::updateRelevance);
    updateRelevance();
}

void TouchpadModuleData::updateRelevance()
{
    bool relevant = m_backend->deviceCount() > 0;
    setRelevant(relevant);
}

#include "moc_touchpadmoduledata.cpp"
