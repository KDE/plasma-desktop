/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include "inputdevice.h"

struct ca_context;

/**
 * @brief Creates a calibration matrix from four points on the screen.
 */
class CalibrationTool : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool finishedCalibration READ finishedCalibration NOTIFY finishedCalibrationChanged)
    Q_PROPERTY(int currentTarget READ currentTarget NOTIFY currentTargetChanged)
    Q_PROPERTY(float width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(float height READ height WRITE setHeight NOTIFY heightChanged)

public:
    void setWidth(float width);
    float width() const;

    void setHeight(float height);
    float height();

    /**
     * @return Whether the calibration has finished. This happesn when all four points on screen are given.
     */
    bool finishedCalibration() const;

    /**
     * @return The current target point index. If the calibration only just started, this would be 0 for example.
     */
    int currentTarget() const;

public Q_SLOTS:
    /**
     * @brief Set a point on the screen for calibration.
     * @param touchX The X coordinate of the point on screen that was touched.
     * @param touchY The Y coordinate of the point on screen that was touched.
     * @param screenX The real X coordinate of the on-screen point.
     * @param screenY The real Y coordinate of the on-screen point.
     * @note The X/Y between touch & screen are meant to be slightly different.
     */
    void calibrate(double touchX, double touchY, double screenX, double screenY);

    /**
     * @brief Sets the calibration matrix for the input device.
     * @param device The input device to set the calibration matrix for.
     * @param matrix The matrix to use, this should come from finished().
     */
    void setCalibrationMatrix(InputDevice *device, const QMatrix4x4 &matrix);

    /**
     * @brief Restores the default calibration matrix for the input device.
     * @param device The input device to restore the calibration for.
     */
    void restoreDefaults(InputDevice *device);

    /**
     * @brief Restarts the calibration process, getting rid of any existing point data.
     */
    void reset();

Q_SIGNALS:
    /**
     * @brief Called when finishedCalibration changed, which can happen either when resetting or finishing a calibration.
     */
    void finishedCalibrationChanged();

    /**
     * @brief Emitted when the calibration was finished, along with the resulting matrix.
     */
    void finished(const QMatrix4x4 &matrix);

    /**
     * @brief Emitted when the current target point changed. This happens regularly during calibration.
     */
    void currentTargetChanged();

    /**
     * @brief Emitted when the width of the screen changed.
     */
    void widthChanged();

    /**
     * @brief Emitted when the height of the screen changed.
     */
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
};
