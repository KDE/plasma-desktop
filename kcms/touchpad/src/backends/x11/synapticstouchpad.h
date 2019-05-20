/*
 * Copyright (C) 2015 Weng Xuetian <wengxt@gmail.com>
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

#ifndef SYNAPTICSTOUCHPAD_H
#define SYNAPTICSTOUCHPAD_H

#include "xlibtouchpad.h"
#include "xcbatom.h"

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

#endif // SYNAPTICSTOUCHPAD_H
