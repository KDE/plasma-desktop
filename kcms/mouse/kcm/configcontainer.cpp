/*
 * Copyright 2018 Roman Gilg <subdiff@gmail.com>
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

#include "configcontainer.h"
#include "configplugin.h"

#include "inputbackend.h"

#include <KWindowSystem/kwindowsystem.h>

extern "C"
{
    Q_DECL_EXPORT void kcminit_mouse()
    {
        InputBackend *backend = InputBackend::implementation();
        backend->kcmInit();
        delete backend;
    }
}

ConfigContainer::ConfigContainer(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    m_plugin = ConfigPlugin::implementation(this);
}

QSize ConfigContainer::minimumSizeHint() const
{
    return m_plugin->minimumSizeHint();
}
QSize ConfigContainer::sizeHint() const
{
    return m_plugin->sizeHint();
}
void ConfigContainer::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_plugin->resize(this->size());
}

void ConfigContainer::load()
{
    m_plugin->load();
}

void ConfigContainer::save()
{
    m_plugin->save();
}

void ConfigContainer::defaults()
{
    m_plugin->defaults();
}

void ConfigContainer::hideEvent(QHideEvent *e)
{
    m_plugin->hideEvent(e);
    KCModule::hideEvent(e);
}
