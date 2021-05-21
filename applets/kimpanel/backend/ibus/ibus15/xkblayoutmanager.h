/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef XKBLAYOUTMANAGER_H
#define XKBLAYOUTMANAGER_H

#include <QSet>
#include <QString>
#include <ibus.h>

class XkbLayoutManager
{
public:
    XkbLayoutManager();
    void setUseXkbModmap(bool use);
    void setLayout(IBusEngineDesc *desc);
    void setLatinLayouts(const gchar **variants, gsize length);
    void getLayout();
    const QString &defaultLayout()
    {
        return m_defaultLayout;
    }
    const QString &defaultVariant()
    {
        return m_defaultVariant;
    }

private:
    QSet<QString> m_latinLayouts;
    QString m_defaultLayout;
    QString m_defaultVariant;
    QString m_defaultOption;
    bool m_useXkbModmap;
    void runXmodmap();
};

#endif // XKBLAYOUT_H
