/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include "inputdevice.h"

struct ca_context;

class CalibrationTool : public QObject
{
    Q_OBJECT
    Q_PROPERTY(InputDevice *device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(bool finishedCalibration READ finishedCalibration NOTIFY finished)
    Q_PROPERTY(int currentTarget READ currentTarget NOTIFY currentTargetChanged)
    Q_PROPERTY(float width MEMBER m_width NOTIFY widthChanged)
    Q_PROPERTY(float height MEMBER m_height NOTIFY heightChanged)

public:
    InputDevice *device() const;
    void setDevice(InputDevice *device);

    bool finishedCalibration() const;
    int currentTarget() const;

public Q_SLOTS:
    void calibrate(double touchX, double touchY, double screenX, double screenY);
    void restoreDefaults();
    void reset();

Q_SIGNALS:
    void deviceChanged();
    void finished();
    void currentTargetChanged();
    void widthChanged();
    void heightChanged();

private:
    void checkIfFinished();

    void playSound(const QString &soundName);
    ca_context *canberraContext();

    float m_width = 0.0f;
    float m_height = 0.0f;

    int m_calibratedTargets = 0;
    bool m_finishedCalibration = false;
    std::array<QPointF, 4> m_screenPoints;
    std::array<QPointF, 4> m_touchPoints;

    ca_context *m_canberraContext = nullptr;
    InputDevice *m_device = nullptr;
};
