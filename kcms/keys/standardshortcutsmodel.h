/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include "basemodel.h"
#include "kglobalaccelmodel_export.h"

class KGLOBALACCELMODEL_EXPORT StandardShortcutsModel : public BaseModel
{
    Q_OBJECT
public:
    explicit StandardShortcutsModel(QObject *parent);

    void load() override;
    void save() override;

    void exportToConfig(KConfigBase &config) override;
    void importConfig(const KConfigBase &config) override;
};
