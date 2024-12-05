/*
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "baloodata.h"
#include "kcm.h"

#include <QVariantList>

#include <baloo/baloosettings.h>

using namespace Baloo;

BalooData::BalooData(QObject *parent)
    : KCModuleData(parent)
    , m_settings(new BalooSettings(this))
{
    autoRegisterSkeletons();
}

BalooSettings *BalooData::settings() const
{
    return m_settings;
}

#include "baloodata.moc"

#include "moc_baloodata.cpp"
