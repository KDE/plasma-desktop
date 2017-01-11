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

#ifndef TOUCHPADCONFIGCONTAINER_H
#define TOUCHPADCONFIGCONTAINER_H

#include <KCModule>

class TouchpadConfigPlugin;
class TouchpadConfigLibinput;
class TouchpadConfigXlib;

class TouchpadConfigContainer : public KCModule
{
    Q_OBJECT

    friend TouchpadConfigXlib;
    friend TouchpadConfigLibinput;

public:
    explicit TouchpadConfigContainer(QWidget *parent,
                            const QVariantList &args = QVariantList());

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void resizeEvent(QResizeEvent *event);

    virtual void load();
    virtual void save();
    virtual void defaults();

    void kcmLoad() {KCModule::load();};
    void kcmSave() {KCModule::save();};
    void kcmDefaults() {KCModule::defaults();};

protected:
    virtual void hideEvent(QHideEvent *);

private:
    TouchpadConfigPlugin* m_plugin;
};

#endif // TOUCHPADCONFIGCONTAINER_H
