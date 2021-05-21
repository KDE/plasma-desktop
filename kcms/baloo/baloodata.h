/*
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BALOODATA_H
#define BALOODATA_H

#include <QObject>

#include "kcmoduledata.h"

class BalooSettings;

class BalooData : public KCModuleData
{
    Q_OBJECT

public:
    explicit BalooData(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    BalooSettings *settings() const;

private:
    BalooSettings *m_settings;
};
#endif
