/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
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

#ifndef TOUCHPADBACKEND_H
#define TOUCHPADBACKEND_H

#include <QObject>
#include <QVariantHash>

#include <kdemacros.h>

class KDE_EXPORT TouchpadBackend : public QObject
{
    Q_OBJECT
public:
    explicit TouchpadBackend(QObject *parent);

    static TouchpadBackend *implementation();

    virtual bool applyConfig(const QVariantHash &) = 0;
    virtual bool getConfig(QVariantHash &) = 0;
    virtual const QStringList &supportedParameters() const = 0;
    virtual const QString &errorString() const = 0;

    enum TouchpadOffState {
        TouchpadEnabled, TouchpadTapAndScrollDisabled, TouchpadFullyDisabled
    };
    virtual void setTouchpadOff(TouchpadOffState) = 0;
    virtual TouchpadOffState getTouchpadOff() = 0;

    virtual bool isTouchpadEnabled() = 0;
    virtual void setTouchpadEnabled(bool) = 0;

    virtual QStringList listMouses(const QStringList &blacklist) = 0;

    virtual void watchForEvents(bool keyboard) = 0;

Q_SIGNALS:
    void touchpadStateChanged();
    void mousesChanged();
    void touchpadReset();
    void keyboardActivityStarted();
    void keyboardActivityFinished();
};

#endif // TOUCHPADBACKEND_H
