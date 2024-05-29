/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputbackend.h"
#include "inputdevice.h"
#include "x11_libinput_dummydevice.h"

#include <QList>

class X11LibinputBackend : public InputBackend
{
    Q_OBJECT

public:
    explicit X11LibinputBackend(QObject *parent = nullptr);
    ~X11LibinputBackend() = default;

    void kcmInit() override;

    bool applyConfig() override;
    bool getConfig() override;
    bool getDefaultConfig() override;
    bool isChangedConfig() const override;
    QString errorString() const override;
    int deviceCount() const override;
    bool isAnonymousInputDevice() const override;
    QList<InputDevice *> inputDevices() const override;

private:
    X11LibinputDummyDevice *m_device = nullptr;
    QString m_errorString;
};
