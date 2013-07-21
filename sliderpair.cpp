#include "sliderpair.h"

SliderPair::SliderPair(QSlider *minSlider, QSlider *maxSlider, QObject *parent)
    : QObject(parent), m_minSlider(minSlider), m_maxSlider(maxSlider)
{
    connect(m_minSlider, SIGNAL(valueChanged(int)), SLOT(adjustMaxSlider()));
    connect(m_maxSlider, SIGNAL(valueChanged(int)), SLOT(adjustMinSlider()));
}

void SliderPair::adjustMaxSlider()
{
    m_maxSlider->setValue(qMax(m_maxSlider->value(), m_minSlider->value()));
}

void SliderPair::adjustMinSlider()
{
    m_minSlider->setValue(qMin(m_maxSlider->value(), m_minSlider->value()));
}
