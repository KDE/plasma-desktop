/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef X11EVDEVBACKEND_H
#define X11EVDEVBACKEND_H

#include "evdev_settings.h"
#include "x11_backend.h"

#include <QX11Info>
#include <X11/Xdefs.h>

class X11EvdevBackend : public X11Backend
{
    Q_OBJECT

public:
    X11EvdevBackend(QObject *parent = nullptr);
    ~X11EvdevBackend();

    void kcmInit() override;

    bool isValid() const override
    {
        return m_dpy != nullptr;
    }

    void load() override;

    void apply(bool force = false);

    EvdevSettings *settings()
    {
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
