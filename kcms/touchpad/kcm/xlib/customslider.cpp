/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "customslider.h"

#include <cmath>

#include <QResizeEvent>

CustomSlider::Interpolator::~Interpolator()
{
}

double CustomSlider::Interpolator::absolute(double relative, double minimum, double maximum) const
{
    return relative * (maximum - minimum) + minimum;
}

double CustomSlider::Interpolator::relative(double absolute, double minimum, double maximum) const
{
    return (absolute - minimum) / (maximum - minimum);
}

double CustomSlider::SqrtInterpolator::absolute(double relative, double minimum, double maximum) const
{
    relative *= relative;
    return Interpolator::absolute(relative, minimum, maximum);
}

double CustomSlider::SqrtInterpolator::relative(double absolute, double minimum, double maximum) const
{
    double value = Interpolator::relative(absolute, minimum, maximum);
    return std::sqrt(value);
}

const CustomSlider::Interpolator CustomSlider::lerp;

CustomSlider::CustomSlider(QWidget *parent)
    : QSlider(parent)
    , m_min(0.0)
    , m_max(1.0)
    , m_interpolator(&lerp)
{
    setSingleStep(10);
    setPageStep(100);

    updateValue();
    updateRange(size());

    connect(this, &QAbstractSlider::actionTriggered, this, &CustomSlider::updateValue);
}

void CustomSlider::resizeEvent(QResizeEvent *e)
{
    QSlider::resizeEvent(e);
    updateRange(e->size());
}

const CustomSlider::Interpolator *CustomSlider::interpolator() const
{
    return m_interpolator;
}

void CustomSlider::setInterpolator(const CustomSlider::Interpolator *v)
{
    m_interpolator = v;
}

void CustomSlider::setDoubleMinimum(double v)
{
    m_min = v;
}

double CustomSlider::doubleMinimum() const
{
    return m_min;
}

void CustomSlider::setDoubleMaximum(double v)
{
    m_max = v;
}

double CustomSlider::doubleMaximum() const
{
    return m_max;
}

double CustomSlider::doubleValue() const
{
    return qBound(m_min, m_value, m_max);
}

void CustomSlider::setDoubleValue(double v)
{
    if (m_value == v) {
        return;
    }

    m_value = v;
    int oldIntValue = value();
    moveSlider();
    if (value() != oldIntValue) {
        Q_EMIT valueChanged(doubleValue());
    }
}

double CustomSlider::intToDouble(int v) const
{
    double relative = lerp.relative(v, minimum(), maximum());
    return m_interpolator->absolute(relative, m_min, m_max);
}

void CustomSlider::updateValue()
{
    m_value = intToDouble(sliderPosition());
    Q_EMIT valueChanged(doubleValue());
}

double CustomSlider::fixup(double v) const
{
    return intToDouble(doubleToInt(v));
}

int CustomSlider::doubleToInt(double v) const
{
    double relative = m_interpolator->relative(v, m_min, m_max);
    double absolute = lerp.absolute(relative, minimum(), maximum());
    return static_cast<int>(std::floor(absolute + 0.5));
}

void CustomSlider::moveSlider()
{
    setValue(doubleToInt(doubleValue()));
}

void CustomSlider::updateRange(const QSize &s)
{
    setRange(0, (orientation() == Qt::Horizontal) ? s.width() : s.height());
    moveSlider();
}
