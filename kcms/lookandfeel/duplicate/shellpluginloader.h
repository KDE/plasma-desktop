/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SHELLPLUGINLOADER_H
#define SHELLPLUGINLOADER_H

#include <Plasma/PluginLoader>

class ShellPluginLoader : public Plasma::PluginLoader
{
public:
    ShellPluginLoader();
    ~ShellPluginLoader();

    static void init();

protected:
    Plasma::Package internalLoadPackage(const QString &packageFormat, const QString &specialization);
};

#endif
