/*
 * Copyright 2017 Roman Gilg <subdiff@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "touchpadconfigcontainer.h"
#include "libinput/touchpadconfiglibinput.h"
#include "xlib/touchpadconfigxlib.h"
#include "touchpadbackend.h"
#include "touchpadconfigplugin.h"

#include <KWindowSystem>
#include <KPluginFactory>

K_PLUGIN_FACTORY(TouchpadConfigContainerFactory, registerPlugin<TouchpadConfigContainer>();)

extern "C" {
Q_DECL_EXPORT void kcminit_touchpad()
{
    if (KWindowSystem::isPlatformX11()) {
        TouchpadConfigContainer::kcmInit();
    }
}
}

TouchpadConfigContainer::TouchpadConfigContainer(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    TouchpadBackend *backend = TouchpadBackend::implementation();
    if (KWindowSystem::isPlatformX11()) {
        if (backend->getMode() == TouchpadInputBackendMode::XLibinput) {
            m_plugin = new TouchpadConfigLibinput(this, backend);
        }
        // For now, if no touchpad is found, always fall back to synaptics frontend,
        // which has a "no touchpad found" message.
        // TODO: show a disabled version of the Libinput frontend as appropriate
        else {
            m_plugin = new TouchpadConfigXlib(this, backend);
        }
    } else if (KWindowSystem::isPlatformWayland()) {
        m_plugin = new TouchpadConfigLibinput(this, backend);
    }
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

QSize TouchpadConfigContainer::minimumSizeHint() const
{
    return m_plugin->minimumSizeHint();
}
QSize TouchpadConfigContainer::sizeHint() const
{
    return m_plugin->sizeHint();
}
void TouchpadConfigContainer::resizeEvent(QResizeEvent * /*event*/)
{
    Q_EMIT changed(false);
    m_plugin->resize(this->size());
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

void TouchpadConfigContainer::hideEvent(QHideEvent *e)
{
    m_plugin->hideEvent(e);
    KCModule::hideEvent(e);
}

#include "touchpadconfigcontainer.moc"
