/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KCModule>
#include <KPluginMetaData>

class TouchpadConfigPlugin;
class TouchpadConfigLibinput;
class TouchpadConfigXlib;

class TouchpadConfigContainer : public KCModule
{
    Q_OBJECT

    friend TouchpadConfigXlib;
    friend TouchpadConfigLibinput;

public:
    explicit TouchpadConfigContainer(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

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
    TouchpadConfigPlugin *m_plugin = nullptr;
};
