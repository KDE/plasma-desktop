/*
 * Copyright 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 * Copyright 1999 Dirk A. Mueller <dmuell@gmx.net>
 * Copyright 2000 David Faure <faure@kde.org>
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

#ifndef XLIBCONFIG_H
#define XLIBCONFIG_H

#include "../configplugin.h"
#include "backends/x11/evdev_settings.h"

#include "ui_kcmmouse.h"
#include <config-workspace.h>

#include <KCModule>


class X11EvdevBackend;

class XlibConfig : public ConfigPlugin, public Ui::KCMMouse
{
    Q_OBJECT
public:
    XlibConfig(ConfigContainer *parent, InputBackend *backend);
    ~XlibConfig() = default;

    static void kcmInit();

    void load() override;
    void save() override;
    void defaults() override;

private Q_SLOTS:
    void slotHandedChanged(int val);
    void slotScrollPolarityChanged();
    void checkAccess();
    void slotThreshChanged(int value);
    void slotDragStartDistChanged(int value);
    void slotWheelScrollLinesChanged(int value);

private:
    double getAccel();
    int getThreshold();
    Handed getHandedness();

    void setAccel(double);
    void setThreshold(int);
    void setHandedness(Handed);

    X11EvdevBackend *m_backend;
};

#endif // XLIBCONFIG_H
