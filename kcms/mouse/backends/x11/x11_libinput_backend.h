/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputbackend.h"

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
    bool isAnonymousDevice() const override;
    QList<QObject *> getDevices() const override;

private:
    QObject *m_device;
    QString m_errorString = QString();
};
