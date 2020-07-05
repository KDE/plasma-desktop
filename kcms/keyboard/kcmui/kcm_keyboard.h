/*
 * SPDX-FileCopyrightText: Gun Park <mujjingun@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <KQuickAddons/ConfigModule>

#include "hardware_config_model.h"
#include "layout_config_model.h"
#include "advanced_filter_proxy_model.h"

class AdvancedModel;

class KcmKeyboard : public KQuickAddons::ConfigModule {
    Q_OBJECT
    Q_PROPERTY(HardwareConfigModel *hardwareModel READ hardwareModel NOTIFY hardwareModelChanged)
    Q_PROPERTY(LayoutConfigModel *layoutModel READ layoutModel NOTIFY layoutModelChanged)
    Q_PROPERTY(AdvancedFilterProxyModel *advancedModel READ advancedModel NOTIFY advancedModelChanged)

public:
    KcmKeyboard(QObject *parent, const QVariantList &args);
    virtual ~KcmKeyboard() override;

public Q_SLOTS:
    virtual void load() override;
    virtual void save() override;
    virtual void defaults() override;
    void changed();

public:
    HardwareConfigModel *hardwareModel();
    LayoutConfigModel *layoutModel();
    AdvancedFilterProxyModel *advancedModel();

Q_SIGNALS:
    void hardwareModelChanged();
    void layoutModelChanged();
    void advancedModelChanged();

private:
    HardwareConfigModel *m_hardwareConfigModel;
    LayoutConfigModel *m_layoutConfigModel;
    AdvancedModel *m_underlyingAdvancedModel;
    AdvancedFilterProxyModel *m_advancedModel;

    KActionCollection *m_actionCollection;
    QAction *m_nextLayoutAction;
};
