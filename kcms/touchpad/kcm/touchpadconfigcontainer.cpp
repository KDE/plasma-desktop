/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchpadconfigcontainer.h"

#include "../logging.h"
#include "libinput/touchpadconfiglibinput.h"
#include "touchpadbackend.h"
#include "touchpadconfigplugin.h"

#include <config-build-options.h>

#if BUILD_KCM_TOUCHPAD_X11
#include "xlib/touchpadconfigxlib.h"
#endif

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
    if (TouchpadBackend *backend = TouchpadBackend::implementation(); backend != nullptr) {
        switch (backend->getMode()) {
#if BUILD_KCM_TOUCHPAD_X11
        case TouchpadInputBackendMode::XLibinput:
        case TouchpadInputBackendMode::Unset:
            m_plugin = new TouchpadConfigLibinput(this, backend);
            break;
        case TouchpadInputBackendMode::XSynaptics:
            m_plugin = new TouchpadConfigXlib(this, backend);
            break;
#endif
#if BUILD_KCM_TOUCHPAD_KWIN_WAYLAND
        case TouchpadInputBackendMode::WaylandLibinput:
            m_plugin = new TouchpadConfigLibinput(this, backend);
            break;
#endif
        default:
            Q_UNUSED(m_plugin);
        }
    }
    if (m_plugin == nullptr) {
        qFatal("Not able to select appropriate configuration plugin, aborting kcm_touchpad startup");
    }

    setButtons(KCModule::Default | KCModule::Apply);
}

void TouchpadConfigContainer::kcmInit()
{
#if BUILD_KCM_TOUCHPAD_X11
    TouchpadBackend *backend = TouchpadBackend::implementation();
    if (backend->getMode() == TouchpadInputBackendMode::XLibinput) {
        backend->getConfig();
        backend->applyConfig();
    } else if (backend->getMode() == TouchpadInputBackendMode::XSynaptics) {
        TouchpadConfigXlib::kcmInit();
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
