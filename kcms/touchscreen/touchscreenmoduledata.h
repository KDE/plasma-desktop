/**
 * SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <KCModuleData>

namespace KWinDevices
{
class DevicesModel;
}

class TouchscreenModuleData : public KCModuleData
{
    Q_OBJECT

public:
    TouchscreenModuleData(QObject *parent = nullptr);

private:
    void updateRelevance();

    KWinDevices::DevicesModel *m_devices;
};
