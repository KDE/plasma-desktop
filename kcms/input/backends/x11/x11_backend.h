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

#ifndef X11BACKEND_H
#define X11BACKEND_H

#include "inputbackend.h"
#include "evdev_settings.h"

#include <QX11Info>
#include <X11/Xdefs.h>

class ConfigPlugin;

class X11Backend : public InputBackend
{
    Q_OBJECT
public:
    X11Backend(QObject *parent = nullptr);
    ~X11Backend();

    bool isValid() const override { return m_dpy != nullptr; }

    void load() override;

    void apply(bool force = false);

    EvdevSettings* settings() {
        return m_settings;
    }

    bool supportScrollPolarity();
    QStringList supportedAccelerationProfiles();
    QString accelerationProfile();
    double accelRate();
    Handed handed();
    int threshold();

    QString currentCursorTheme();
    void applyCursorTheme(const QString &name, int size);

Q_SIGNALS:
    void mouseStateChanged();
    void mousesChanged();
    void mouseReset();

private:
    void initAtom();
    bool evdevApplyReverseScroll(int deviceid, bool reverse);
    bool libinputApplyReverseScroll(int deviceid, bool reverse);
    void libinputApplyAccelerationProfile(int deviceid, QString profile);

    Atom m_libinputAccelProfileAvailableAtom;
    Atom m_libinputAccelProfileEnabledAtom;
    Atom m_libinputNaturalScrollAtom;

    Atom m_evdevWheelEmulationAtom;
    Atom m_evdevScrollDistanceAtom;
    Atom m_evdevWheelEmulationAxesAtom;

    Atom m_touchpadAtom;
    // We may still need to do something on non-X11 platform due to Xwayland.
    Display* m_dpy;
    bool m_platformX11;
    EvdevSettings *m_settings = nullptr;
    int m_numButtons = 1;
    Handed m_handed = Handed::NotSupported;
    double m_accelRate = 1.0;
    int m_threshold = 0;
    int m_middleButton = -1;
    QStringList m_supportedAccelerationProfiles;
    QString m_accelerationProfile;
};

#endif // X11BACKEND_H
