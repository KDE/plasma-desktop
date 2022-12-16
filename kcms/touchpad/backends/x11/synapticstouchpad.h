/*
    SPDX-FileCopyrightText: 2015 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "xcbatom.h"
#include "xlibtouchpad.h"

class SynapticsTouchpad : public QObject, public XlibTouchpad
{
    Q_OBJECT

public:
    SynapticsTouchpad(Display *display, int deviceId);

    void setTouchpadOff(int touchpadOff) override;
    int touchpadOff() override;

    XcbAtom &touchpadOffAtom() override;

protected:
    double getPropertyScale(const QString &name) const override;

private:
    XcbAtom m_capsAtom, m_touchpadOffAtom;
    int m_resX, m_resY;
    QStringList m_scaleByResX, m_scaleByResY, m_toRadians;
};
