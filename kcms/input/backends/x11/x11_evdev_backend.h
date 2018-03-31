/*
 * Copyright 2017 Xuetian Weng <wengxt@gmail.com>
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

#ifndef X11EVDEVBACKEND_H
#define X11EVDEVBACKEND_H

#include "x11_backend.h"
#include "evdev_settings.h"

#include <QX11Info>
#include <X11/Xdefs.h>

class X11EvdevBackend : public X11Backend
{
    Q_OBJECT

public:
    X11EvdevBackend(QObject *parent = nullptr);
    ~X11EvdevBackend();

    void kcmInit() override;

    bool isValid() const override { return m_dpy != nullptr; }

    void load() override;

    void apply(bool force = false);

    EvdevSettings* settings() {
        return m_settings;
    }

    bool supportScrollPolarity();

    double accelRate();
    Handed handed();
    int threshold();

Q_SIGNALS:
    void mouseStateChanged();
    void mousesChanged();
    void mouseReset();

private:
    void initAtom();
    bool evdevApplyReverseScroll(int deviceid, bool reverse);


    Atom m_evdevWheelEmulationAtom;
    Atom m_evdevScrollDistanceAtom;
    Atom m_evdevWheelEmulationAxesAtom;

    Atom m_touchpadAtom;

    EvdevSettings *m_settings = nullptr;
    int m_numButtons = 1;
    Handed m_handed = Handed::NotSupported;
    double m_accelRate = 1.0;
    int m_threshold = 0;
    int m_middleButton = -1;
};

#endif // X11EVDEVBACKEND_H
