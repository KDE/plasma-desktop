/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calibrationtool.h"

#include "logging.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <canberra.h>

void CalibrationTool::setWidth(const float width)
{
    if (m_width != width) {
        m_width = width;
        Q_EMIT widthChanged();
    }
}

float CalibrationTool::width() const
{
    return m_width;
}

void CalibrationTool::setHeight(const float height)
{
    if (m_height != height) {
        m_height = height;
        Q_EMIT heightChanged();
    }
}

float CalibrationTool::height()
{
    return m_height;
}

bool CalibrationTool::finishedCalibration() const
{
    return m_finishedCalibration;
}

int CalibrationTool::currentTarget() const
{
    return m_calibratedTargets;
}

void CalibrationTool::calibrate(const double touchX, const double touchY, const double screenX, const double screenY)
{
    Q_ASSERT(m_calibratedTargets >= 0 && m_calibratedTargets < 4);
    if (m_calibratedTargets < 0 || m_calibratedTargets >= 4) {
        return;
    }

    m_screenPoints[m_calibratedTargets] = {screenX, screenY};
    m_touchPoints[m_calibratedTargets] = {touchX, touchY};

    m_calibratedTargets++;
    Q_EMIT currentTargetChanged();

    checkIfFinished();
    playSound(QStringLiteral("completion-partial"));
}

void CalibrationTool::setCalibrationMatrix(InputDevice *device, const QMatrix4x4 &matrix)
{
    Q_ASSERT(device);
    device->setCalibrationMatrix(device->calibrationMatrix() * matrix);
}

void CalibrationTool::restoreDefaults(InputDevice *device)
{
    Q_ASSERT(device);
    device->setCalibrationMatrix(device->defaultCalibrationMatrix());
    playSound(QStringLiteral("dialog-information"));
}

void CalibrationTool::reset()
{
    m_calibratedTargets = 0;
    Q_EMIT currentTargetChanged();

    m_finishedCalibration = false;
    Q_EMIT finishedCalibrationChanged();
}

void ca_finish_callback(ca_context *c, uint32_t id, int error_code, void *userdata)
{
    Q_UNUSED(c);
    Q_UNUSED(id);
    Q_UNUSED(error_code);
    Q_UNUSED(userdata);
}

QMatrix3x3 invert(const QMatrix3x3 &m)
{
    /*
     * from https://stackoverflow.com/questions/983999/simple-3x3-matrix-inverse-code-c
     * with some simplification
     */
    const float m4857 = m.data()[4] * m.data()[8] - m.data()[5] * m.data()[7];
    const float m3746 = m.data()[3] * m.data()[7] - m.data()[4] * m.data()[6];
    const float m5638 = m.data()[5] * m.data()[6] - m.data()[3] * m.data()[8];
    const float det = m.data()[0] * (m4857) + m.data()[1] * (m5638) + m.data()[2] * (m3746);

    const float invdet = 1 / det;

    QMatrix3x3 minv;
    minv.data()[0] = (m4857)*invdet;
    minv.data()[1] = (m.data()[2] * m.data()[7] - m.data()[1] * m.data()[8]) * invdet;
    minv.data()[2] = (m.data()[1] * m.data()[5] - m.data()[2] * m.data()[4]) * invdet;
    minv.data()[3] = (m5638)*invdet;
    minv.data()[4] = (m.data()[0] * m.data()[8] - m.data()[2] * m.data()[6]) * invdet;
    minv.data()[5] = (m.data()[2] * m.data()[3] - m.data()[0] * m.data()[5]) * invdet;
    minv.data()[6] = (m3746)*invdet;
    minv.data()[7] = (m.data()[1] * m.data()[6] - m.data()[0] * m.data()[7]) * invdet;
    minv.data()[8] = (m.data()[0] * m.data()[4] - m.data()[1] * m.data()[3]) * invdet;

    return minv;
}

