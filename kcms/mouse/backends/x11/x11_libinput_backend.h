/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputbackend.h"
#include "inputdevice.h"
#include "x11_libinput_dummydevice.h"

#include <QList>

#include <memory>

class X11LibinputBackend : public InputBackend
{
    Q_OBJECT

public:
    explicit X11LibinputBackend(QObject *parent = nullptr);
    ~X11LibinputBackend() = default;

    void kcmInit() override;

    bool save() override;
    bool load() override;
    bool defaults() override;
    bool isSaveNeeded() const override;
    QString errorString() const override;
    int deviceCount() const override;
    bool isAnonymousInputDevice() const override;
    QList<InputDevice *> inputDevices() const override;

private:
    std::unique_ptr<X11LibinputDummyDevice> m_device;
    QString m_errorString;
};
