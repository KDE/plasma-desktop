/*
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QObject>

#include "kcmoduledata.h"

class BalooSettings;

class BalooData : public KCModuleData
{
    Q_OBJECT

public:
    explicit BalooData(QObject *parent = nullptr);
    BalooSettings *settings() const;

private:
    BalooSettings *m_settings;
};
