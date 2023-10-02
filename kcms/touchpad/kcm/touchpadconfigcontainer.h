/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KCModule>
#include <KPluginMetaData>

class TouchpadConfigPlugin;
class TouchpadConfigLibinput;

class TouchpadConfigContainer : public KCModule
{
    Q_OBJECT

    friend TouchpadConfigLibinput;

public:
    explicit TouchpadConfigContainer(QObject *parent, const KPluginMetaData &data);

    static void kcmInit();

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
    TouchpadConfigLibinput *m_plugin;
};
