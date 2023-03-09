/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchpadconfigcontainer.h"
#include "libinput/touchpadconfiglibinput.h"
#include "xlib/touchpadconfigxlib.h"
#include "touchpadbackend.h"
#include "touchpadconfigplugin.h"

#include <KWindowSystem>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(TouchpadConfigContainer, "kcm_touchpad.json")

extern "C" {
Q_DECL_EXPORT void kcminit()
{
    if (KWindowSystem::isPlatformX11()) {
        TouchpadConfigContainer::kcmInit();
    }
}
}

TouchpadConfigContainer::TouchpadConfigContainer(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
{
    TouchpadBackend *backend = TouchpadBackend::implementation();
    if (KWindowSystem::isPlatformX11()) {
        if (backend->getMode() == TouchpadInputBackendMode::XLibinput || backend->getMode() == TouchpadInputBackendMode::Unset) {
            m_plugin = new TouchpadConfigLibinput(this, backend);
        } else {
            m_plugin = new TouchpadConfigXlib(this, backend);
        }
    } else if (KWindowSystem::isPlatformWayland()) {
        m_plugin = new TouchpadConfigLibinput(this, backend);
    }

    setButtons(KCModule::Default | KCModule::Apply);
}

void TouchpadConfigContainer::kcmInit()
{
    TouchpadBackend *backend = TouchpadBackend::implementation();
    if (backend->getMode() == TouchpadInputBackendMode::XLibinput) {
        backend->getConfig();
        backend->applyConfig();
    } else if (backend->getMode() == TouchpadInputBackendMode::XSynaptics) {
        TouchpadConfigXlib::kcmInit();
    }
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

/*
QSize TouchpadConfigContainer::minimumSizeHint() const
{
    return m_plugin->minimumSizeHint();
}
QSize TouchpadConfigContainer::sizeHint() const
{
    return m_plugin->sizeHint();
}
void TouchpadConfigContainer::resizeEvent(QResizeEvent * event)
{
    m_plugin->resize(this->size());
}

void TouchpadConfigContainer::hideEvent(QHideEvent *e)
{
    m_plugin->hideEvent(e);
    KCModule::hideEvent(e);
}
*/

#include "touchpadconfigcontainer.moc"
