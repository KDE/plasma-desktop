/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "propertymanager.h"

PropertyManager::PropertyManager()
    : m_props(nullptr)
{
}

PropertyManager::~PropertyManager()
{
    if (m_props)
        g_object_unref(m_props);
}

void PropertyManager::setProperties(IBusPropList *props)
{
    if (m_props)
        g_object_unref(m_props);
    m_props = props;
    if (m_props)
        g_object_ref(m_props);
}

IBusProperty *PropertyManager::property(const QByteArray &key)
{
    if (!m_props)
        return nullptr;

    return searchList(key, m_props);
}

IBusProperty *PropertyManager::searchList(const QByteArray &key, IBusPropList *props)
{
    if (!props)
        return nullptr;

    int i = 0;
    while (true) {
        IBusProperty *prop = ibus_prop_list_get(props, i);
        if (!prop)
            break;
        if (ibus_property_get_key(prop) == key)
            return prop;
        if (ibus_property_get_prop_type(prop) == PROP_TYPE_MENU) {
            IBusProperty *p = searchList(key, ibus_property_get_sub_props(prop));
            if (p)
                return p;
        }
        i++;
    }
    return nullptr;
}

void PropertyManager::updateProperty(IBusProperty *prop)
{
    if (m_props)
        ibus_prop_list_update_property(m_props, prop);
}
