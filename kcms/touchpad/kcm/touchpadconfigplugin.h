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

#ifndef TOUCHPADCONFIGPLUGIN_H
#define TOUCHPADCONFIGPLUGIN_H

#include <QWidget>

class TouchpadConfigContainer;
class TouchpadBackend;

class TouchpadConfigPlugin : public QWidget
{
    Q_OBJECT

public:
    TouchpadConfigPlugin(QWidget *parent, TouchpadBackend *backend);
    virtual ~TouchpadConfigPlugin() {}

    virtual void load() {}
    virtual void save() {}
    virtual void defaults() {}

    void hideEvent(QHideEvent *) override {}

protected:
    TouchpadConfigContainer *m_parent;
    TouchpadBackend *m_backend;
};

#endif // TOUCHPADCONFIGPLUGIN_H
