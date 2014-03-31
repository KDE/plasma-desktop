/*
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2008 Michael Jansen <kde@michael-jansen.biz>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SHORTCUTS_MODULE_H
#define SHORTCUTS_MODULE_H

#include <kcmodule.h>
#include <QtCore/QHash>

class KGlobalShortcutsEditor;

class GlobalShortcutsModule : public KCModule
{
    Q_OBJECT
public:
    GlobalShortcutsModule(QWidget *parent, const QVariantList &args);
    ~GlobalShortcutsModule();

    virtual void save();
    virtual void load();
    virtual void defaults();

private:
    KGlobalShortcutsEditor *editor;
};


#endif
