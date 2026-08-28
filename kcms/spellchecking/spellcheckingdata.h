/*
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QObject>

#include "kcmoduledata.h"

namespace Sonnet
{
class Settings;
}

class SpellCheckingData : public KCModuleData
{
    Q_OBJECT

public:
    explicit SpellCheckingData(QObject *parent = nullptr);
    bool isDefaults() const override;

private:
    Sonnet::Settings *m_settings;
};
