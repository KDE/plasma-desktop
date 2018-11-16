/*
 *  Copyright (C) 2014 Weng Xuetian <wengxt@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "propertymanager.h"
#include <QByteArray>

PropertyManager::PropertyManager() : m_props(nullptr)
{

}

PropertyManager::~PropertyManager()
{
    if (m_props)
        g_object_unref(m_props);
}

void PropertyManager::setProperties(IBusPropList* props)
{
    if (m_props)
        g_object_unref(m_props);
    m_props = props;
    if (m_props)
        g_object_ref(m_props);
}

IBusProperty* PropertyManager::property(const QByteArray& key)
{
    if (!m_props)
        return nullptr;

    return searchList(key, m_props);
}

IBusProperty* PropertyManager::searchList(const QByteArray& key, IBusPropList* props)
{
    if (!props)
        return nullptr;

    int i = 0;
    while (true) {
        IBusProperty* prop = ibus_prop_list_get(props, i);
        if (!prop)
            break;
        if (ibus_property_get_key(prop) == key)
            return prop;
        if (ibus_property_get_prop_type(prop) == PROP_TYPE_MENU) {
            IBusProperty* p = searchList(key, ibus_property_get_sub_props(prop));
            if (p)
                return p;
        }
        i++;
    }
    return nullptr;
}

void PropertyManager::updateProperty(IBusProperty* prop)
{
    if (m_props)
        ibus_prop_list_update_property(m_props, prop);
}
