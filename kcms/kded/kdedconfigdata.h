/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEDCONFIGDATA_H
#define KDEDCONFIGDATA_H

#include <KCModuleData>
#include <QObject>

class KDEDConfigData : public KCModuleData
{
    Q_OBJECT

public:
    KDEDConfigData(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    bool isDefaults() const override;
};

#endif // KDEDCONFIGDATA_H
