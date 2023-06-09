/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <KCModuleData>

class KeysData : public KCModuleData
{
    Q_OBJECT
public:
    KeysData(QObject *parent = nullptr);
    bool isDefaults() const override;

private:
    bool m_isDefault = true;
    int m_pendingComponentCalls = 0;
};
