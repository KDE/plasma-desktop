/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sliderpair.h"

#include <QSlider>

SliderPair::SliderPair(QSlider *minSlider, QSlider *maxSlider, QObject *parent)
    : QObject(parent)
    , m_minSlider(minSlider)
    , m_maxSlider(maxSlider)
{
    connect(m_minSlider, &QAbstractSlider::valueChanged, this, &SliderPair::adjustMaxSlider);
    connect(m_maxSlider, &QAbstractSlider::valueChanged, this, &SliderPair::adjustMinSlider);
}

void SliderPair::adjustMaxSlider()
{
    m_maxSlider->setValue(qMax(m_maxSlider->value(), m_minSlider->value()));
}

void SliderPair::adjustMinSlider()
{
    m_minSlider->setValue(qMin(m_maxSlider->value(), m_minSlider->value()));
}
