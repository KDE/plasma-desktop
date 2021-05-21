/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef PROPERTYMANAGER_H
#define PROPERTYMANAGER_H
#include <QByteArray>
#include <ibus.h>

class PropertyManager
{
public:
    PropertyManager();
    virtual ~PropertyManager();

    void setProperties(IBusPropList *props);
    void updateProperty(IBusProperty *prop);
    IBusProperty *property(const QByteArray &key);
    IBusPropList *properties()
    {
        return m_props;
    }

private:
    IBusPropList *m_props;
    IBusProperty *searchList(const QByteArray &key, IBusPropList *props);
};

#endif // PROPERTYMANAGER_H
