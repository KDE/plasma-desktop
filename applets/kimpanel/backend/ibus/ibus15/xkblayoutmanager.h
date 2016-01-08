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
#ifndef XKBLAYOUTMANAGER_H
#define XKBLAYOUTMANAGER_H

#include <QString>
#include <QSet>
#include <ibus.h>

class XkbLayoutManager
{
public:
    XkbLayoutManager();
    void setUseXkbModmap(bool use);
    void setLayout(IBusEngineDesc* desc);
    void setLatinLayouts(const gchar** variants, gsize length);
    void getLayout();
    const QString& defaultLayout() { return m_defaultLayout; }
    const QString& defaultVariant() { return m_defaultVariant; }
private:
    QSet<QString> m_latinLayouts;
    QString m_defaultLayout;
    QString m_defaultVariant;
    QString m_defaultOption;
    bool m_useXkbModmap;
    void runXmodmap();
};

#endif // XKBLAYOUT_H
