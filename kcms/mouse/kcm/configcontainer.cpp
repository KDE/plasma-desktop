/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configcontainer.h"

#include "configplugin.h"
#include "inputbackend.h"

#include <KWindowSystem>

#include <memory> // std::unique_ptr

extern "C" {
Q_DECL_EXPORT void kcminit()
{
    std::unique_ptr<InputBackend> backend(InputBackend::implementation());
    if (backend) {
        backend->kcmInit();
    }
}
}

ConfigContainer::ConfigContainer(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    m_plugin = ConfigPlugin::implementation(this);
}

void ConfigContainer::load()
{
    if (m_plugin) {
        m_plugin->load();
    }
}

void ConfigContainer::save()
{
    if (m_plugin) {
        m_plugin->save();
    }
}

void ConfigContainer::defaults()
{
    if (m_plugin) {
        m_plugin->defaults();
    }
}
