/*
 * Copyright 2017 Xuetian Weng <wengxt@gmail.com>
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

#ifndef XLIBMOUSEBACKEND_H
#define XLIBMOUSEBACKEND_H

#include "mousebackend.h"

#include <QX11Info>
#include <X11/Xdefs.h>

class X11MouseBackend : public MouseBackend
{
    Q_OBJECT
public:
    X11MouseBackend(QObject *parent = nullptr);
    ~X11MouseBackend();

    bool isValid() const override { return m_dpy != nullptr; }

    void load() override;
    bool supportScrollPolarity() override;
    QStringList supportedAccelerationProfiles() override;
    QString accelerationProfile() override;
    double accelRate() override;
    MouseHanded handed() override;
    int threshold() override;
    void apply(const MouseSettings & settings, bool force) override;

    QString currentCursorTheme() override;
    void applyCursorTheme(const QString &name, int size) override;

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
    int m_numButtons = 1;
    MouseHanded m_handed = MouseHanded::NotSupported;
    double m_accelRate = 1.0;
    int m_threshold = 0;
    int m_middleButton = -1;
    QStringList m_supportedAccelerationProfiles;
    QString m_accelerationProfile;
};

#endif // XLIBMOUSEBACKEND_H