void CalibrationTool::checkIfFinished()
{
    if (m_calibratedTargets >= 4) {
        Q_ASSERT(m_width > 0.0f);
        Q_ASSERT(m_height > 0.0f);

        // Calibration math based off of https://github.com/kreijack/xlibinput_calibrator
        // The gist is that we want to calculate the difference between screen and touch position, and then normalize it before passing to KWin

        enum {
            UpperLeft = 0,
            UpperRight = 1,
            LowerLeft = 2,
            LowerRight = 3,
        };

        // We have four points to use for calibration.
        // To construct a mapping between screen space and touch space, we only need three points.
        // So we use four sets of each permutation, and calculate an average based off of the sum of the individual matrices.
        const std::array permutations{
            std::array{UpperLeft, UpperRight, LowerLeft},
            std::array{LowerRight, UpperRight, LowerLeft},
            std::array{LowerRight, UpperLeft, LowerLeft},
            std::array{LowerRight, UpperLeft, UpperRight},
        };

        QMatrix3x3 sum;
        sum.fill(0.0f); // Ensure that the sum matrix is zeroed, not an identity. The identity will throw the sum off.

        for (int i = 0; i < 4; i++) {
            const QPointF screenA = m_screenPoints[permutations[i][0]];
            const QPointF screenB = m_screenPoints[permutations[i][1]];
            const QPointF screenC = m_screenPoints[permutations[i][2]];

            // clang-format off
            const std::array screenMatrix
            {
                static_cast<float>(screenA.x()), static_cast<float>(screenA.y()), 1.0f,
                static_cast<float>(screenB.x()), static_cast<float>(screenB.y()), 1.0f,
                static_cast<float>(screenC.x()), static_cast<float>(screenC.y()), 1.0f,
            };
            // clang-format on

            const QPointF touchA = m_touchPoints[permutations[i][0]];
            const QPointF touchB = m_touchPoints[permutations[i][1]];
            const QPointF touchC = m_touchPoints[permutations[i][2]];

            // clang-format off
            const std::array touchMatrix{
                static_cast<float>(touchA.x()), static_cast<float>(touchA.y()), 1.0f,
                static_cast<float>(touchB.x()), static_cast<float>(touchB.y()), 1.0f,
                static_cast<float>(touchC.x()), static_cast<float>(touchC.y()), 1.0f
            };
            // clang-format on

            sum += invert(QMatrix3x3(touchMatrix.data())) * QMatrix3x3(screenMatrix.data());
        }

        QMatrix3x3 average = sum / 4.0f;

        // Change and normalize coordinate spaces
        average(0, 1) *= m_height / m_width;
        average(0, 2) *= 1.0f / m_width;

        average(1, 0) *= m_width / m_height;
        average(1, 2) *= 1.0f / m_height;

        // Remove the junk
        average(2, 0) = 0.0f;
        average(2, 1) = 0.0f;
        average(2, 2) = 1.0f;

        m_finishedCalibration = true;
        Q_EMIT finishedCalibrationChanged();
        Q_EMIT finished(QMatrix4x4(average));

        playSound(QStringLiteral("completion-success"));
    }
}

void CalibrationTool::playSound(const QString &soundName)
{
    const auto soundThemeConfig = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    const KConfigGroup soundGroup = soundThemeConfig->group(QStringLiteral("Sounds"));
    const QString soundTheme = soundGroup.readEntry("Theme", QStringLiteral("ocean"));

    ca_proplist *props = nullptr;
    ca_proplist_create(&props);
    ca_proplist_sets(props, CA_PROP_CANBERRA_XDG_THEME_NAME, qUtf8Printable(soundTheme));
    ca_proplist_sets(props, CA_PROP_CANBERRA_CACHE_CONTROL, "permanent");

    ca_proplist_sets(props, CA_PROP_EVENT_ID, soundName.toLatin1().constData());
    ca_context_play_full(canberraContext(), 0, props, &ca_finish_callback, this);

    ca_proplist_destroy(props);
}

ca_context *CalibrationTool::canberraContext()
{
    if (!m_canberraContext) {
        int ret = ca_context_create(&m_canberraContext);
        if (ret != CA_SUCCESS) {
            qCWarning(KCM_TABLET) << "Failed to initialize canberra context:" << ca_strerror(ret);
            m_canberraContext = nullptr;
            return nullptr;
        }

        // clang-format off
        ca_context_change_props(m_canberraContext,
                                CA_PROP_APPLICATION_NAME, "Tablet KCM",
                                CA_PROP_APPLICATION_ID, "kcm_tablet",
                                CA_PROP_APPLICATION_ICON_NAME, "preferences-desktop-tablet",
                                nullptr);
        // clang-format on
    }

    return m_canberraContext;
}

#include "calibrationtool.moc"
#include "moc_calibrationtool.cpp"
