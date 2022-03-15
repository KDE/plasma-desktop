/*
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KRUNNERDATA_H
#define KRUNNERDATA_H

#include <QObject>

#include <KCModuleData>
#include <KSharedConfig>

class KRunnerSettingsBase;

class KRunnerData : public KCModuleData
{
    Q_OBJECT
public:
    KRunnerData(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    bool isDefaults() const override;

private:
    KSharedConfigPtr m_krunnerConfig;
    KRunnerSettingsBase *m_settings;
};

#endif // KRUNNERDATA_H
