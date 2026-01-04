/*
 * SPDX-FileCopyrightText: 2026 Alexander Wilms <f.alexander.wilms@outlook.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "mousemoduledata.h"

#include "inputbackend.h"

MouseModuleData::MouseModuleData(QObject *parent)
    : KCModuleData(parent)
{
    m_backend = InputBackend::create();
    connect(m_backend.get(), &InputBackend::inputDevicesChanged, this, &MouseModuleData::updateRelevance);
    updateRelevance();
}

void MouseModuleData::updateRelevance()
{
    bool relevant = m_backend->deviceCount() > 0;
    setRelevant(relevant);
}

#include "moc_mousemoduledata.cpp"
