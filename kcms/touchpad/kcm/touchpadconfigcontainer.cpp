/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchpadconfigcontainer.h"

#include "../logging.h"
#include "libinput/touchpadconfiglibinput.h"
#include "touchpadbackend.h"
#include <config-build-options.h>

#include <KPluginFactory>
#include <KWindowSystem>

K_PLUGIN_CLASS_WITH_JSON(TouchpadConfigContainer, "kcm_touchpad.json")

extern "C" {
Q_DECL_EXPORT void kcminit()
{
#if BUILD_KCM_TOUCHPAD_X11
    if (KWindowSystem::isPlatformX11()) {
        TouchpadConfigContainer::kcmInit();
    }
#endif
}
}

TouchpadConfigContainer::TouchpadConfigContainer(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_plugin(nullptr)
{
    TouchpadBackend *backend = TouchpadBackend::implementation();
    m_plugin = new TouchpadConfigLibinput(this, backend);

    setButtons(KCModule::Default | KCModule::Apply);
}

void TouchpadConfigContainer::kcmInit()
{
#if BUILD_KCM_TOUCHPAD_X11
    TouchpadBackend *backend = TouchpadBackend::implementation();
    if (backend->getMode() == TouchpadInputBackendMode::XLibinput) {
        backend->getConfig();
        backend->applyConfig();
    }
#endif
}

void TouchpadConfigContainer::load()
{
    m_plugin->load();
}

void TouchpadConfigContainer::save()
{
    m_plugin->save();
}

void TouchpadConfigContainer::defaults()
{
    m_plugin->defaults();
}

#include "touchpadconfigcontainer.moc"
