/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KEYSDATA_H
#define KEYSDATA_H

#include <KCModuleData>

class KeysData : public KCModuleData
{
    Q_OBJECT
public:
    KeysData(QObject *parent = nullptr, const QVariantList &args = {});
    bool isDefaults() const override;

private:
    bool m_isDefault = true;
    int m_pendingComponentCalls = 0;
};

#endif
