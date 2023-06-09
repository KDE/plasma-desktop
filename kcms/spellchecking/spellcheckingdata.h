/*
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QObject>

#include "kcmoduledata.h"

class SpellCheckingSkeleton;

class SpellCheckingData : public KCModuleData
{
    Q_OBJECT

public:
    explicit SpellCheckingData(QObject *parent = nullptr);
    SpellCheckingSkeleton *settings() const;
    bool isDefaults() const override;

private:
    SpellCheckingSkeleton *m_settings;
};
