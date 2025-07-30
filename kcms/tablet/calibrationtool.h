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

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(int currentTarget READ currentTarget NOTIFY currentTargetChanged)
    Q_PROPERTY(float width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(float height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(int resetSecondsLeft READ resetSecondsLeft NOTIFY resetSecondsLeftChanged)

public:
    CalibrationTool();

    void setWidth(float width);

    /**
     * @return The width of the calibration window.
     */
    float width() const;

    void setHeight(float height);

    /**
     * @return The height of the calibration window.
     */
    float height();

    enum class State {
        Calibrating, /**< The user is calibrating by touching each point. >*/
        Confirming, /**< The user is confirming that their calibration works correctly. >*/
        Testing, /**< The user is testing out their new calibration. >*/
    };
    Q_ENUM(State)

    /**
     * @return The current state of the tool.
     */
    State state() const;

    /**
     * @return The current target point index. If the calibration only just started, this would be 0 for example.
     */
    int currentTarget() const;

    /**
     * @return The seconds left in the reset counter.
     */
    int resetSecondsLeft() const;

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
     * @brief Called when the state changed.
     */
    void stateChanged();

    /**
     * @brief Emitted when the calibration was finished, along with the resulting matrix.
     */
    void calibrationCreated(const QMatrix4x4 &matrix);

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

    /**
     * @brief Emitted when the reset seconds left counter changes.
     */
    void resetSecondsLeftChanged();

    /**
     * @brief Emitted when the device should reset it's calibration matrix to the saved one.
     */
    void resetFromSaved();

private:
    void checkIfFinished();

    void playSound(const QString &soundName);
    ca_context *canberraContext();

    float m_width = 0.0f;
    float m_height = 0.0f;

    int m_calibratedTargets = 0;
    State m_state = State::Calibrating;
    std::array<QPointF, 4> m_screenPoints;
    std::array<QPointF, 4> m_touchPoints;
    int m_resetCountdown = 0;
    QTimer m_resetTimer;

    ca_context *m_canberraContext = nullptr;

    /// Number of seconds before the calibration tool resets if no action is taken.
    const int actionResetSeconds = 15;
};
