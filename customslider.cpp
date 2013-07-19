#include "customslider.h"

#include <cmath>

#include <QResizeEvent>

CustomSlider::Interpolator::~Interpolator()
{
}

double CustomSlider::Interpolator::absolute(double relative,
                                            double minimum,
                                            double maximum) const
{
    return relative * (maximum - minimum) + minimum;
}

double CustomSlider::Interpolator::relative(double absolute,
                                            double minimum,
                                            double maximum) const
{
    return (absolute - minimum) / (maximum - minimum);
}

const CustomSlider::Interpolator CustomSlider::lerp;

CustomSlider::CustomSlider(QWidget *parent) :
    QSlider(parent), m_min(0.0), m_max(1.0), m_interpolator(&lerp)
{
    setSingleStep(10);
    setPageStep(100);

    updateValue();
    updateRange(size());

    connect(this, SIGNAL(actionTriggered(int)), SLOT(updateValue()));
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
    m_value = v;
    moveSlider();
    Q_EMIT valueChanged(doubleValue());
}

void CustomSlider::updateValue()
{
    double relative = lerp.relative(sliderPosition(), minimum(), maximum());
    m_value = m_interpolator->absolute(relative, m_min, m_max);
    Q_EMIT valueChanged(doubleValue());
}

void CustomSlider::moveSlider()
{
    double relative = m_interpolator->relative(doubleValue(), m_min, m_max);
    double absolute = lerp.absolute(relative, minimum(), maximum());
    setValue(static_cast<int>(std::floor(absolute + 0.5)));
}

void CustomSlider::updateRange(const QSize &s)
{
    setRange(0, (orientation() == Qt::Horizontal) ? s.width() : s.height());
    moveSlider();
}
