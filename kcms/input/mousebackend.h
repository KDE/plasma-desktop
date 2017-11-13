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

#ifndef MOUSEBACKEND_H
#define MOUSEBACKEND_H

#include <QObject>
#include "mousesettings.h"

class MouseBackend : public QObject
{
    Q_OBJECT
protected:
    explicit MouseBackend(QObject *parent) : QObject(parent) {}

public:
    static MouseBackend *implementation();

    virtual bool isValid() const = 0;

    // This function will be called before query any property below, thus it
    // can be used to save some round trip.
    virtual void load() = 0;
    virtual void apply(const MouseSettings &settings, bool force) = 0;

    // Return the value from display server or compositor if applicable.
    virtual bool supportScrollPolarity() = 0;
    virtual QStringList supportedAccelerationProfiles() = 0;
    virtual QString accelerationProfile() = 0;
    virtual double accelRate() = 0;
    virtual int threshold() = 0;
    virtual MouseHanded handed() = 0;

    virtual QString currentCursorTheme() = 0;
    virtual void applyCursorTheme(const QString &name, int size) = 0;
};

#endif // MOUSEBACKEND_H
