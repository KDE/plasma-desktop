/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KCModule>

class ConfigPlugin;

class ConfigContainer : public KCModule
{
    Q_OBJECT

public:
    explicit ConfigContainer(QObject *parent, const KPluginMetaData &data);

    void load() override;
    void save() override;
    void defaults() override;

    void kcmLoad()
    {
        KCModule::load();
    }
    void kcmSave()
    {
        KCModule::save();
    }
    void kcmDefaults()
    {
        KCModule::defaults();
    }

private:
    ConfigPlugin *m_plugin = nullptr;
};
